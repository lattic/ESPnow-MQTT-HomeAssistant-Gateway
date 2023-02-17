/*
sender.ino
*/

#include <Arduino.h>  
#include "devices_config.h"         // devices configuration file
#include "common_config.h"          // common configuration file
// #include "devices_config.h"         // devices configuration file
#include "passwords.h"              // sensistive data

// ========================================================================== libraries  
// Firmware update
#include <ArduinoJson.h>
#include <Cipher.h>
#include <HTTPClient.h>
#include <Update.h>

// other libraries and variables
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Wire.h>

// config in nvs - Preferences
#include <nvs_flash.h>

// captive portal
#include <DNSServer.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"

// timers
#include "freertos/timers.h"

// preferences
// #include <Preferences.h>

// filesystem
#include "FS.h"
#include <LittleFS.h>
#include "littlefs-configfile-structure.h"
#include "littlefs-configfile-functions.h"

// MAX17048 - battery fuel gauge, I2C
#if (USE_MAX17048 == 1)
  #include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
#endif

// sht31 - temperature and humidity, I2C
#if (USE_SHT31 == 1)
  #include "Adafruit_SHT31.h"
#endif

// lux from TSL2561 - light sensor, I2C
#if (USE_TSL2561 == 1)
  #include <SparkFunTSL2561.h>
#endif
// ========================================================================== libraries END

// some consistency checks 
#if (!defined (HOSTNAME) or !defined(DEVICE_NAME) or !defined(BOARD_TYPE) )
  #error "HOSTNAME, DEVICE_NAME, BOARD_TYPE are obligatory"
#endif


// ========================================================================== variables
// ESPnow configuration
uint8_t broadcastAddress[]        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};       // auxuliary variable
uint8_t broadcastAddress_all[]    = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};       // BROADCAST - any receiver - not used yet

// for MAC manipulation
char mac_org_char[18];
uint8_t mac_org[6];
char mac_new_char[18];
uint8_t mac_new[6];


esp_now_peer_info_t peerInfo;

bool espnow_data_sent=false;      // for OnDataSent()

// Firmware update
HTTPClient firmware_update_client;
int fw_totalLength                = 0;
int fw_currentLength              = 0;
bool perform_update_firmware      = false;
int update_progress               = 0;
int old_update_progress           = 0;
bool blink_led_status             = false;
// Firmware update END

// captive portal
DNSServer dnsServer;
AsyncWebServer server(80);
String cp_html_page;

String ssid, old_ssid;
String password,old_password;
String channel,old_channel;
String sleeptime_s_str,old_sleeptime_s_str;
bool is_setup_done                = false;
bool valid_ssid_received          = false;
bool valid_channel_received       = false;
bool valid_password_received      = false;
bool valid_sleeptime_s_str_received = false;
bool wifi_timeout                 = false;
int sleeptime_s                   = SLEEP_TIME_S;
// captive portal END

// tasks
TaskHandle_t led_blink_handle     = NULL;
BaseType_t xReturned_led_blink;

// timers
TimerHandle_t cp_timer_handle     = NULL;
int id                            = 1;

// MAX17048 - battery fuel gauge, I2C
#if (USE_MAX17048 == 1)
  SFE_MAX1704X lipo(MAX1704X_MAX17048);
#endif

// sht31 - temperature and humidity, I2C
#if (USE_SHT31 == 1)
  Adafruit_SHT31 sht31 = Adafruit_SHT31();
#endif

// lux from TSL2561 - light sensor, I2C
#if (USE_TSL2561 == 1)
  SFE_TSL2561 light;
#endif

// data variables
typedef struct struct_message           // 92 bytes
{
  char host[10];                        // esp32123   - 9 characters maximum (esp32123=8)
  char name[16];                        // 15 characters maximum
  char ver[10];                         // 123.56.89  - 9 characters maximum (123.56.89=9)
  char sender_type[10];                 // "env", "motion","env+mot"
  char charg[5];                        // "FULL","ON","NC" - 4 characters maximum
  float temp;
  float hum;
  float lux;
  float bat;
  float batpct;
  float batchr;
  byte motion;                          // 0 - no motion, 1 - motion
  unsigned int boot;
  unsigned long ontime;                 // seconds, probably unsigned int would be enough - check it
  byte boardtype;
  u_int8_t wifi_ok;                     // 0 - wifi not configured, 1 - wifi configured
  u_int8_t motion_enabled;              // 0 - motion disabled, 1 - motion enabled
} struct_message;

struct_message myData;

// device type - this array can expand [first dimension] but the max size of string is 9 characters - this text goes to myData.sender_type over ESPnow
const char sender_type_char[5][10] = {"env", "motion", "env+mot","battery","undef"};

// auxuliary variables:

// to calculate time:
unsigned long start_espnow_time = 0;
unsigned long program_start_time,em,tt;
unsigned long function_start, function_end;

// check reasons to wake up and start
esp_sleep_wakeup_cause_t wakeup_reason;
byte boot_reason; // esp_reset_reason_t ???
uint64_t wakeup_gpio_mask;
byte wakeup_gpio;

bool fw_update = false;
bool motion = false;
// unsigned long bootCount = 1;  //we don't start from 0 but from 1

bool sht31ok = true;
bool tslok   = true;
bool max17ok = true;

char charging[5];       // global as assigned in setup() - to avoid calling again
uint8_t charging_int = 0;       // by default NC

// config file
const char *config_file = "/config.json";
Config config;


//ciphering
char* cipher_key = (char*)CIPHER_KEY;   


// captive portal html page
class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      //request->addInterestingHeader("ANY");
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      #include "html-page.h"
      const char *index_html =cp_html_page.c_str();
      request->send_P(200, "text/html", index_html);
    }
};
// captive portal html page END

// receiving command from gateway
bool command_received = false;
typedef struct struct_message_recv 
{
  u_int8_t command;
} struct_message_recv;
struct_message_recv data_recv;
// receiving command from gateway END



// ========================================================================== variables END

// ========================================================================== FUNCTIONS declaration

bool load_config();
void save_config(const char* reason);
void gather_data();
u_int8_t charging_state();
void change_mac();

void hibernate(bool force, int final_sleeping_time_s);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
bool send_data();
void OnDataReceived(const uint8_t * mac, const uint8_t *incomingData, int len); 


void do_esp_restart();
void do_esp_go_to_sleep();
void disable_espnow();
void sos(int led);
void erase_all_data();
void do_update();
void updateFirmware(uint8_t *data, size_t len);
int update_firmware_prepare();

void initiate_all_leds();
void set_act_blue_led_level(u_int8_t level); 
void set_error_red_led_level(u_int8_t level);  

// captive portal
void connect_wifi();
void setup_CP_Server();
void WiFiSoftAPSetup();
void WiFiStationSetup(String rec_ssid, String rec_password, String rec_sleep_s);
void StartCaptivePortal();
void led_blink(void *pvParams);
void led_blink_erase(void *pvParams);
void cp_timer( TimerHandle_t cp_timer_handle );
void get_old_wifi_credentials();

// ========================================================================== FUNCTIONS declaration END

// ========================================================================== FUNCTIONS implementation

void OnDataReceived(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&data_recv, incomingData, sizeof(data_recv));
  command_received = true;
}

// check charging
u_int8_t charging_state()
{
  uint8_t charging_int_local = 0;
  // function_start = micros();
  #if defined(CHARGING_GPIO) and defined(POWER_GPIO)
  /*
    - both GPIO must be PULLUP as LOW is active from TP4056
    - LEDs on TP4056 are NOT needed if PULL_UP both GPIO
    
    #define POWER_GPIO                38  // GREEN, STDB PIN6 on TP4056, LOW on CHARGED (LED ON),       comment out if not in use - don't use "0" here
    #define CHARGING_GPIO             39  // RED,   CHRG PIN7 on TP4056, LOW during CHARGING (LED ON),  comment out if not in use - don't use "0" here
    
    truth table:
    NC:               POWER_GPIO HIGH (PULLUP)    CHARGING_GPIO HIGH (PULLUP)
    CHARGING:         POWER_GPIO HIGH (PULLUP)    CHARGING_GPIO LOW     
    FULL (CHARGED):   POWER_GPIO LOW              CHARGING_GPIO HIGH (PULLUP)
    OFF (DISABLED):   POWER_GPIO HIGH (PULLUP)    CHARGING_GPIO HIGH (PULLUP) ???? - not checked yet

  */
    // moved to setup()
    // pinMode(CHARGING_GPIO,INPUT_PULLUP);
    // pinMode(POWER_GPIO,INPUT_PULLUP);
    
    uint8_t power_gpio_state    = digitalRead(POWER_GPIO);
    uint8_t charging_gpio_state = digitalRead(CHARGING_GPIO);
    

    if ((power_gpio_state)   and (charging_gpio_state))   {snprintf(charging,sizeof(CHARGING_NC), "%s",   CHARGING_NC);     charging_int_local = 0;}
    if ((power_gpio_state)   and (!charging_gpio_state))  {snprintf(charging,sizeof(CHARGING_ON), "%s",   CHARGING_ON);     charging_int_local = 1;}
    if ((!power_gpio_state)  and (charging_gpio_state))   {snprintf(charging,sizeof(CHARGING_FULL), "%s", CHARGING_FULL);   charging_int_local = 2;}
    // if ((digitalRead(POWER_GPIO) == 1) and (digitalRead(CHARGING_GPIO) == 1)) charging_int_local = 3; // OFF ??

    #ifdef DEBUG
    Serial.printf("[%s]: charging=%s\n",__func__,charging);
    #endif
  
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
  #endif
  return charging_int_local;
}


// update firmware
// turn off espnow
void disable_espnow()
{
  // function_start = micros();
  esp_now_unregister_send_cb();
  esp_now_deinit();
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
}


// blink nicely - SOS on upgrade failure
void sos(int led)
{
  // function_start = micros();
  #define DIT_MS 50;
  int dit = DIT_MS;
  int dah = 3 * dit;
  int inter_dit = dit;
  int inter_letter = 3 * dit;
  int inter_word = 7 * dit;

  digitalWrite(led,LOW);
  delay(inter_word);

  // s
  for (byte i=0; i<3; i++)
  {
    digitalWrite(led,HIGH);
    delay(dit);
    digitalWrite(led,LOW);
    delay(dit);
  }
  delay(inter_letter);
  // 0
  for (byte i=0; i<3; i++)
  {
    digitalWrite(led,HIGH);
    delay(dah);
    digitalWrite(led,LOW);
    delay(dit);
  }
  delay(inter_letter);
  // s
  for (byte i=0; i<3; i++)
  {
    digitalWrite(led,HIGH);
    delay(dit);
    digitalWrite(led,LOW);
    delay(dit);
  }
  delay(inter_word);
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
}


// FW upgrade wrapper
void do_update()
{
  // function_start = micros();
  Serial.printf("[%s]: FW UPGRADE starting...\n",__func__);
  // turn OFF BLUE LED
  set_act_blue_led_level(0);
  int update_firmware_status = -1;
  update_firmware_status=update_firmware_prepare();
  if (update_firmware_status == 0)
  {
    Serial.printf("[%s]: RESTARTING - FW update SUCCESSFULL\n\n",__func__);
    // blink slowly when FW upgrade successfull
    for (int i=0;i<3;i++)
    {
      #ifdef ERROR_RED_LED_GPIO
        set_error_red_led_level(0);
        delay(500);
        set_error_red_led_level(1);
        delay(100);
      #elif defined(ACT_BLUE_LED_GPIO)
        set_act_blue_led_level(0);
        delay(100);
        set_act_blue_led_level(1);
        delay(30);
        set_act_blue_led_level(0);
      #endif
    }

  } else
  {
    Serial.printf("[%s]: FW update failed - reason: %d\nRESTARTING - FW update failed\n\n",__func__,update_firmware_status);
    #ifdef ERROR_RED_LED_GPIO
      sos(ERROR_RED_LED_GPIO);
    #elif defined(ACT_BLUE_LED_GPIO)
      sos(ACT_BLUE_LED_GPIO);
    #endif
  }
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
  do_esp_restart();
}


// real upgrade
void updateFirmware(uint8_t *data, size_t len)
{
  // blink ERROR_RED_LED_GPIO or...
  #ifdef ERROR_RED_LED_GPIO
    if (blink_led_status) {
      blink_led_status=LOW;
      set_error_red_led_level(blink_led_status);
    } else {
      blink_led_status=HIGH;
      set_error_red_led_level(blink_led_status);
    }
  #else
    // ...blink ACT_BLUE_LED_GPIO
    #ifdef ACT_BLUE_LED_GPIO
      if (blink_led_status) {
        blink_led_status=LOW;
        set_act_blue_led_level(blink_led_status);
      } else {
        blink_led_status=HIGH;
        set_act_blue_led_level(blink_led_status);
      }
    #endif
  #endif

  Update.write(data, len);
  fw_currentLength += len;
  old_update_progress=update_progress;
  update_progress=(fw_currentLength * 100) / fw_totalLength;
  if (update_progress>old_update_progress){
    if (update_progress % 5 == 0){ //it prints every 5%
      Serial.printf("[%s]: FW update: %d%%\n",__func__,update_progress);
    }
  }
  // if current length of written firmware is not equal to total firmware size, repeat
  if(fw_currentLength != fw_totalLength) return;
  Update.end(true);
  Serial.printf("\n[%s]: Update Success, Total Size: %d bytes\n",__func__,fw_currentLength);
}


// download from webserver
int update_firmware_prepare()
{
  char firmware_file[255];
  #ifndef CLIENT
    // UPDATE_FIRMWARE_HOST is in local network
    snprintf(firmware_file,sizeof(firmware_file),"%s/01-Production/0-ESPnow/firmware/%s.bin",UPDATE_FIRMWARE_HOST,HOSTNAME);
  #else
    // UPDATE_FIRMWARE_HOST is on internet
    snprintf(firmware_file,sizeof(firmware_file),"%s/%s.bin",UPDATE_FIRMWARE_HOST,HOSTNAME);
  #endif

  fw_totalLength=0;
  fw_currentLength=0;
  Serial.printf("[%s]: uploading file: %s\n",__func__,firmware_file);
  firmware_update_client.begin(firmware_file);
  int resp = firmware_update_client.GET();
  Serial.printf("[%s]: Response: %d\n",__func__,resp);
  // If file is reachable, start downloading
  if(resp == 200){
    // get length of document (is -1 when Server sends no Content-Length header)
    fw_totalLength = firmware_update_client.getSize();
    // transfer to local variable
    int len = fw_totalLength;
    // this is required to start firmware update process
    Update.begin(UPDATE_SIZE_UNKNOWN);
    Serial.printf("[%s]: FW Size: %lu bytes\n",__func__,fw_totalLength);

    // create buffer for read
    uint8_t buff[128] = { 0 };
    // get tcp stream
    WiFiClient * stream = firmware_update_client.getStreamPtr();
    // read all data from server
    Serial.printf("[%s]: Updating firmware progress:\n",__func__);
    while(firmware_update_client.connected() && (len > 0 || len == -1))
    {
      // get available data size
      size_t size = stream->available();
      if(size) {
        // read up to 128 byte
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        // pass to function
        updateFirmware(buff, c);
        if(len > 0) {
           len -= c;
        }
      }
      delay(1);
      }
  }else
  {
    Serial.printf("[%s]: Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location.\n",__func__);
    Serial.printf("[%s]: firmware update prepare UNSUCESSFUL\n",__func__);
    return resp;
  }
  firmware_update_client.end();
  Serial.printf("[%s]: firmware update prepare SUCESSFULL\n",__func__);
  return 0;
}
// update firmware END


// hibernate - force to indicate no GPIO wake up - used for low battery
void hibernate(bool force, int final_sleeping_time_s) 
{
  esp_wifi_stop();                                                        // wifi
  esp_deep_sleep_disable_rom_logging();                                   // it does not display welcome message - shorter time to wake up

  /* all below makes CP not appearing sometimes, still current without it is 38uA on S2 so that should be OK */
//   esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
//   esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M,        ESP_PD_OPTION_OFF);
//   esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,      ESP_PD_OPTION_OFF);
//   esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
//   esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

//  // the next 2 lines are NOT working - CP cannot start!
//   esp_sleep_config_gpio_isolate();
//   esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);



  if (force) // used when battery too low - don't wake up on GPIO
  {
    Serial.printf("[%s]: Battery empty - going to sleep for %d hours\n",__func__,(final_sleeping_time_s / 3600));
    esp_sleep_enable_timer_wakeup(final_sleeping_time_s * uS_TO_S_FACTOR);
    do_esp_go_to_sleep();
  }

  #ifdef DEBUG
    #ifdef MOTION_SENSOR_GPIO
      Serial.printf("[%s]: MOTION_SENSOR_GPIO=%d\n",__func__,MOTION_SENSOR_GPIO);
    #endif
    #ifdef FW_UPGRADE_GPIO
      Serial.printf("[%s]: FW_UPGRADE_GPIO=%d\n",__func__,FW_UPGRADE_GPIO);
    #endif
  #endif
  uint64_t bitmask_dec;

  // https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/ for ext0 and ext1 examples
  #ifdef MOTION_SENSOR_GPIO
    if (motion)
    // when motion detected we don't allow second wakup on motion - first cooling time only or FW update
    {
      //send ESP to deep unconditional sleep for predefined time -  wake up on timer (cooling period)
      esp_sleep_enable_timer_wakeup(COOLING_SLEEP_DURATION_S * uS_TO_S_FACTOR);
      //... or on GPIO ext1 (FW update)
      #ifdef FW_UPGRADE_GPIO //if FW_UPGRADE_GPIO  defined, wake up on it
        bitmask_dec = pow(2,FW_UPGRADE_GPIO);
        // bitmask_dec =  (1ULL << FW_UPGRADE_GPIO);// | (1ULL << MOTION_SENSOR_GPIO);
        #ifdef DEBUG
          Serial.printf("[%s]: bitmask_dec (FW_UPGRADE_GPIO) in dec=%ju\n",__func__,bitmask_dec);
        #endif
        // ESP32-C3
        #if (BOARD_TYPE == 4)
          {
            esp_deep_sleep_enable_gpio_wakeup(bitmask_dec, ESP_GPIO_WAKEUP_GPIO_HIGH);
          }
        #else
          {
            esp_sleep_enable_ext1_wakeup(bitmask_dec, ESP_EXT1_WAKEUP_ANY_HIGH);
          }
        #endif

      #endif

      Serial.printf("[%s]: going to sleep for %ds (cooling time)\n",__func__,COOLING_SLEEP_DURATION_S);
    } else
    {
      //send ESP to deep unconditional sleep for predefined time -  wake up on timer...(heartbeat)
      
      // esp_sleep_enable_timer_wakeup(SLEEP_TIME_S * uS_TO_S_FACTOR);
      esp_sleep_enable_timer_wakeup(final_sleeping_time_s * uS_TO_S_FACTOR);

      //... or on GPIO ext1 (PIR motion detected) or FW_UPGRADE_GPIO
      #ifdef FW_UPGRADE_GPIO //if FW_UPGRADE_GPIO  defined, wake up on it or on PIR - actually FW_UPGRADE_GPIO is obligatory


        if (g_motion == 1)
          bitmask_dec = pow(2,FW_UPGRADE_GPIO) + pow(2,MOTION_SENSOR_GPIO);
        else 
          // don't wake up on MOTION_SENSOR_GPIO
          bitmask_dec = pow(2,FW_UPGRADE_GPIO);

        // bitmask_dec =  (1ULL << FW_UPGRADE_GPIO) | (1ULL << MOTION_SENSOR_GPIO);
        #ifdef DEBUG
          Serial.printf("[%s]: bitmask_dec (FW_UPGRADE_GPIO + MOTION_SENSOR_GPIO) in dec=%ju\n",__func__,bitmask_dec);
        #endif
      #else //if FW_UPGRADE_GPIO not defined, wake up only on PIR - btw this is not allowed anyway as FW_UPGRADE_GPIO is mandatory
        bitmask_dec = pow(2,MOTION_SENSOR_GPIO);
        // bitmask_dec =   (1ULL << MOTION_SENSOR_GPIO);
        #ifdef DEBUG
          Serial.printf("[%s]: bitmask_dec=2^GPIO in dec=%d\n",__func__,bitmask_dec);
        #endif
      #endif
      // ESP32-C3
      #if (BOARD_TYPE == 4)
        {
          esp_deep_sleep_enable_gpio_wakeup(bitmask_dec, ESP_GPIO_WAKEUP_GPIO_HIGH);
        }
      #else
        {
          esp_sleep_enable_ext1_wakeup(bitmask_dec, ESP_EXT1_WAKEUP_ANY_HIGH);
        }
      #endif

      Serial.printf("[%s]: going to sleep until next motion detected\n",__func__);
      Serial.printf("[%s]:  or for %ds (heartbeat)\n",__func__,final_sleeping_time_s);
    }
  #else
    //send ESP to deep unconditional sleep for predefined time -  wake up on timer...(heartbeat)
    esp_sleep_enable_timer_wakeup(final_sleeping_time_s * uS_TO_S_FACTOR);
    //... or on GPIO ext1 FW_UPGRADE_GPIO
    #ifdef FW_UPGRADE_GPIO //if FW_UPGRADE_GPIO  defined, wake up on it or on PIR - actually FW_UPGRADE_GPIO is obligatory
      bitmask_dec = pow(2,FW_UPGRADE_GPIO);
      // bitmask_dec =  (1ULL << FW_UPGRADE_GPIO);// | (1ULL << MOTION_SENSOR_GPIO);
      #ifdef DEBUG
        Serial.printf("[%s]: bitmask_dec (FW_UPGRADE_GPIO) in dec=%ju\n",__func__,bitmask_dec);
      #endif
      // ESP32-C3
      #if (BOARD_TYPE == 4)
        {
          esp_deep_sleep_enable_gpio_wakeup(bitmask_dec, ESP_GPIO_WAKEUP_GPIO_HIGH);
        }
      #else
        {
          esp_sleep_enable_ext1_wakeup(bitmask_dec, ESP_EXT1_WAKEUP_ANY_HIGH);
        }
      #endif
    #endif
    Serial.printf("[%s]: going to sleep for %ds (heartbeat)\n",__func__,final_sleeping_time_s);
  #endif
  // testing with PPK2 - go to sleep
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,HIGH);
  #endif
  do_esp_go_to_sleep();
}


// restart ESP
void do_esp_restart()
{
  save_config("Config saved before restart");
  Serial.printf("[%s]: Bye...\n========= R E S T A R T I N G =========\n",__func__);
  ESP.restart();
}

// on sent callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    espnow_data_sent = true;
    #ifdef DEBUG
      Serial.printf("[%s]: espnow_data_sent=%d OK\n",__func__,ESP_NOW_SEND_SUCCESS);
    #endif
  } else 
  {
    espnow_data_sent = false;
    #ifdef DEBUG
      Serial.printf("[%s]: espnow_data_sent=%d FAILED\n",__func__,ESP_NOW_SEND_SUCCESS);
    #endif
  }
}


// gather data from sensors, returns false if (bat_volts < MINIMUM_VOLTS) so ESPnow will not start - goes to sleep instead of sending
void gather_data()
{
  // function_start = micros();
  // testing with PPK2 - start gather data
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,LOW);
  #endif
  #ifdef DEBUG
    Serial.printf("[%s]:\n",__func__);
  #endif
  // sender_type
  snprintf(myData.sender_type,sizeof(myData.sender_type),"%s",sender_type_char[SENSOR_TYPE]);

  #ifdef DEBUG
    Serial.printf(" Data gatehered:\n\tsender_type=%s\n",myData.sender_type);
    // Serial.printf("\tmotion=%d\n",myData.motion);
  #endif

  // wifi_ok
  myData.wifi_ok = g_wifi_ok;

  // motion enabled/disable
  #ifdef MOTION_SENSOR_GPIO
    myData.motion_enabled = g_motion;
  #else
    myData.motion_enabled = 0;
  #endif

  // hostname
  strcpy(myData.host, HOSTNAME);
  #ifdef DEBUG
    Serial.printf("\thost=%s\n",myData.host);
  #endif

  // name
  strcpy(myData.name, DEVICE_NAME);
  #ifdef DEBUG
    Serial.printf("\tname=%s\n",myData.name);
  #endif

  // motion
  myData.motion = motion;
  #ifdef DEBUG
    Serial.printf("\tmotion=%d\n",myData.motion);
  #endif

  // sht31
  myData.temp = 0.00f;
  myData.hum = 0.00f;
  #if (USE_SHT31 == 1)
    if (sht31ok)
    {
      #ifndef CALIBRATE_TEMPERATURE
        #define CALIBRATE_TEMPERATURE       0   
      #endif

      #ifndef CALIBRATE_HUMIDITY
        #define CALIBRATE_HUMIDITY          0   
      #endif

      myData.temp = sht31.readTemperature() + (CALIBRATE_TEMPERATURE);
      myData.hum = sht31.readHumidity()     + (CALIBRATE_HUMIDITY);
    }
  #endif
  #ifdef DEBUG
    Serial.printf("\ttemp=%fC\n",myData.temp);
    Serial.printf("\thum=%f%%\n",myData.hum);
  #endif

  // lux
  myData.lux = 0.0f;
  #if (USE_TSL2561 == 1)
    unsigned int data0, data1; //data0=infrared, data1=visible light
    double lux;              // Resulting lux value
    boolean good;            // True if neither sensor is saturated
    boolean gain = false;     // Gain setting, 0 = X1, 1 = X16; if with gain and overshoots it goes from 0 again - better false
    unsigned int ms;         // Integration ("shutter") time in milliseconds
    // If time = 0, integration will be 13.7ms
    // If time = 1, integration will be 101ms
    // If time = 2, integration will be 402ms
    // If time = 3, use manual start / stop to perform your own integration
    unsigned char time = 0;

    light.begin();
    light.setTiming(gain,time,ms);
    if (light.setPowerUp()) tslok = true; else tslok = false;

    light.manualStart();
    delay(ms);
    light.manualStop();

    myData.lux = 0.0f;
    if (tslok)
    {
      if (light.getData(data0,data1))
      {
        good = light.getLux(gain,ms,data0,data1,lux);
        myData.lux = lux;
      }
    }
  if (g_led_pwm != 0)
  {
    if (lux <= 1) 
    {
      g_led_pwm = 5;          // darkest
    } else 
    {
      if (lux < 10) 
      {
        g_led_pwm = 50;       // middle
      } else 
      {
        g_led_pwm = 255;      // full
      }
    }
  }
  #else 
    // g_led_pwm = 50;           // default for sensors with no lux measurements (if there is LED even there)
  #endif
  #ifdef DEBUG
    Serial.printf("\tlux=%flx\n",myData.lux);
  #endif

  // battery
  myData.bat = 0.0f;
  myData.batpct = 0.0f;
  myData.batchr = 0.0f;

  #if (USE_MAX17048 == 1)
    if (max17ok)
    {
      myData.bat = lipo.getVoltage();
      myData.batpct = lipo.getSOC();
      myData.batchr = lipo.getChangeRate();

      // EMERGENCY - LOW BATTERY - don't start WiFi or ESPnow - hibernate ASAP for 1h
      if (myData.bat < MINIMUM_VOLTS)
      {
        Serial.printf("[%s]: battery volts=%0.2fV, that is below minimum [%0.2fV]\n",__func__,myData.bat,MINIMUM_VOLTS);
        Serial.printf("[%s]: that is below MINIMUM\n",__func__);
        hibernate(true, (SLEEP_TIME_H_BATTERY_EMPTY*60*60)); // SLEEP_TIME_H_BATTERY_EMPTY hours sleep if battery empty
      } else
      {
        #ifdef DEBUG
          Serial.printf("\tbattery level=OK\n");
        #endif
      }
    }
  #endif
  #ifdef DEBUG
    Serial.printf("\tbat=%fV\n",myData.bat);
    Serial.printf("\tbatpct=%f%%\n",myData.batpct);
    Serial.printf("\tbatchr=%f%%\n",myData.batchr);
  #endif

  // version
  if (fw_update)
  {
    // send to HA "FW UPG..." instead of version during FW upgrade to indicate the process
    snprintf(myData.ver,sizeof(myData.ver),"%s","upgrading");
  } else
  {
    strcpy(myData.ver, ZH_PROG_VERSION);
  }
  #ifdef DEBUG
    Serial.printf("\tver=%s\n",myData.ver);
  #endif

  // charging - already gathered in setup()
  snprintf(myData.charg,5,"%s",charging);
  #ifdef DEBUG
    Serial.printf("\tcharg=%s\n",myData.charg);
  #endif

  // bootCount calculated in save_config()
  myData.boot = g_bootCount;
  #ifdef DEBUG
    Serial.printf("\tboot=%d\n",myData.boot);
  #endif

  // ontime in seconds rather than ms - updated in save_config() - this is previous reading - it does not include the current run
   if (charging_int == 0) 
   {
      myData.ontime = g_saved_ontime_ms/1000;
   } else 
   {
      myData.ontime = 0;
   }


  
  #ifdef DEBUG
    Serial.printf("[%s]: ontime=%lus\n",__func__,myData.ontime);
  #endif

  // boardtype
  myData.boardtype = BOARD_TYPE;
  #ifdef DEBUG
    Serial.printf("\tboardtype=%d\n",myData.boardtype);
  #endif

  #ifdef DEBUG
    Serial.printf(" Total struct size=%d bytes\n",sizeof(myData));
  #endif

  // testing with PPK2 - end gather_data
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,HIGH);
  #endif
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
}


// send data to gateway over ESPnow
bool send_data()      // unsigned long s1=millis();Serial.println("sending took " + String(millis()-s1) + "ms");
{
  char receiver_mac[18];
  int channel, last_gw;
  unsigned long loop_timer;

  unsigned long s1=millis();  // micros

  #if (USE_FAKE_RECEIVER == 1)
    #pragma message ("\n\n[MESSAGE] NOT sending data to any receiver - FAKE MAC ADDRESS USED")
    return false;
  #endif

  //   #if (MAC_BROADCAST == 1)
  //     #pragma message ("\n\n[MESSAGE] sending data to all ESP - BROADCAST")
  //     memcpy(broadcastAddress, broadcastAddress_all, sizeof(broadcastAddress));
  //   #endif

  channel = g_wifi_channel;
  last_gw = g_last_gw;
  if ((channel < 1) or (channel > 13)) channel = 1;                       
  if ((last_gw < 0) or (last_gw > (NUMBER_OF_GATEWAYS-1))) last_gw = 0;
  #ifdef DEBUG
    Serial.printf("[%s]: Configuration data from file: channel=%d, last gateway=%d\n",__func__,channel,last_gw);
  #endif
  // read config END

  WiFi.mode(WIFI_MODE_STA);

 // ======================================================== LETS TRY SAVED CONFIG ========================================================
  // lets try saved data first to shorten the time: saved receiver, saved channel
  #ifdef DEBUG
    Serial.printf("\n[%s]: Lets try saved config first...\n\n",__func__);
  #endif
  memcpy(broadcastAddress, receivers[last_gw], sizeof(receivers[last_gw]));
  snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);

  // init espnow
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK)
  {
    Serial.printf("[%s]: esp_now_init FAILED\n",__func__);
    set_error_red_led_level(1);
    delay(100);
    // FATAL ERROR - EXIT
    return false;
  } else
  {
    #ifdef DEBUG
      Serial.printf("[%s]: esp_now_init SUCCESSFUL\n",__func__);
    #endif
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, sizeof(broadcastAddress));
  peerInfo.ifidx   = WIFI_IF_STA;
  peerInfo.encrypt = false;

  // register cb
  if (esp_now_register_send_cb(OnDataSent) != ESP_OK)
  {
    Serial.printf("[%s]: esp_now_register_send_cb FAILED\n",__func__);
  } else
  {
    #ifdef DEBUG
      Serial.printf("[%s]: esp_now_register_send_cb SUCCESSFUL\n",__func__);
    #endif
  }

  if (esp_now_register_recv_cb(OnDataReceived) != ESP_OK)
  {
    Serial.printf("[%s]: esp_now_register_recv_cb FAILED\n",__func__);
  } else
  {
    #ifdef DEBUG
      Serial.printf("[%s]: esp_now_register_recv_cb SUCCESSFUL\n",__func__);
    #endif
  }  


  // add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.printf("[%s]: esp_now_add_peer FAILED\n",__func__);
  } else
  {
    #ifdef DEBUG
      Serial.printf("[%s]: esp_now_add_peer SUCCESSFUL\n",__func__);
    #endif
  }
  #ifdef DEBUG
    Serial.printf("[%s]: [SAVED CHANNEL, SAVED RECEIVER] trying channel=%d, receiver=%d \n",__func__,channel,last_gw);
  #endif
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  #ifdef DEBUG
    Serial.printf("[%s]: [SAVED CHANNEL, SAVED RECEIVER] sending took=%dms\n",__func__,millis()-s1);
  #endif
  loop_timer=millis(); while(millis() < loop_timer + 10) {}  // unblocking loop for 10ms
  if(espnow_data_sent)
  {
    // #ifdef DEBUG
      Serial.printf("[%s]:\n [SAVED CHANNEL, SAVED RECEIVER] esp_now_send SUCCESSFUL on channel=%d, receiver=%d, MAC=%s; it took=%dms\n",__func__,channel,last_gw,receiver_mac,millis()-s1);
    // #endif
    return true;
  }
  #ifdef DEBUG
    Serial.printf("[%s]: [SAVED CHANNEL, SAVED RECEIVER] esp_now_send FAILED on channel=%d, receiver=%d, MAC=%s; it took=%dms\n",__func__,channel,last_gw,receiver_mac,millis()-s1);
  #endif
  // lets try saved data first to shorten the time: saved receiver, saved channel END
// ======================================================== LETS TRY SAVED CONFIG END ========================================================


// ======================================================== LOOP THROUGH ALL RECEIVERS =======================================================
  // if we are here, it means it failed - so now lets try everything else
  #ifdef DEBUG
    Serial.printf("\n[%s]: Loop through all receivers...\n",__func__);
  #endif
  for (int receiver = 0; receiver<NUMBER_OF_GATEWAYS; receiver++)   // lets go through all gateways
  {
    memcpy(broadcastAddress, receivers[receiver], sizeof(receivers[receiver]));
    snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);
    #ifdef DEBUG
      Serial.printf("\n[%s]: gateway=%d, MAC=%s...\n\n",__func__,receiver,receiver_mac);
    #endif

    // init espnow
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    if (esp_now_init() != ESP_OK) 
    {
      Serial.printf("[%s]: esp_now_init FAILED\n",__func__);
      set_error_red_led_level(1);
      delay(100);
      // FATAL ERROR - EXIT
      return false;
    } else 
    {
      #ifdef DEBUG
        Serial.printf("[%s]: esp_now_init SUCCESSFUL\n",__func__);
      #endif
    }

    memcpy(peerInfo.peer_addr, broadcastAddress, sizeof(broadcastAddress));
    peerInfo.ifidx   = WIFI_IF_STA;
    peerInfo.encrypt = false;

    // register cb
    if (esp_now_register_send_cb(OnDataSent) != ESP_OK)
    {
      Serial.printf("[%s]: esp_now_register_send_cb FAILED\n",__func__);
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: esp_now_register_send_cb SUCCESSFUL\n",__func__);
      #endif
    }

    if (esp_now_register_recv_cb(OnDataReceived) != ESP_OK)
    {
      Serial.printf("[%s]: esp_now_register_recv_cb FAILED\n",__func__);
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: esp_now_register_recv_cb SUCCESSFUL\n",__func__);
      #endif
    }  

    // add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.printf("[%s]: esp_now_add_peer FAILED\n",__func__);
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: esp_now_add_peer SUCCESSFUL\n",__func__);
      #endif
    }

// ======================================================== FIRST TRY STORED CHANNEL =======================================================
    #ifdef DEBUG
      Serial.printf("[%s]: [SAVED CHANNEL] trying channel = %d \n",__func__,channel);
    #endif
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    #ifdef DEBUG
      Serial.printf("[%s]: [SAVED CHANNEL] sending took=%dms\n",__func__,millis()-s1);
    #endif
    unsigned long loop_timer=millis(); while(millis() < loop_timer + 10) {}  // unblocking loop for 10ms
    if(espnow_data_sent) 
    {
      // #ifdef DEBUG
        Serial.printf("[%s]:\n [SAVED CHANNEL] esp_now_send SUCCESSFUL on receiver=%d, channel=%d, MAC=%s; it took=%dms\n",__func__,receiver,channel,receiver_mac,millis()-s1);
      // #endif

      // save successful data
      g_wifi_channel = channel;
      g_last_gw = receiver;
      #ifdef DEBUG
        Serial.printf("[%s]: [ALL CHANNELS] Configuration data stored: receiver=%d, channel=%d\n",__func__,receiver,channel);
      #endif
      loop_timer=millis(); while(millis() < loop_timer + 1) {}  // unblocking loop for 1ms
      // save successful data END
      return true;
    }
    Serial.printf("[%s]: [SAVED CHANNEL] esp_now_send FAILED on channel %d, it took=%dms\n",__func__,channel,millis()-s1);
    // fist attempt: on saved channel END
// ======================================================== FIRST TRY STORED CHANNEL END =======================================================

// ======================================================== LOOP THROUGH ALL CHANNELS ==========================================================
    // second attempt: all channels
    #ifdef DEBUG
      Serial.printf("[%s]: [ALL CHANNELS] trying all channels now \n",__func__);
    #endif
    for (int i = 1; i <= 13; i++)
    {
      esp_wifi_set_channel(i, WIFI_SECOND_CHAN_NONE);
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      #ifdef DEBUG
        Serial.printf("[%s]: [ALL CHANNELS] sending took=%dms\n",__func__,millis()-s1);
      #endif
      unsigned long loop_timer=millis(); while(millis() < loop_timer + 30) {}  // unblocking loop 
      if(espnow_data_sent) 
      {
        // #ifdef DEBUG
          Serial.printf("[%s]:\n [ALL CHANNELS] esp_now_send SUCCESSFUL on receiver=%d, channel=%d, MAC=%s; it took=%dms\n",__func__,receiver,i,receiver_mac,millis()-s1);
        // #endif
        // save successful data
        g_wifi_channel = i;
        g_last_gw = receiver;
        #ifdef DEBUG
          Serial.printf("[%s]: [ALL CHANNELS] Configuration data stored: receiver=%d, channel=%d\n",__func__,receiver,i);
        #endif
        loop_timer=millis(); while(millis() < loop_timer + 1) {}  // unblocking loop for 1ms
        // save successful data END
        return true;
      }
      #ifdef DEBUG
        Serial.printf("[%s]: [ALL CHANNELS] FAILED on channel=%d, it took=%dms\n",__func__,i,millis()-s1);
      #endif
    }
  }
// ======================================================== LOOP THROUGH ALL CHANNELS END ======================================================

// NOTHING WORKED...
  Serial.printf("[%s]: esp_now_send TOTALLY FAILED, it took=%dms\n",__func__,millis()-s1);
  set_error_red_led_level(1);
  delay(100);
  return false;
// ======================================================== LOOP THROUGH ALL RECEIVERS END =======================================================
}


void setup_CP_Server()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    const char *index_html = cp_html_page.c_str();
    request->send_P(200, "text/html", index_html);
    Serial.printf("[%s]: Client Connected\n",__func__);
  });
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request)
  {
    String inputMessage;
    String inputParam;

    if (request->hasParam("ssid"))
    {
      inputMessage = request->getParam("ssid")->value();
      inputParam = "ssid";
      ssid = inputMessage;
      Serial.printf("[%s]: SSID received: %s\n",__func__,ssid);
      if (strlen(ssid.c_str()) == 0)
      {
        ssid = old_ssid;
        Serial.printf("[%s]: SSID is blank, reusing the old value: %s\n",__func__,ssid);
      } else
      {
        ssid = inputMessage;
      }
      valid_ssid_received = true;
    }

    if (request->hasParam("password"))
    {
      inputMessage = request->getParam("password")->value();
      inputParam = "password";
      password = inputMessage;
      Serial.printf("[%s]: password received: ",__func__);Serial.println(password);
      if (strlen(password.c_str()) == 0)
      {
        password = old_password;
        Serial.printf("[%s]: password is blank, reusing the old value: ",__func__); Serial.println(password);
      } else
      {
        password = inputMessage;
      }
      valid_password_received = true;
    }


    if (request->hasParam("sleeptime_s_str"))
    {
      inputMessage = request->getParam("sleeptime_s_str")->value();
      inputParam = "sleeptime_s_str";
      sleeptime_s_str = inputMessage;
      Serial.printf("[%s]: sleeptime_s_str received: %s\n",__func__,sleeptime_s_str);
      uint16_t old_sleeptime_s = sleeptime_s_str.toInt();
      if ((strlen(sleeptime_s_str.c_str()) == 0) or (old_sleeptime_s <= 0))
      {
        sleeptime_s_str = String(SLEEP_TIME_S);
        Serial.printf("[%s]: sleeptime_s_str is INCORRECT (%ss), reusing the default value: %ss\n",__func__,inputMessage,sleeptime_s_str);
      } else
      {
        sleeptime_s_str = inputMessage;
      }
      valid_sleeptime_s_str_received = true;
    }

    request->send(200, "text/html", "The values entered by you have been successfully sent to the device. It will now attempt WiFi connection");
  });
}


void WiFiSoftAPSetup()
{
  char cp_ssid[32];
  snprintf(cp_ssid,sizeof(cp_ssid),"%s",HOSTNAME);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(cp_ssid);
  Serial.printf("[%s]: AP IP address: ",__func__);
  Serial.println(WiFi.softAPIP());
  Serial.printf("[%s]: SSID: %s\n",__func__,cp_ssid);
  Serial.printf("[%s]: NO PASSWORD\n",__func__);
}


void WiFiStationSetup(String rec_ssid, String rec_password, String rec_sleep_s)
{
  wifi_timeout = false;
  WiFi.mode(WIFI_STA);
  char ssid_arr[33];
  char password_arr[65];
  int sleeptime_s;
  rec_ssid.toCharArray(ssid_arr, rec_ssid.length() + 1);
  rec_password.toCharArray(password_arr, rec_password.length() + 1);

  sleeptime_s = rec_sleep_s.toInt();

  #ifdef DEBUG
    Serial.printf("[%s]: Received SSID: %s, password: %s, sleeping time: %d \n",__func__,ssid_arr,password_arr, sleeptime_s);
  #endif

  if ((sleeptime_s > 0) and (sleeptime_s <= (24* 3600)))
  {
     Serial.printf("[%s]: Valid sleeptime: %d seconds\n", __func__,sleeptime_s);
  } else
  {
    Serial.printf("[%s]: NOT valid sleeptime: %d, using default: %d seconds\n", __func__,sleeptime_s, (SLEEP_TIME_S));
  }
  WiFi.begin(ssid_arr, password_arr);
  Serial.printf("[%s]: Connecting...\n",__func__);
  uint32_t t1 = millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(20);
    // Serial.print(".");
    if (millis() - t1 > WAIT_FOR_WIFI * 1000)
    {
      Serial.println();
      Serial.printf("[%s]: Timeout connecting to WiFi. The SSID or Password seem incorrect\n",__func__);
      valid_ssid_received = false;
      valid_password_received = false;
      valid_sleeptime_s_str_received = false;
      is_setup_done = false;
      StartCaptivePortal();
      wifi_timeout = true;
      break;
    }
  }
  if (!wifi_timeout)
  {
    is_setup_done = true;
    Serial.printf("\n[%s]: WiFi connected to: %s\n",__func__,rec_ssid);
    valid_ssid_received = false;
    Serial.printf("[%s]: STA IP address: ",__func__);
    Serial.println(WiFi.localIP());
    if( led_blink_handle != NULL )
    {
      Serial.printf("[%s]: Disabling blinking LEDs\n",__func__);
      vTaskDelete( led_blink_handle );
      delay(5);
    } else
    {
      Serial.printf("[%s]: LEDs still bllinking or were never blinking\n",__func__);
    }
    Serial.printf("[%s]: Done\n",__func__);
    snprintf(g_wifi_ssid,sizeof(g_wifi_ssid),"%s",ssid_arr);
    snprintf(g_wifi_password,sizeof(g_wifi_password),"%s",password_arr);
    if (sleeptime_s > 0) g_sleeptime_s = sleeptime_s; else g_sleeptime_s = SLEEP_TIME_S;
    g_wifi_ok = 1;
  }
}


void StartCaptivePortal() 
{
  // blink LEDs in task, if not yet blinking
  #if defined(ERROR_RED_LED_GPIO) 
    if (led_blink_handle == NULL)
    {
      xReturned_led_blink = xTaskCreate(led_blink, "led_blink", 2000, NULL, 1, &led_blink_handle);
      if( xReturned_led_blink != pdPASS )
      {
        Serial.printf("[%s]: CANNOT create led_blink task\n",__func__);
      } else
      {
        #ifdef DEBUG
          Serial.printf("[%s]: Task led_blink created\n",__func__);
        #endif
      }
    } else
    // blink LEDs already blinking
    {
      Serial.printf("[%s]: Task led_blink ALREADY created\n",__func__);
    }

    // create CP timer if not yet created
    if (cp_timer_handle  == NULL)
    {
      cp_timer_handle = xTimerCreate("MyTimer", pdMS_TO_TICKS(CP_TIMEOUT_S * 1000), pdTRUE, ( void * )id, &cp_timer);
      if( xTimerStart(cp_timer_handle, 10 ) != pdPASS )
      {
        Serial.printf("[%s]: CP timer start error\n",__func__);
      } else
      {
        #ifdef DEBUG
          Serial.printf("[%s]: CP timer STARTED\n",__func__);
        #endif
      }
    } else
    // CP timer created so restart it
    {
      if( xTimerReset( cp_timer_handle, 0 ) != pdPASS )
      {
        Serial.printf("[%s]: CP timer was not reset\n",__func__);
      } else
      {
        Serial.printf("[%s]: CP timer RE-STARTED\n",__func__);
      }
    }
  #endif
  Serial.printf("[%s]: Setting up AP Mode\n",__func__);

  WiFiSoftAPSetup();

  Serial.printf("[%s]: Setting up Async WebServer\n",__func__);
  setup_CP_Server();
  Serial.printf("[%s]: Starting DNS Server\n",__func__);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);             //only when requested from AP
  server.begin();
  dnsServer.processNextRequest();
  Serial.printf("[%s]: Captive Portal and AP created, timeout set to %ds\n",__func__,CP_TIMEOUT_S);
}


void cp_timer( TimerHandle_t cp_timer_handle )
{
  Serial.printf("[%s]: Captive Portal timer expired, RESTARTING...\n",__func__);
  // do_esp_restart();      // panicking in save_config()
  ESP.restart();
}


void connect_wifi()
{
  if (g_wifi_ok == 1) is_setup_done = true; else is_setup_done = false;
  ssid = String(g_wifi_ssid);
  password = String(g_wifi_password);
  if (g_sleeptime_s > 0) sleeptime_s_str = String(g_sleeptime_s); else sleeptime_s_str = String(SLEEP_TIME_S);

  if (!is_setup_done)
  {
    StartCaptivePortal();
  }
  else
  {
    Serial.printf("[%s]: Using saved SSID and PASS to attempt WiFi Connection...\n",__func__);
    Serial.printf("[%s]: SSID: ",__func__);Serial.println(ssid);
    Serial.printf("[%s]: PASS: ",__func__);Serial.println(password);
    Serial.printf("[%s]: Sleep time : ",__func__);Serial.println(sleeptime_s_str);
    WiFiStationSetup(ssid, password, sleeptime_s_str);
  }

  while (!is_setup_done)
  {
    dnsServer.processNextRequest();
    delay(10);
    if (valid_ssid_received && valid_password_received)
    {
      Serial.printf("[%s]: Attempting WiFi Connection...\n",__func__);
      WiFiStationSetup(ssid, password, sleeptime_s_str);
    }
  }
  Serial.printf("[%s]: Done\n",__func__);
}


void led_blink(void *pvParams)
{
  int delay_ms = 60;
  #if defined(ERROR_RED_LED_GPIO) and defined(ACT_BLUE_LED_GPIO)
    while(1)
    {
      // both low
      set_error_red_led_level(0);
      set_act_blue_led_level(0);

      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);

      // red high
      set_error_red_led_level(1);

      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);

      // both low
      set_error_red_led_level(0);
      set_act_blue_led_level(0);
      
      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);

      // blue high
      set_act_blue_led_level(1);

      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);
    }
  #else 
    while (1) 
    {
      set_error_red_led_level(0);
      vTaskDelay(delay_ms/portTICK_RATE_MS);
      set_error_red_led_level(1);
      vTaskDelay(delay_ms/portTICK_RATE_MS);
    }
  #endif
}


void led_blink_erase(void *pvParams)
{
  int delay_ms = 20;
    while(1)
    {
      //  low
      set_error_red_led_level(0);

      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);

      // red high
      set_error_red_led_level(1);

      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);
    }
}


void erase_all_data()
{
  // blink LEDs in task, if not yet blinking
  #if defined(ERROR_RED_LED_GPIO)
    if (led_blink_handle == NULL)
    {
      xReturned_led_blink = xTaskCreate(led_blink_erase, "led_blink_erase", 2000, NULL, 1, &led_blink_handle);
      if( xReturned_led_blink != pdPASS )
      {
        Serial.printf("[%s]: CANNOT create led_blink task\n",__func__);
      } else
      {
        #ifdef DEBUG
          Serial.printf("[%s]: Task led_blink created\n",__func__);
        #endif
      }
    } else
    // blink LEDs already blinking
    {
      Serial.printf("[%s]: Task led_blink ALREADY created\n",__func__);
    }
  #endif

  if (!LittleFS.format()) 
  {
    Serial.printf("[%s]: LittleFS.format FAILED\n",__func__);
  } else 
  {
    Serial.printf("[%s]: LittleFS.format SUCCEESFUL\n",__func__);
  }
  delay(20);  

  if (nvs_flash_erase() != ESP_OK) 
  {
    Serial.printf("[%s]: nvs_flash_erase FAILED\n",__func__);
  } else 
  {
    Serial.printf("[%s]: nvs_flash_erase SUCCEESFUL\n",__func__);
  }
  
  delay(20);


  if (nvs_flash_init() != ESP_OK) 
  {
    Serial.printf("[%s]: nvs_flash_init FAILED\n",__func__);
  } else 
  {
    Serial.printf("[%s]: nvs_flash_init SUCCEESFUL\n",__func__);
  }
  delay(20);
}


void do_esp_go_to_sleep()
{
  save_config("Config saved before going to sleep");
  Serial.printf("[%s]: Bye...\n========= E N D =========\n",__func__);
  esp_deep_sleep_start();
}


bool load_config()
{
  // function_start = micros();
  if(!LittleFS.begin(true))
  {   
      Serial.printf("[%s]: LITTLEFS Mount Failed\n",__func__);
      // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
      return false;
  } else
  {
    // cr_printFile(config_file);
    // cr_listDir(LittleFS,"/",1);
    #ifdef ENCRYPT_CONFIG
      if (!loadEncConfigFile(config_file, config))        //ciphered config file
    #else
      cr_printFile(config_file);
      if (!loadPlainConfigFile(config_file, config))      //plain config file
    #endif
      {
        Serial.printf("[%s]: loadEncConfigFile FAILED, using default values\n",__func__);
        g_sleeptime_s = SLEEP_TIME_S;
        g_bootCount = 0;
        g_saved_ontime_ms = 0;
        g_wifi_channel = 0;
        g_wifi_ok = 0; 
        g_last_gw = 0; 
        g_led_pwm = 122;

        snprintf(g_wifi_ssid,sizeof(g_wifi_ssid),"%s","default_ssid");
        snprintf(g_wifi_password,sizeof(g_wifi_password),"%s","default_password");
      } else 
      {
        #ifdef DEBUG
          Serial.printf("[%s]: loadEncConfigFile SUCCESSFUL\n",__func__);
        #endif
      }
  }
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
  return true;
}


// save config
void save_config(const char* reason)
{
  uint16_t extra_reset_time;
  // function_start = micros();

  // bootCount
  if ((boot_reason == 1) or (boot_reason == 3))         // reset or power ON
  {
    g_bootCount = 1;
    extra_reset_time = ESP32_BOOT_TIME_EXTRA;
  } else 
  {
    ++g_bootCount;
    extra_reset_time = 0;
  }

  // ontime
  uint32_t work_time = millis() - program_start_time + (ESP32_BOOT_TIME) + (ESP32_TAIL_TIME) + extra_reset_time;
  g_saved_ontime_ms = g_saved_ontime_ms + work_time;
  if (charging_int != 0) 
  {
    #ifdef DEBUG
      Serial.printf("[%s]: charging int=%d\n",__func__,charging_int);
    #endif
    g_saved_ontime_ms = 0; // reset on charging
  }
  Serial.printf("[%s]: Program finished after %lums.\n",__func__,work_time);
  
  // testing with PPK2 - end save ontime
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,LOW);
  #endif
  
  #ifdef ENCRYPT_CONFIG
    saveEncConfigFile(config_file, config, reason);           // ciphered
  #else
    savePlainConfigFile(config_file, config, reason);         // plain
  #endif
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
}


void get_old_wifi_credentials()
{
  Serial.printf("[%s]: Storring old configuration data...\n",__func__);

  old_ssid = g_wifi_ssid;
  old_password = g_wifi_password;
  old_sleeptime_s_str = g_sleeptime_s;

  Serial.printf("[%s]: old_ssid:            %s\n",__func__,old_ssid);
  Serial.printf("[%s]: old_password:        ",__func__); Serial.println(old_password);
  Serial.printf("[%s]: old_sleeptime_s_str: %s seconds\n",__func__,old_sleeptime_s_str);

  Serial.printf("[%s]: Resetting wifi credentials...\n",__func__);
  snprintf(g_wifi_ssid,sizeof(g_wifi_ssid),"%s","default_ssid");
  snprintf(g_wifi_password,sizeof(g_wifi_password),"%s","default_password");
  g_sleeptime_s = SLEEP_TIME_S;

}

void change_mac()
{
  // Serial.printf("[%s]: DEVICE_ID=%d\n",__func__,DEVICE_ID);
  char hexString[7];
  uint8_t byte5 = (DEVICE_ID / 100);
  uint8_t byte6 = (DEVICE_ID % 100);

  #ifdef DEBUG
    Serial.printf("[%s]: DEVICE_ID=%d, changing byte [5] to: %d\n",__func__,DEVICE_ID,byte6);
  #endif
  snprintf(hexString,sizeof(hexString),"0x%d",byte6);
  FixedMACAddress[5] = strtol(hexString, 0, 16);            // change only bit [5]

  if (DEVICE_ID > 99)              
  {
    #ifdef DEBUG
      Serial.printf("[%s]: DEVICE_ID=%d (>99) so changing also byte [4] to: %d\n",__func__,DEVICE_ID,byte5);
    #endif
    snprintf(hexString,sizeof(hexString),"0x%d",byte5);
    FixedMACAddress[4] = strtol(hexString, 0, 16);          // change also bit [4]
  }

  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac_org);
  snprintf(mac_org_char, sizeof(mac_org_char), "%02x:%02x:%02x:%02x:%02x:%02x",mac_org[0], mac_org[1], mac_org[2], mac_org[3], mac_org[4], mac_org[5]);

  esp_err_t mac_changed = esp_wifi_set_mac(WIFI_IF_STA, &FixedMACAddress[0]);

  WiFi.macAddress(mac_new);
  snprintf(mac_new_char, sizeof(mac_new_char), "%02x:%02x:%02x:%02x:%02x:%02x",mac_new[0], mac_new[1], mac_new[2], mac_new[3], mac_new[4], mac_new[5]);
  
  if (mac_changed == ESP_OK) 
  {
    // #ifdef DEBUG
      Serial.printf("[%s]: OLD MAC: %s, NEW MAC: %s\n",__func__,mac_org_char,mac_new_char);
    // #endif
  } else 
  {
    Serial.printf("[%s]: Changing MAC FAILED!\n|",__func__);
  }

}
// ========================================================================== FUNCTIONS implementation END

void initiate_all_leds()
{
  #ifdef ACT_BLUE_LED_GPIO
    // PWM
    #if (ACT_BLUE_LED_GPIO_USE_PWM == 1)
      ledcSetup(ACT_BLUE_LED_PWM_CHANNEL, ACT_BLUE_LED_PWM_FREQ, ACT_BLUE_LED_PWM_RESOLUTION);
      ledcAttachPin(ACT_BLUE_LED_GPIO, ACT_BLUE_LED_PWM_CHANNEL);
    // or fixed
    #else
      pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
    #endif
  #endif  

  #ifdef ERROR_RED_LED_GPIO
    // PWM
    #if (ERROR_RED_LED_GPIO_USE_PWM == 1)
      ledcSetup(ERROR_RED_LED_PWM_CHANNEL, ERROR_RED_LED_PWM_FREQ, ERROR_RED_LED_PWM_RESOLUTION);
      ledcAttachPin(ERROR_RED_LED_GPIO, ERROR_RED_LED_PWM_CHANNEL);
    // or fixed
    #else
      pinMode(ERROR_RED_LED_GPIO, OUTPUT);
    #endif
  #endif  
}

void set_act_blue_led_level(u_int8_t level)  
{
  #ifdef ACT_BLUE_LED_GPIO
    #if (ACT_BLUE_LED_GPIO_USE_PWM == 1)
      if (level == 1)
      {
        ledcWrite(ACT_BLUE_LED_PWM_CHANNEL, g_led_pwm);
        #ifdef DEBUG
          Serial.printf("[%s]: ACT_BLUE_LED_GPIO DC set to: %d\n",__func__,g_led_pwm);
        #endif
      } else 
      {
        ledcWrite(ACT_BLUE_LED_PWM_CHANNEL, 0);
        #ifdef DEBUG
          Serial.printf("[%s]: ACT_BLUE_LED_GPIO DC set to: 0\n",__func__);
        #endif
      }
    #else 
      if (level == 1)
      {
        digitalWrite(ACT_BLUE_LED_GPIO,HIGH);
        #ifdef DEBUG
          Serial.printf("[%s]: ACT_BLUE_LED_GPIO set to: HIGH\n",__func__);
        #endif
      } else 
      {
        digitalWrite(ACT_BLUE_LED_GPIO,LOW);
        #ifdef DEBUG
          Serial.printf("[%s]: ACT_BLUE_LED_GPIO set to: LOW\n",__func__);
        #endif
      }
    #endif
  #endif
}

void set_error_red_led_level(u_int8_t level)  
{
  u_int8_t red_level;
  #ifdef ERROR_RED_LED_GPIO
    #if (ERROR_RED_LED_GPIO_USE_PWM == 1)
      if (level == 1)
      {
        if ((g_led_pwm *2) <= 255)
          red_level = g_led_pwm *2;
        else 
          red_level = g_led_pwm;
        ledcWrite(ERROR_RED_LED_PWM_CHANNEL, red_level);
        #ifdef DEBUG
          Serial.printf("[%s]: ERROR_RED_LED_GPIO DC set to: %d\n",__func__,red_level);
        #endif
      } else 
      {
        ledcWrite(ERROR_RED_LED_PWM_CHANNEL, 0);
        #ifdef DEBUG
          Serial.printf("[%s]: ERROR_RED_LED_GPIO DC set to: 0\n",__func__);
        #endif
      }
    #else 
      if (level == 1)
      {
          digitalWrite(ERROR_RED_LED_GPIO,HIGH);
          #ifdef DEBUG
            Serial.printf("[%s]: ERROR_RED_LED_GPIO set to: HIGH\n",__func__);
          #endif
      } else 
      {
        digitalWrite(ERROR_RED_LED_GPIO,LOW);
        #ifdef DEBUG
          Serial.printf("[%s]: ERROR_RED_LED_GPIO set to: LOW\n",__func__);
        #endif
      }
    #endif
  #endif
}


void setup()
{
  program_start_time = millis();
  // testing with PPK2 - start
  #ifdef PPK2_GPIO
    pinMode(PPK2_GPIO,OUTPUT);
    digitalWrite(PPK2_GPIO,HIGH);
  #endif

  // start LEDs  
  initiate_all_leds();

  Serial.begin(115200);
  Serial.printf("\n======= S T A R T =======\n");
  Serial.printf("[%s]: Device: %s, hostname=%s, version=%s, sensor type=%s, MCU type=%s\n",__func__,DEVICE_NAME,HOSTNAME,ZH_PROG_VERSION, sender_type_char[SENSOR_TYPE],MODEL);

  // check if device is charging
  snprintf(charging,4,"%s","N/A");
  #if (defined(CHARGING_GPIO) and defined(POWER_GPIO))
    pinMode(CHARGING_GPIO,INPUT_PULLUP);
    pinMode(POWER_GPIO,INPUT_PULLUP);
    #ifdef DEBUG
      Serial.printf("[%s]: CHARGING_GPIO and POWER_GPIO enabled\n",__func__);
    #endif
    charging_int=charging_state();
  #else
    // snprintf(charging,4,"%s","N/A");
    #ifdef DEBUG
      Serial.printf("[%s]: checking CHARGING DISABLED\n",__func__);
    #endif
  #endif

  //Initialize NVS or erase it if not initialised (that usually never happens)
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // apply new MAC address
  change_mac(); // it provides: mac_org_char,  mac_new_char changed globally

  // power for sensors from GPIO - MUST be before any I2C sensor is in use obviously!
  #ifdef ENABLE_3V_GPIO
    pinMode(ENABLE_3V_GPIO,OUTPUT);
    #ifdef DEBUG
      Serial.printf("[%s]: Enabling 3V to power sensors on GPIO=%d\n",__func__,ENABLE_3V_GPIO);
    #endif
    digitalWrite(ENABLE_3V_GPIO, HIGH);
    while (!digitalRead(ENABLE_3V_GPIO)){}
    delay(10);
  #endif
  

  // start Wire after enabling 3.3V
  #if defined(CUSTOM_SDA_GPIO) and defined(CUSTOM_SCL_GPIO)
    #ifdef DEBUG
      Serial.printf("[%s]: CUSTOM I2C GPIO pins applied: SDA=%d, SCL=%d\n",__func__,CUSTOM_SDA_GPIO,CUSTOM_SCL_GPIO);
    #endif
    Wire.setPins(CUSTOM_SDA_GPIO,CUSTOM_SCL_GPIO);
  #endif
  Wire.begin();
  delay(1);  

  // ciphered config
  cipher->setKey(cipher_key);     //for encryption/decryption of config file
// load config file
  if (!load_config())
  {
    Serial.printf("[%s]: Loading config file FAILED, restarting\n",__func__);
    do_esp_restart();
  } else 
  {
    #ifdef DEBUG
      Serial.printf("[%s]: Loading config file SUCCESSFUL\n",__func__);
    #endif
  }

  // just check if WiFi is configured - start ERROR LED if not
  if (g_wifi_ok != 1) 
  {
    Serial.printf("[%s]: WiFi not configured\n",__func__);
    set_error_red_led_level(1);
  } else 
  {
    #ifdef DEBUG
      Serial.printf("[%s]: WiFi seems OK\n",__func__);
    #endif
  }

  // read_saved_sleeping_time();
  boot_reason = esp_reset_reason();
  wakeup_reason = esp_sleep_get_wakeup_cause();


  Serial.printf("[%s]: Boot cause=%d - ",__func__,boot_reason);
  switch(boot_reason)
  {
    // 1 = reset/power on
    case ESP_RST_POWERON:
    {
      Serial.printf("power on or reset\n");
      set_act_blue_led_level(1);
      set_error_red_led_level(1);
      break;
    }
    // 3 = Software reset via esp_restart
    case ESP_RST_SW:
    {
      Serial.printf("Software reset via esp_restart\n");
      set_act_blue_led_level(1);
      set_error_red_led_level(1);
      break;
    }
    // 8 = deep sleep
    case ESP_RST_DEEPSLEEP:
    {
      Serial.printf("wake up from deep sleep \n");
      break;
    }
    default:
    {
      Serial.printf("other boot cause=%d\n");
      break;
    }
  }



  Serial.printf("[%s]: Wakeup cause=%d - ",__func__,wakeup_reason);
  switch(wakeup_reason)
  {
    // 0 = not deep sleep
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    {
      Serial.printf("wake up was not caused by exit from deep sleep\n");
      break;
    }
    // 2 = not in use
    case ESP_SLEEP_WAKEUP_EXT0:
    {
      Serial.printf("external signal using RTC_IO (motion detected)\n");
      break;
    }
    // 3 = fw update (FW_UPGRADE_GPIO) or motion detected (MOTION_SENSOR_GPIO) - not for ESP32-C3!
    #if (BOARD_TYPE != 4)
      case ESP_SLEEP_WAKEUP_EXT1:
      {
        wakeup_gpio_mask = esp_sleep_get_ext1_wakeup_status();
        wakeup_gpio = log(wakeup_gpio_mask)/log(2);
        Serial.printf("external signal using GPIO=%d, GPIO_MASK=%ju\n",wakeup_gpio,wakeup_gpio_mask);
        set_error_red_led_level(1);
        #ifdef FW_UPGRADE_GPIO
          if (wakeup_gpio == FW_UPGRADE_GPIO)
          {
            fw_update = true;
            Serial.printf("[%s]: woke up on FW button pressed\n",__func__);
            Serial.printf("[%s]: debouncing for %dms\n",__func__,DEBOUNCE_MS);
            delay(DEBOUNCE_MS);
          }
        #endif
        #ifdef MOTION_SENSOR_GPIO
          if (wakeup_gpio == MOTION_SENSOR_GPIO)
          {
            motion = true;
          }
          break;
        #endif
      }
    #endif
    // 4 = wake up on timer (SLEEP_TIME_S or COOLING_SLEEP_DURATION_S)
    case ESP_SLEEP_WAKEUP_TIMER:
    {
      Serial.printf("timer (cooling or heartbeat)\n");
      set_act_blue_led_level(1);
      break;
    }
    // 5 = not in use
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
    {
      Serial.printf("WAKEUP_TOUCHPAD\n");
      break;
    }
    // 6 = not in use
    case ESP_SLEEP_WAKEUP_ULP:
    {
      Serial.printf("WAKEUP_ULP\n");
      break;
    }
    // 7 = for ESP32-C3 - ESP_SLEEP_WAKEUP_GPIO,for others it is from light sleep only (ESP32, S2 and S3) so not programmed
    #if (BOARD_TYPE == 4)
      case ESP_SLEEP_WAKEUP_GPIO:
      {
        wakeup_gpio_mask = esp_sleep_get_gpio_wakeup_status();
        wakeup_gpio = log(wakeup_gpio_mask)/log(2);
        Serial.printf("external signal using GPIO=%d, GPIO_MASK=%ju\n",wakeup_gpio,wakeup_gpio_mask);
        set_error_red_led_level(1);
        #ifdef FW_UPGRADE_GPIO
          if (wakeup_gpio == FW_UPGRADE_GPIO)
          {
            fw_update = true;
            Serial.printf("[%s]: woke up on FW button pressed\n",__func__);
            Serial.printf("[%s]: debouncing for %dms\n",__func__,DEBOUNCE_MS);
            delay(DEBOUNCE_MS);
          }
        #endif
        #ifdef MOTION_SENSOR_GPIO
          if (wakeup_gpio == MOTION_SENSOR_GPIO)
          {
            motion = true;
          }
          break;
        #endif
        break;
      }
    #endif
    default:
    {
      Serial.printf("OTHER Wakeup cause\n");
      break;
    }
  }


  // resest on long, 3s press of FW_UPGRADE_GPIO button
  #ifdef FW_UPGRADE_GPIO
    // RED LED ON, BLUE OFF if FW_UPGRADE_GPIO was pressed
    if (fw_update)
    {
      set_act_blue_led_level(0);
      set_error_red_led_level(1);
    }
    // check if FW_UPGRADE_GPIO is still high after boot
    pinMode(FW_UPGRADE_GPIO,INPUT);
    if ((digitalRead(FW_UPGRADE_GPIO)) and (fw_update))
    {
      Serial.printf("[%s]: FW_UPGRADE_GPIO still HIGH after boot\n",__func__);
      Serial.printf("[%s]: Waiting for 3s...\n",__func__);
      delay(3000);
      if (digitalRead(FW_UPGRADE_GPIO))
      {
        // first 3s done
        Serial.printf("[%s]: 3s passed\n",__func__);
        Serial.printf("[%s]: FW_UPGRADE_GPIO HIGH still\n",__func__);

        set_error_red_led_level(0);
        delay(50);
        set_act_blue_led_level(1);
        Serial.printf("[%s]: Cancelling FW upgrade\n",__func__);
        fw_update = false;
        Serial.printf("[%s]: Next delay for 3s\n",__func__);
        delay(3000);
        if (digitalRead(FW_UPGRADE_GPIO))
        {
          // second 3s done so 6s in total
          set_error_red_led_level(1);
          delay(50);
          set_act_blue_led_level(0);
          Serial.printf("[%s]: Erasing ALL stored data on ESP\n",__func__);
          erase_all_data();
          Serial.printf("[%s]: RESTARTING ESP\n\n\n",__func__);
          // do_esp_restart();
          ESP.restart();  // restart without saving data!
        } else
        {
          // only first 3s done
          Serial.printf("[%s]: Long press but <6s - Starting Captive Portal...\n",__func__);
          get_old_wifi_credentials();
          g_wifi_ok = 0;
          connect_wifi();
          Serial.printf("[%s]: Restarting...\n",__func__);
          do_esp_restart();

        }
      } else
      // FW_UPGRADE_GPIO LOW after 3s - FW upgrade here
      {
        Serial.printf("[%s]: FW_UPGRADE_GPIO LOW after 3s - continuing normal boot and applying FW upgrade\n",__func__);
      }
    } else
    {
      // very short press, not detected AFTER boot - no FW update just normal, forced run
      fw_update = false;
      #ifdef DEBUG
        Serial.printf("[%s]: FW_UPGRADE_GPIO LOW after boot - no FW update scheduled - normal forced run\n",__func__);
      #endif
    }
  #endif

  #ifdef MOTION_SENSOR_GPIO
    if (motion)
    {
      set_error_red_led_level(1);
      Serial.printf("[%s]: motion DETECTED\n",__func__);
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: motion CLEARED\n",__func__);
      #endif
    }
  #endif

// MAX17048 - fuel gauge
  #if (USE_MAX17048 == 1)
    
    #ifdef DEBUG
      lipo.enableDebugging();
      Serial.printf("[%s]: start USE_MAX17048\n",__func__);
    #endif
    if (! lipo.begin())
    {
      // #ifdef DEBUG
        Serial.printf("[%s]: MAX17048 NOT detected ... Check your wiring or I2C ADDR!\n",__func__);
      // #endif
      max17ok = false;
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: start MAX17048 OK\n",__func__);
      #endif

      // reset MAX17048 if NOT woke up from deep sleep and apply MAX17048_DELAY_ON_RESET_MS
      if (boot_reason != 8)
      {
        // #ifdef DEBUG
          Serial.printf("[%s]: Resetting MAX17048 and applying delay for %dms\n",__func__,MAX17048_DELAY_ON_RESET_MS);
        // #endif
        lipo.reset();
        delay(MAX17048_DELAY_ON_RESET_MS);
      }

    }
  #else
    #ifdef DEBUG
      Serial.printf("[%s]: DONT USE_MAX17048\n",__func__);
    #endif
  #endif

// Firmware update
// print only in DEBUG otherwise security breach
  #ifdef DEBUG
    char firmware_file[255];
    #ifndef CLIENT
      // UPDATE_FIRMWARE_HOST is in local network
      snprintf(firmware_file,sizeof(firmware_file),"%s/01-Production/0-ESPnow/firmware/%s.bin",UPDATE_FIRMWARE_HOST,HOSTNAME);
    #else
      // UPDATE_FIRMWARE_HOST is on internet
      snprintf(firmware_file,sizeof(firmware_file),"%s/%s.bin",UPDATE_FIRMWARE_HOST,HOSTNAME);
    #endif
  Serial.printf("[%s]: firmware file:\n %s\n",__func__,firmware_file);
  #endif
// Firmware update END

//lux
  #if (USE_TSL2561 == 1)
    #ifdef DEBUG
      Serial.printf("[%s]: start USE_TSL2561\n",__func__);
    #endif
    if (! light.begin())  // it does NOT return false so this checking is meaningless I think
    {
      #ifdef DEBUG
        Serial.printf("[%s]: TSL2561  NOT detected ... Check your wiring or I2C ADDR!\n",__func__);
      #endif
      tslok = false;
    }
    else
    {
      if (light.setPowerUp())
      {
        #ifdef DEBUG
          Serial.printf("[%s]: tslok =%d\n",__func__,tslok);
        #endif
      } else
      {
        Serial.printf("[%s]: TSL2561  NOT detected ... Check your wiring or I2C ADDR!\n",__func__);
      }
    }
  #else
    #ifdef DEBUG
      Serial.printf("[%s]: DONT USE_TSL2561\n",__func__);
    #endif
  #endif

//sht31
  #if (USE_SHT31 == 1)
    #ifdef DEBUG
      Serial.printf("[%s]: start USE_SHT31\n",__func__);
    #endif
    if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
      // #ifdef DEBUG
        Serial.printf("[%s]: SHT31    NOT detected ... Check your wiring or I2C ADDR!\n",__func__);
      // #endif
      sht31ok = false;
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: sht31ok =%d\n",__func__,sht31ok);
      #endif
    }
  #else
    #ifdef DEBUG
      Serial.printf("[%s]: DONT USE_SHT31\n",__func__);
    #endif
  #endif

// gather data
gather_data();
// off 3V
#ifdef ENABLE_3V_GPIO
  #ifdef DEBUG
    Serial.printf("[%s]: Disabling 3V on GPIO=%d\n",__func__,ENABLE_3V_GPIO);
  #endif
  digitalWrite(ENABLE_3V_GPIO, LOW);
#endif
#ifdef DEBUG
  em = millis(); tt = em - program_start_time;
  Serial.printf("[%s]: gathering data took: %dms\n",__func__,tt);
#endif

 Serial.printf("[%s]: Temp=%0.2fC, Hum=%0.2f%%, batpct=%0.2f%%, charging=%s, ontime=%us\n",__func__,myData.temp,myData.hum,myData.batpct,myData.charg,myData.ontime);

// turn off power for sensors - all code below is only valid if gather_data() was successfull
  #ifdef ENABLE_3V_GPIO
    digitalWrite(ENABLE_3V_GPIO, LOW);
  #endif

  if (send_data())
  {
    #ifdef DEBUG
      Serial.printf("[%s]: Sendig data SUCCESSFUL\n",__func__);
    #endif
  } else 
  {
    Serial.printf("[%s]: Sendig data FAILED\n",__func__);
  }

  // wait for command from gateway
  #ifdef DEBUG
    Serial.printf("[%s]: Waiting for command from gateway...\n",__func__);
  #endif
  // unsigned long t_wait_answer_start = millis();
  u_int32_t start_waiting = micros();
  while(!command_received && micros() <= start_waiting + (WAIT_FOR_COMMAND_MS * 1000))
  {
    // delayMicroseconds(1);
  }
  if (command_received) 
  {
    #ifdef DEBUG
      Serial.printf("[%s]: Received command from gateway = %d\n",__func__,data_recv.command); 
    #endif
    // act on commands here
    // OTA
    if (data_recv.command == 1) 
    {
      fw_update = true;
      Serial.printf("[%s]: Received command from gateway to perform fw update\n",__func__);
    }
    else 
    // RESTART
    if (data_recv.command == 2) 
    {
      Serial.printf("[%s]: Received command from gateway to perform RESTART\n",__func__);
      // reset bootcount, it will increase to 1 on saving config
      g_bootCount = 0;
      do_esp_restart();
    }   
    else 
    // CP
    if (data_recv.command == 3) 
    {
      Serial.printf("[%s]: Received command from gateway TO start CAPTIVE PORTAL\n",__func__);
      get_old_wifi_credentials();
      g_wifi_ok = 0;
      connect_wifi();
      Serial.printf("[%s]: Restarting...\n",__func__);
      do_esp_restart();
    }   
    else 
    // Factory
    if (data_recv.command == 4) 
    {
      Serial.printf("[%s]: Received command from gateway to perform FACTORY RESET\n",__func__);
      Serial.printf("[%s]: Erasing ALL stored data on ESP\n",__func__);
      erase_all_data();
      Serial.printf("[%s]: RESTARTING ESP\n\n\n",__func__);
      ESP.restart();  // restart without saving data!
    }  
    else    
    // Motion OFF
    if (data_recv.command == 10) 
    {
      #ifdef DEBUG
        Serial.printf("[%s]: Received command from gateway to turn OFF motion\n",__func__);
      #endif
      g_motion = 0;
      do_esp_restart();
    }  
    else 
    // Motion ON
    if (data_recv.command == 11) 
    {
      #ifdef DEBUG
        Serial.printf("[%s]: Received command from gateway to turn ON motion\n",__func__);
      #endif
      g_motion = 1;
      do_esp_restart();
    }      
    else 
    // gw1
    if (data_recv.command == 21) 
    {
      Serial.printf("[%s]: Received command from gateway to attach sensor to gateway 1\n",__func__);
      if (NUMBER_OF_GATEWAYS > 1) // only if there is more than 1 gw, otherwise it makes no sense
      {
        g_last_gw = 0;
      }
    }  
    else  
    // gw2
    if (data_recv.command == 22) 
    {
      Serial.printf("[%s]: Received command from gateway to attach sensor to gateway 2\n",__func__);
      if (NUMBER_OF_GATEWAYS > 1) // only if there is more than 1 gw, otherwise it makes no sense
      {
        g_last_gw = 1;
      }
    }  
    else 
    // gw3
    if (data_recv.command == 23) 
    {
      Serial.printf("[%s]: Received command from gateway to attach sensor to gateway 3\n",__func__);
      if (NUMBER_OF_GATEWAYS > 2) // only if there is more than 2 gw, otherwise it makes no sense
      {
        g_last_gw = 2;
      }
    }  
    else  
    // LEDs OFF   
    if (data_recv.command == 200) 
    {
      Serial.printf("[%s]: Received command from gateway to perform set LED PMW DC to 0\n",__func__);
      g_led_pwm = 0;
    }     
    else 
    // LEDs ON
    if (data_recv.command == 201) 
    {
      Serial.printf("[%s]: Received command from gateway to perform set LED PMW DC to 255\n",__func__);
      g_led_pwm = 255;
    }                       
  } 
  else 
  {
    Serial.printf("[%s]: Command from gateway NOT received within %dms\n",__func__,WAIT_FOR_COMMAND_MS);
  }

  #ifdef DEBUG
    u_int32_t end_waiting = micros();
    u_int32_t waiting_time = end_waiting - start_waiting;
    Serial.printf("[%s]: Waiting for command from gateway took: %luus\n",__func__,waiting_time);
  #endif 

  disable_espnow();

  if (fw_update)
  {
    Serial.printf("[%s]: FW update requested\n",__func__);
    connect_wifi();
    do_update();
  }

  // eventually: go to sleep ;-)
  if (g_sleeptime_s > 0) sleeptime_s = g_sleeptime_s;
  hibernate(false, sleeptime_s);
}


// loop is empty - sleep is the only option (or restart if ESPnow is not working)
void loop(){}
