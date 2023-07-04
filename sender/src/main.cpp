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
#include <ESPmDNS.h>

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
  #include <Wire.h>
  #include <Adafruit_Sensor.h>
  #include <Adafruit_TSL2561_U.h>
  Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
  #include "measure-lux.h"
#endif

// temp from MAX31855 - temp sensor, SPI
#if (USE_MAX31855 == 1)
  #include <Adafruit_MAX31855.h>
  Adafruit_MAX31855 thermocouple(CLK_GPIO, CS_GPIO, MISO_GPIO);
#endif

#if (TOUCHPAD_ONLY == 1)
  #include "driver/touch_pad.h"
  #include "soc/rtc_periph.h"
  #include "soc/sens_periph.h"
  #include "driver/touch_sensor.h"
  #include "driver/touch_sensor_common.h"
  #include "soc/touch_sensor_channel.h"
  #include "hal/touch_sensor_types.h"
#endif

#if (USE_DALLAS_18B20 == 1)
  #include "OneWireNg_CurrentPlatform.h"
  #include "drivers/DSTherm.h"
  #include "utils/Placeholder.h"
  static Placeholder<OneWireNg_CurrentPlatform> _ow;

  #include "measure-ds18b20.h"
#endif

// LCD
#if (USE_ST7735_160_80_ALI == 1)
  #warning "building with LCD"
  #include <TFT_eSPI.h> 
  #include <SPI.h>
  TFT_eSPI tft = TFT_eSPI();  // Invoke library
  TFT_eSprite img = TFT_eSprite(&tft);
  #ifndef ST7735_160_80_ALI
    #error "wrong display configured- check TFT_eSPI library and use the one with ST7735_160_80_ALI in it"
  #endif 
#endif

#if (USE_ADC == 1)
  #include "measure-volts-adc.h"
  double get_volts(int pin, int attennuation, int iteration, float resistors_calibration); 
  #if (BATTERY_FROM_ADC == 1)
    uint8_t get_bat_pcnt (double volts);
  #endif  
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

// OTA web server
bool ota_web_server_needed = false;
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer ota_web_server(OTA_WEB_SERVER_PORT);


// MAX17048 - battery fuel gauge, I2C
#if (USE_MAX17048 == 1)
  SFE_MAX1704X lipo(MAX1704X_MAX17048);
#endif

// sht31 - temperature and humidity, I2C
#if (USE_SHT31 == 1)
  Adafruit_SHT31 sht31 = Adafruit_SHT31();
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
  uint8_t wifi_ok;                      // 0 - wifi not configured, 1 - wifi configured
  uint8_t motion_enabled;               // 0 - motion disabled, 1 - motion enabled
  uint8_t light_high_sensitivity;       // 0 - low, light meas. time 13ms, 1 - high, light meas. time 403ms
  uint8_t button_pressed = 0;           // 0 = none, >0 = the button number as per button_gpio[NUMBER_OF_BUTTONS] - NOT GPIO NUMBER! index starts from 1
  uint16_t working_time_ms;             // last working time in ms
  uint16_t sleep_time_s;                // 
  uint8_t valid = 1;                    // make it invalid in case some info is missing, incorrect or flagged, don't publish to HA if invalid
} struct_message;

struct_message myData;

// device type - this array can expand [first dimension] but the max size of string is 9 characters - this text goes to myData.sender_type over ESPnow
const char sender_type_char[5][10] = {"env", "motion", "env+mot","battery","push_b"};

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
bool max31855ok = true;

char charging[5];       // global as assigned in setup() - to avoid calling again
uint8_t charging_int = 0;       // by default NC

// config file
const char *config_file = "/config.json";
Config config;


//ciphering
char* cipher_key = (char*)CIPHER_KEY;   

// for power saving features
bool power_savings_cpu_used = false;


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
bool connect_wifi();
void setup_CP_Server();
void WiFiSoftAPSetup();
void WiFiStationSetup(String rec_ssid, String rec_password, String rec_sleep_s);
void StartCaptivePortal();
void led_blink(void *pvParams);
void led_blink_erase(void *pvParams);
void cp_timer( TimerHandle_t cp_timer_handle );
void get_old_wifi_credentials();

// lux
void get_lux(char* lux_char);  

// dummy for touchpad
void tp_callback();

// ota web server
void start_web_server();
void stop_cp_timer();
void delete_cp_timer();
void start_cp_timer(int16_t timeout);

// dallas
#if (USE_DALLAS_18B20 == 1)
  static bool printId(const OneWireNg::Id& id);
  void get_all_ds18b20_addresses();
  float get_ds18b20();
#endif  

// ========================================================================== FUNCTIONS declaration END

// ========================================================================== FUNCTIONS implementation

void stop_cp_timer()
{
    if( xTimerStop( cp_timer_handle, 0 ) != pdPASS )
    {
      Serial.printf("[%s]: Timer was NOT stopped\n",__func__);
    } else
    {
      Serial.printf("[%s]: Timer stopped\n",__func__);
    }
}

void delete_cp_timer()
{
    if( xTimerDelete( cp_timer_handle, 0 ) != pdPASS )
    {
      Serial.printf("[%s]: Timer was NOT deleted\n",__func__);
    } else
    {
      Serial.printf("[%s]: Timer deleted\n",__func__);
      cp_timer_handle  = NULL;
    }
}

void start_cp_timer(int16_t timeout)
{
  Serial.printf("[%s]: starting with timeout=%d\n",__func__,timeout);
      // create CP timer if not yet created
  if (cp_timer_handle  == NULL)
  {
    cp_timer_handle = xTimerCreate("MyTimer", pdMS_TO_TICKS(timeout * 1000), pdTRUE, ( void * ) 0, cp_timer);
    if( xTimerStart(cp_timer_handle, 10 ) != pdPASS )
    {
      Serial.printf("[%s]: Timer start error\n",__func__);
    } else
    {
    //   #ifdef DEBUG
        Serial.printf("[%s]: Timer STARTED for %d seconds\n",__func__,timeout);
    //   #endif
    }
  } else
  {
    stop_cp_timer();
    delete_cp_timer();
    start_cp_timer(timeout);
  }
}



// ota web server
void start_web_server()
{
  #define OTA_USER "admin"
  #define OTA_PASSWORD "password"
  Serial.printf("[%s]: Enabling OTA on port=%d...\n",__func__,OTA_WEB_SERVER_PORT);
  Serial.printf("[%s]: user=%s, password=%s\n",__func__,OTA_USER,OTA_PASSWORD);
  Serial.printf("[%s]: Go to: http://%s:%d/update\n",__func__,WiFi.localIP().toString(),OTA_WEB_SERVER_PORT);
  ota_web_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String introtxt = "This is " + String(HOSTNAME)+ ", device ID="+String(DEVICE_ID) +", Version=" + String(ZH_PROG_VERSION) + ", MAC=" + String(WiFi.macAddress());
    request->send(200, "text/plain", String(introtxt));
  });
  AsyncElegantOTA.begin(&ota_web_server, OTA_USER,OTA_PASSWORD);    // Start ElegantOTA
  ota_web_server.begin();
}

// dummy for touchpad
void tp_callback()
{
  // it is executed only when touch is detected AND esp32 is awake - not when it slept
}

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
  esp_wifi_stop();                                                        // wifi - big impact on current: from 115mA to 42mA
  esp_deep_sleep_disable_rom_logging();                                   // it does not display welcome message - shorter time to wake up

  // these DON'T interfere with touchpad 
  // these DON'T improve sleeping current
  // these make RTC unusable
        // esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
        // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M,        ESP_PD_OPTION_OFF);
        // esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,      ESP_PD_OPTION_OFF);
        // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
        // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
        // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);


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
  uint64_t bitmask_dec = 0;

  #if (PUSH_BUTTONS == 1)
    for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
      bitmask_dec = bitmask_dec + pow(2, button_gpio[i]);
    }
    #ifdef DEBUG
      Serial.printf("[%s]: without FW_UPGRADE_GPIO: bitmask_dec=%ju\n",__func__,bitmask_dec);
    #endif
    #ifdef FW_UPGRADE_GPIO
      bitmask_dec = bitmask_dec + pow(2, FW_UPGRADE_GPIO);
    #endif
    #ifdef DEBUG
      Serial.printf("[%s]: with FW_UPGRADE_GPIO: bitmask_dec=%ju\n",__func__,bitmask_dec);
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

    esp_sleep_enable_timer_wakeup(final_sleeping_time_s * uS_TO_S_FACTOR);
    
    do_esp_go_to_sleep();
  #endif

  #if (TOUCHPAD_ONLY == 1)
    uint16_t touch_value;
    uint16_t thr;
    uint32_t pad_intr;
    #define TOUCH_THRESH_NO_USE   (0)

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TOUCHPAD)
    {
      Serial.printf("[%s]: sleeping before measuring for=%dms\n",__func__,TOUCHPAD_COOLTIME_MS);
      delay(TOUCHPAD_COOLTIME_MS);
    }

    if (touch_pad_init() != ESP_OK) Serial.printf("touch_pad_init ERROT\n");
    if (touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER) != ESP_OK) Serial.printf("touch_pad_set_fsm_mode ERROR\n");
    if (touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V) != ESP_OK) 
    {
      Serial.printf("touch_pad_set_voltage ERROR\n");
      Serial.printf("TRIED: refh=%d, refl=%d, atten=%d\n",TOUCH_HVOLT_MAX, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_MAX);
      touch_high_volt_t refh;
      touch_low_volt_t refl;
      touch_volt_atten_t atten;
      touch_pad_get_voltage(&refh, &refl,&atten);
      Serial.printf("GOT: refh=%d, refl=%d, atten=%d\n",refh, refl, atten);
    }

    for (int i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
      if (touch_pad_config((touch_pad_t)button_gpio[i], TOUCH_THRESH_NO_USE) != ESP_OK) Serial.printf("touch_pad_config ERROR\n");
    }

    if (touch_pad_filter_start(TOUCHPAD_MEASUREMENTS_MS) != ESP_OK) Serial.printf("touch_pad_filter_start ERROR\n");

    for (int i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
      if (touch_pad_read_filtered((touch_pad_t)button_gpio[i], &touch_value) != ESP_OK) Serial.printf("touch_pad_read_filtered ERROR\n");
      thr = touch_value * (float)touchpad_thr_multiplier[i];
      // #ifdef DEBUG
        Serial.printf("touch pad [%d] val is %d, setting threshold=%d (touchpad_thr_multiplier=%0.2f)\n", i, touch_value,thr,touchpad_thr_multiplier[i]);
      // #endif
      if (touch_pad_config((touch_pad_t)button_gpio[i], thr) != ESP_OK) Serial.printf("touch_pad_config ERROR\n"); 
    }
  
    #ifdef DEBUG
      uint16_t clock_cycle;
      uint16_t interval_cycle;
      uint16_t sleep_cycle;
      uint16_t meas_cycle;
      touch_pad_t touch_num;
      uint16_t threshold;
      touch_trigger_mode_t mode;
      touch_trigger_src_t src;
      uint32_t p_period_ms;

      // touch_pad_get_measurement_clock_cycles(&clock_cycle);
      // touch_pad_get_measurement_interval(&interval_cycle);
      touch_pad_get_meas_time(&sleep_cycle, &meas_cycle);
      touch_pad_get_thresh(touch_num, &threshold);
      touch_pad_get_trigger_mode(&mode);
      touch_pad_get_trigger_source(&src);
      touch_pad_get_filter_period(&p_period_ms);

      Serial.printf("[%s]: sleep_cycle=%d\n",__func__,sleep_cycle);
      Serial.printf("[%s]: meas_cycle=%d\n",__func__,meas_cycle);

      Serial.printf("[%s]: mode=%d\n",__func__,mode);
      Serial.printf("[%s]: src=%d\n",__func__,src);
      Serial.printf("[%s]: p_period_ms=%d\n",__func__,p_period_ms);
    #endif

    touch_pad_clear_status();

    if (touch_pad_intr_enable() != ESP_OK) Serial.printf("touch_pad_intr_enable ERROR\n");
    if (NUMBER_OF_BUTTONS > 0)  esp_sleep_enable_touchpad_wakeup();
    esp_sleep_enable_timer_wakeup(final_sleeping_time_s * uS_TO_S_FACTOR);
    do_esp_go_to_sleep();
  #endif

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
      #ifdef DEBUG_LIGHT
        Serial.printf("[%s]: going to sleep until next motion detected\n",__func__);
        Serial.printf("[%s]:  or for %ds (heartbeat)\n",__func__,final_sleeping_time_s);
      #endif
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
    #ifdef DEBUG_LIGHT
      Serial.printf("[%s]: going to sleep for %ds (heartbeat)\n",__func__,final_sleeping_time_s);
    #endif
  #endif
  // testing with PPK2 - go to sleep
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,HIGH);
  #endif

  // the next 2 lines are NOT working - CP cannot start! ????
  // esp_sleep_config_gpio_isolate();

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
  #ifdef DEBUG
    unsigned long function_start = micros(); 
  #endif

  // testing with PPK2 - start gather data
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,LOW);
  #endif

  // valid measurements
  myData.valid = g_valid;  
  #ifdef DEBUG
    Serial.printf("[%s]: myData.valid=%d\n",__func__,myData.valid);
  #endif
  if (myData.valid == 0) Serial.printf("[%s]: Data is marked INVALID(%d)! - it will NOT go to Home Assistant from Receiver\n",__func__,myData.valid);
  
  // button pressed
  #if ((PUSH_BUTTONS == 1) or (TOUCHPAD_ONLY))
    if (button_pressed > 0)
    {
      myData.button_pressed = button_pressed;
    }
  #endif
  #ifdef DEBUG
    Serial.printf("[%s]: myData.button_pressed=%d\n",__func__,myData.button_pressed);
  #endif

  // sender_type
  snprintf(myData.sender_type,sizeof(myData.sender_type),"%s",sender_type_char[SENSOR_TYPE]);
  #ifdef DEBUG
    Serial.printf("[%s]: myData.sender_type=%s\n",__func__,myData.sender_type);
  #endif

  // wifi_ok
  myData.wifi_ok = g_wifi_ok;
  #ifdef DEBUG
    Serial.printf("[%s]: myData.wifi_ok=%d\n",__func__,myData.wifi_ok);
  #endif

  // lux high sensitivity
  myData.light_high_sensitivity = g_lux_high_sens;
  #ifdef DEBUG
    Serial.printf("[%s]: myData.light_high_sensitivity=%d\n",__func__,myData.light_high_sensitivity);
  #endif

  // motion enabled/disable
  #ifdef MOTION_SENSOR_GPIO
    myData.motion_enabled = g_motion;
  #else
    myData.motion_enabled = 0;
  #endif
  #ifdef DEBUG
    Serial.printf("[%s]: myData.motion_enabled=%d\n",__func__,myData.motion_enabled);
  #endif

  // hostname
  strcpy(myData.host, HOSTNAME);
  #ifdef DEBUG
    Serial.printf("[%s]: myData.host=%s\n",__func__,myData.host);
  #endif

  // name
  strcpy(myData.name, DEVICE_NAME);
  #ifdef DEBUG
    Serial.printf("[%s]: myData.name=%s\n",__func__,myData.name);
  #endif

  // motion
  myData.motion = motion;
  #ifdef DEBUG
    Serial.printf("[%s]: myData.motion=%d\n",__func__,myData.motion);
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

  // MAX31855
  #if (USE_MAX31855 == 1)
    if (max31855ok)
    {
      delay(USE_MAX31855_DELAY_MS);
      double c = thermocouple.readCelsius();
      #ifdef DEBUG
        Serial.printf("[%s]: USE_MAX31855: First check after delay for %dms, temp=%0.2f\n",__func__,USE_MAX31855_DELAY_MS,c);
      #endif
      if (isnan(c) or (c == 0))
      {
        for (uint8_t i=0; i < 10; i++)
        {
          #ifdef DEBUG
            Serial.printf("[%s]: USE_MAX31855: repetition %d, delay for %dms, temp=%0.2f\n",__func__,i,USE_MAX31855_DELAY_MS,c);
          #endif 
          delay(USE_MAX31855_DELAY_MS);
          c = thermocouple.readCelsius();
          if (isnan(c) or (c == 0)) 
            continue;
          else
          {
            myData.temp = c;
            break;
          }
        }
      } else
      {
        myData.temp = c;
      }
    }
  #endif
  
  // DALLAS_18B20
  #if (USE_DALLAS_18B20 == 1) 
    #ifndef CALIBRATE_TEMPERATURE
      #define CALIBRATE_TEMPERATURE       0   
    #endif
    myData.temp = get_ds18b20() + (CALIBRATE_TEMPERATURE);
    #ifdef DEBUG
      Serial.printf("[%s]: myData.temp=%0.2f\n",__func__,myData.temp);
    #endif
  #endif

  #ifdef DEBUG
    Serial.printf("[%s]: myData.temp=%0.2f\n",__func__,myData.temp);
    Serial.printf("[%s]: myData.hum=%0.2f\n",__func__,myData.hum);
  #endif

  if ((myData.temp < -33) and (myData.temp > -34))
  {
    Serial.printf("[%s]: TEMPERATURE ERROR!\n",__func__);
  } 

  // lux
  myData.lux = 0.0f;
  #if (USE_TSL2561 == 1)
    char lux_ch[10];
    get_lux(lux_ch);
    float lux = atof(lux_ch);
    myData.lux = lux;
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
    Serial.printf("[%s]: myData.lux=%0.2f\n",__func__,myData.lux);
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
          Serial.printf("[%s]: Battery level OK\n",__func__);
        #endif
      }
    }
  #endif

  #if (BATTERY_FROM_ADC == 1)
    myData.bat = get_volts(ADC_GPIO, ADC_ATTEN, 250, ADC_DIVIDER);
    myData.batpct = get_bat_pcnt(myData.bat);
  #endif
  

  #ifdef DEBUG
    Serial.printf("[%s]: myData.bat=%0.2f\n",__func__,myData.bat);
    Serial.printf("[%s]: myData.batpct=%0.2f\n",__func__,myData.batpct);
    Serial.printf("[%s]: myData.batchr=%0.2f\n",__func__,myData.batchr);
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
    Serial.printf("[%s]: myData.ver=%s\n",__func__,myData.ver);
  #endif

  // charging - already gathered in setup()
  snprintf(myData.charg,5,"%s",charging);
  #ifdef DEBUG
    Serial.printf("[%s]: myData.charg=%s\n",__func__,myData.charg);
  #endif

  // bootCount calculated in save_config() + 1
  // myData.boot = g_bootCount;

  ++g_bootCount;
  myData.boot = g_bootCount;
  #ifdef DEBUG
    Serial.printf("[%s]: myData.boot=%d\n",__func__,myData.boot);
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
    Serial.printf("[%s]: myData.boardtype=%d\n",__func__,myData.boardtype);
  #endif

// working_time_ms
  myData.working_time_ms = g_last_working_time_ms;
  #ifdef DEBUG
    Serial.printf("[%s]: working_time_ms %ums\n",__func__,myData.working_time_ms);
  #endif

// sleep_time_s
  myData.sleep_time_s = g_sleeptime_s;
  #ifdef DEBUG
    Serial.printf("[%s]: sleep_time_s %ds\n",__func__,myData.sleep_time_s);
  #endif

  #ifdef DEBUG
    Serial.printf("[%s]: Total struct size=%d bytes\n",__func__,sizeof(myData));
  #endif

  // testing with PPK2 - end gather_data
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,HIGH);
  #endif

  #ifdef DEBUG
    Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
  #endif
}


// send data to gateway over ESPnow
bool send_data()     
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
    #ifdef DEBUG_LIGHT
      Serial.printf("[%s]:\n [SAVED CHANNEL, SAVED RECEIVER] esp_now_send SUCCESSFUL on channel=%d, receiver=%d, MAC=%s; it took=%dms\n",__func__,channel,last_gw,receiver_mac,millis()-s1);
    #endif
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
      Serial.printf("[%s]: password received: %s\n",__func__,password.c_str());
      if (strlen(password.c_str()) == 0)
      {
        password = old_password;
        Serial.printf("[%s]: password is blank, reusing the old value: %s\n",__func__,password.c_str()); 
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
  Serial.printf("[%s]: AP IP address: %s\n",__func__,WiFi.softAPIP().toString().c_str());

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

  // THIS SETS PROPERLY THE HOSTNAME - WiFi.setHostname(HOSTNAME); is NOT WORKING - sender
  esp_netif_t *esp_netif = NULL;
  esp_netif = esp_netif_next(esp_netif);
  esp_netif_set_hostname(esp_netif, HOSTNAME);

  WiFi.begin(ssid_arr, password_arr);
  Serial.printf("[%s]: Connecting...\n",__func__);
  uint32_t t1 = millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(20);
    if (millis() - t1 > WAIT_FOR_WIFI * 1000)
    {
      Serial.printf("\n[%s]: Timeout connecting to WiFi. The SSID or Password seem incorrect\n",__func__);
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
    Serial.printf("[%s]: STA IP address: %s\n",__func__,WiFi.localIP().toString().c_str());

    if(!MDNS.begin(HOSTNAME))
    {
      Serial.printf("[%s]: Error starting MDNS\n",__func__);
    } else
    {
      Serial.printf("[%s]: MDNS started with hostname: %s\n",__func__,HOSTNAME);
    } 

    if( led_blink_handle != NULL )
    {
      Serial.printf("[%s]: Disabling blinking LEDs\n",__func__);
      vTaskDelete( led_blink_handle );
      delay(5);
    } else
    {
      // Serial.printf("[%s]: LEDs still bllinking or were never blinking\n",__func__);
      Serial.printf("[%s]: LEDs were never blinking\n",__func__);
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
  #endif
  
  // create CP timer if not yet created
  start_cp_timer(CP_TIMEOUT_S);

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
  Serial.printf("[%s]: Timer expired, RESTARTING...\n",__func__);
  // do_esp_restart();      // panicking in save_config()
  ESP.restart();
}


bool connect_wifi()
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

    Serial.printf("[%s]: SSID: %s\n",__func__,g_wifi_ssid);                     
    Serial.printf("[%s]: PASS: %s\n",__func__,g_wifi_password);                
    Serial.printf("[%s]: Sleep time: %s\n",__func__,sleeptime_s_str);   
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
  save_config("wifi connected");
  return true;
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
  
  #if (PUSH_BUTTONS == 1)
    if (button_pressed)
    {
      Serial.printf("[%s]: debouncing for %dms\n",__func__,DEBOUNCE_MS_PUSHBUTTON);
      delay(DEBOUNCE_MS_PUSHBUTTON);
    }
  #endif

  // #ifdef DEBUG_LIGHT
    Serial.printf("[%s]: millis=%lums, Bye...\n========= E N D =========\n",__func__,millis());
  // #endif
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
        g_last_working_time_ms = 0;

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
    // g_bootCount = 1;
    g_bootCount = 0;
    extra_reset_time = ESP32_BOOT_TIME_EXTRA;
  } else 
  {
    // ++g_bootCount;
    extra_reset_time = 0;
  }

  // ontime
  // add extra 350ms if CPU reduced
  #if ((POWER_SAVINGS_CPU == 1) and (power_savings_cpu_used))
    uint32_t work_time = millis() - program_start_time + (ESP32_BOOT_TIME) + (ESP32_TAIL_TIME) + extra_reset_time + POWER_SAVINGS_CPU_TAIL_EXTRA_MS;
  #else
    uint32_t work_time = millis() - program_start_time + (ESP32_BOOT_TIME) + (ESP32_TAIL_TIME) + extra_reset_time;
  #endif 
  
  g_last_working_time_ms = work_time;
  #ifdef DEBUG
    Serial.printf("[%s]: millis=%d, program_start_time=%d, ESP32_BOOT_TIME=%d, ESP32_TAIL_TIME=%d, extra_reset_time=%d,  Program finished after %lums (adjusted).\n",__func__,millis(),program_start_time, ESP32_BOOT_TIME,ESP32_TAIL_TIME,extra_reset_time, work_time);
  #endif
  g_saved_ontime_ms = g_saved_ontime_ms + work_time;
  // also used charging_int = 10 to reset ontime
  if (charging_int != 0) 
  {
    #ifdef DEBUG
      Serial.printf("[%s]: charging int=%d\n",__func__,charging_int);
    #endif
    g_saved_ontime_ms = 0; // reset on charging
  }
  
  #ifndef DEBUG
    Serial.printf("[%s]: millis=%lums, Program finished after %lums (adjusted).\n",__func__,millis(), work_time);
  #endif
  
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
  Serial.printf("[%s]: old_password:        %s\n",__func__,g_wifi_password); 
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
    #ifdef DEBUG_LIGHT
      Serial.printf("[%s]: OLD MAC: %s, NEW MAC: %s\n",__func__,mac_org_char,mac_new_char);
    #endif
    // #endif
  } else 
  {
    Serial.printf("[%s]: Changing MAC FAILED!\n|",__func__);
  }

}
// ========================================================================== FUNCTIONS implementation END

void initiate_all_leds()
{
  // 700us
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
  // 70us
  #ifdef ACT_BLUE_LED_GPIO
    #if (ACT_BLUE_LED_GPIO_USE_PWM == 1)
      if (level == 1)
      {
        ledcWrite(ACT_BLUE_LED_PWM_CHANNEL, g_led_pwm);
        #ifdef DEBUG
          // Serial.printf("[%s]: ACT_BLUE_LED_GPIO DC set to: %d\n",__func__,g_led_pwm);
        #endif
      } else 
      {
        ledcWrite(ACT_BLUE_LED_PWM_CHANNEL, 0);
        #ifdef DEBUG
          // Serial.printf("[%s]: ACT_BLUE_LED_GPIO DC set to: 0\n",__func__);
        #endif
      }
    #else 
      if (level == 1)
      {
        digitalWrite(ACT_BLUE_LED_GPIO,HIGH);
        #ifdef DEBUG
          // Serial.printf("[%s]: ACT_BLUE_LED_GPIO set to: HIGH\n",__func__);
        #endif
      } else 
      {
        digitalWrite(ACT_BLUE_LED_GPIO,LOW);
        #ifdef DEBUG
          // Serial.printf("[%s]: ACT_BLUE_LED_GPIO set to: LOW\n",__func__);
        #endif
      }
    #endif
  #endif
}

void set_error_red_led_level(u_int8_t level)  
{
  // 70us
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
          // Serial.printf("[%s]: ERROR_RED_LED_GPIO DC set to: %d\n",__func__,red_level);
        #endif
      } else 
      {
        ledcWrite(ERROR_RED_LED_PWM_CHANNEL, 0);
        #ifdef DEBUG
          // Serial.printf("[%s]: ERROR_RED_LED_GPIO DC set to: 0\n",__func__);
        #endif
      }
    #else 
      if (level == 1)
      {
          digitalWrite(ERROR_RED_LED_GPIO,HIGH);
          #ifdef DEBUG
            // Serial.printf("[%s]: ERROR_RED_LED_GPIO set to: HIGH\n",__func__);
          #endif
      } else 
      {
        digitalWrite(ERROR_RED_LED_GPIO,LOW);
        #ifdef DEBUG
          // Serial.printf("[%s]: ERROR_RED_LED_GPIO set to: LOW\n",__func__);
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

  Serial.begin(115200);
  delay(1);

  // start LEDs  
  initiate_all_leds();

  // #ifdef DEBUG_LIGHT
  Serial.printf("\n======= S T A R T =======\n");
  Serial.printf("[%s]: millis=%lums, Device: %s, hostname=%s, version=%s, sensor type=%s, MCU type=%s\n",__func__,program_start_time, DEVICE_NAME,HOSTNAME,ZH_PROG_VERSION, sender_type_char[SENSOR_TYPE],MODEL);
  // #endif

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
    while (!digitalRead(ENABLE_3V_GPIO)){delay(1);}
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

  // check if WiFi is configured - start ERROR LED if not
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
  #ifdef DEBUG_LIGHT
    Serial.printf("[%s]: Boot cause=%d - ",__func__,boot_reason);
  #endif
  switch(boot_reason)
  {
    // 1 = reset/power on
    case ESP_RST_POWERON:
    {
      Serial.printf("[%s]: power on or reset\n",__func__);
      set_act_blue_led_level(1);
      set_error_red_led_level(1);
      break;
    }
    // 3 = Software reset via esp_restart
    case ESP_RST_SW:
    {
      Serial.printf("[%s]: Software reset via esp_restart\n",__func__);
      set_act_blue_led_level(1);
      set_error_red_led_level(1);
      break;
    }
    // 8 = deep sleep
    case ESP_RST_DEEPSLEEP:
    {
      #ifdef DEBUG_LIGHT
        Serial.printf("[%s]: wake up from deep sleep \n",__func__);
      #endif
      break;
    }
    default:
    {
      Serial.printf("[%s]: other boot cause=%d\n",__func__);
      break;
    }
  }

  #ifdef DEBUG_LIGHT
    Serial.printf("[%s]: Wakeup cause=%d - ",__func__,wakeup_reason);
  #endif
  switch(wakeup_reason)
  {
    // 0 = not deep sleep
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    {
      Serial.printf("[%s]: wake up was not caused by exit from deep sleep\n",__func__);
      break;
    }
    // 2 = not in use
    case ESP_SLEEP_WAKEUP_EXT0:
    {
      Serial.printf("[%s]: external signal using RTC_IO (motion detected)\n",__func__);
      break;
    }
    // 3 = PUSH BUTTONS or fw update (FW_UPGRADE_GPIO) or motion detected (MOTION_SENSOR_GPIO) - not for ESP32-C3!
    #if (BOARD_TYPE != 4)
      case ESP_SLEEP_WAKEUP_EXT1:
      {
        Serial.printf("[%s]: debouncing for %dms\n",__func__,DEBOUNCE_MS_ANY_GPIO);
        delay(DEBOUNCE_MS_ANY_GPIO);
        wakeup_gpio_mask = esp_sleep_get_ext1_wakeup_status();
        wakeup_gpio = log(wakeup_gpio_mask)/log(2);
        Serial.printf("[%s]: external signal using GPIO=%d, GPIO_MASK=%ju\n",__func__,wakeup_gpio,wakeup_gpio_mask);
        set_error_red_led_level(1);
        #ifdef FW_UPGRADE_GPIO
          if (wakeup_gpio == FW_UPGRADE_GPIO)
          {
            fw_update = true;
            Serial.printf("[%s]: woke up on FW button pressed\n",__func__);
            break;
          }
        #endif
        #ifdef MOTION_SENSOR_GPIO
          if (wakeup_gpio == MOTION_SENSOR_GPIO)
          {
            motion = true;
            break;
          }
        #endif
        #if (PUSH_BUTTONS == 1)
          for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++)
          {
            if (wakeup_gpio == button_gpio[i])
            {
              button_pressed = i + 1;   // start counting buttons from 1 not from 0
              Serial.printf("[%s]: #button pressed=%d, button GPIO=%d, wakeup_gpio=%d\n",__func__,button_pressed,button_gpio[i],wakeup_gpio);
              break;
            }
          }
        #endif
      }
    #endif
    // 4 = wake up on timer (SLEEP_TIME_S or COOLING_SLEEP_DURATION_S)
    case ESP_SLEEP_WAKEUP_TIMER:
    {
      #ifdef DEBUG_LIGHT
        Serial.printf("timer (cooling or heartbeat)\n");
      #endif
      set_act_blue_led_level(1);
      break;
    }
    // 5 = not in use
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
    {
      #if (TOUCHPAD_ONLY == 1)
        touch_pad_t touchPin;
        #ifdef DEBUG
          Serial.printf("[%s]: wakeup_reason=WAKEUP_TOUCHPAD\n",__func__);
        #endif
        touchPin = esp_sleep_get_touchpad_wakeup_status();
        switch(touchPin)
        {
          case TOUCH_PAD_NUM0:
          {
            wakeup_gpio = 4;
            break;
          }
          case TOUCH_PAD_NUM1:
          {
            wakeup_gpio = 0;
            break;
          }
          case TOUCH_PAD_NUM2:
          {
            wakeup_gpio = 2;
            break;
          }
          case TOUCH_PAD_NUM3:
          {
            wakeup_gpio = 15;
            break;
          }
          case TOUCH_PAD_NUM4:
          {
            wakeup_gpio =13;
            break;
          }
          case TOUCH_PAD_NUM5:
          {
            wakeup_gpio = 12;
            break;
          }
          case TOUCH_PAD_NUM6:
          {
            wakeup_gpio = 14;
            break;
          }
          case TOUCH_PAD_NUM7:
          {
            wakeup_gpio = 27;
            break;
          }
          case TOUCH_PAD_NUM8:
          {
            wakeup_gpio = 33;
            break;
          }
          case TOUCH_PAD_NUM9:
          {
            wakeup_gpio = 32;
            break;
          }
          default : 
          {
            Serial.printf("[%s]: Wakeup NOT caused by touchpad\n",__func__); 
            wakeup_gpio = 255; // (error)
            break;
          }
        }    
        if (wakeup_gpio < 255)
        {
          set_error_red_led_level(1);
          for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++)
          {
            if (i == touchPin)
            {
              button_pressed = i + 1;   // start counting buttons from 1 not from 0
            }
          }
          Serial.printf("[%s]: wakeup TOUCH_PAD_NUM%d, GPIO=%d, button_pressed=%d\n",__func__,touchPin,wakeup_gpio,button_pressed);
        }
      #endif  
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
        Serial.printf("[%s]: debouncing for %dms\n",__func__,DEBOUNCE_MS_ANY_GPIO);
        delay(DEBOUNCE_MS_ANY_GPIO);
        wakeup_gpio_mask = esp_sleep_get_gpio_wakeup_status();
        wakeup_gpio = log(wakeup_gpio_mask)/log(2);
        Serial.printf("7: external signal using GPIO=%d, GPIO_MASK=%ju\n",wakeup_gpio,wakeup_gpio_mask);
        set_error_red_led_level(1);
        #ifdef FW_UPGRADE_GPIO
          if (wakeup_gpio == FW_UPGRADE_GPIO)
          {
            fw_update = true;
            Serial.printf("[%s]: woke up on FW button pressed\n",__func__);
            
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
      Serial.printf("[%s]: debouncing for %dms\n",__func__,DEBOUNCE_MS_FW_GPIO);
      delay(DEBOUNCE_MS_FW_GPIO);
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

// ===============  SENSORS INITIALISATION
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

//lux
  #if (USE_TSL2561 == 1)
    #ifdef DEBUG
      Serial.printf("[%s]: start USE_TSL2561\n",__func__);
    #endif
    if(!tsl.begin())
    {
      #ifdef DEBUG
        Serial.printf("[%s]: TSL2561  NOT detected ... Check your wiring or I2C ADDR!\n",__func__);
      #endif
      tslok = false;
    } else 
    {
      #ifdef DEBUG
        Serial.printf("[%s]: tslok =%d\n",__func__,tslok);
      #endif
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

//MAX31855
  #if (USE_MAX31855 == 1)
    #ifdef DEBUG
      Serial.printf("[%s]: start USE_MAX31855\n",__func__);
    #endif
    if (!thermocouple.begin()) {   // Set to 0x45 for alternate i2c addr
        Serial.printf("[%s]: MAX31855 NOT detected ... Check your wiring!\n",__func__);
      max31855ok = false;
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: max31855ok =%d\n",__func__,max31855ok);
      #endif
    }
  #else
    #ifdef DEBUG
      Serial.printf("[%s]: DONT USE_MAX31855\n",__func__);
    #endif
  #endif


  //18B20 Dallas OneWireNg Temperature sensors 
  #if (USE_DALLAS_18B20 == 1) 
    #ifdef DEBUG
      Serial.printf("[%s]: start USE_DALLAS_18B20\n",__func__);
    #endif
    new (&_ow) OneWireNg_CurrentPlatform(OW_PIN, false);
    DSTherm drv(_ow);
    #if (CONFIG_MAX_SRCH_FILTERS > 0)
      static_assert(CONFIG_MAX_SRCH_FILTERS >= DSTherm::SUPPORTED_SLAVES_NUM,
          "CONFIG_MAX_SRCH_FILTERS too small");
      drv.filterSupportedSlaves();
    #endif
    #ifdef COMMON_RES
      //Set common resolution for all sensors.
      drv.writeScratchpadAll(0, 0, COMMON_RES);
      drv.copyScratchpadAll(PARASITE_POWER);
    #endif
  #else
    #ifdef DEBUG
      Serial.printf("[%s]: DONT USE_DALLAS_18B20\n",__func__);
    #endif
  #endif

// ===============  SENSORS INITIALISATION END

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

// gather data
gather_data();

// off 3V
#ifdef ENABLE_3V_GPIO
  #ifdef DEBUG
    Serial.printf("[%s]: Disabling 3V on GPIO=%d\n",__func__,ENABLE_3V_GPIO);
  #endif
  digitalWrite(ENABLE_3V_GPIO, LOW);
#endif

#ifdef DEBUG_LIGHT
  Serial.printf("[%s]: Temp=%0.2fC, Hum=%0.2f%%, Light=%0.2flx (high sensitivity=%d), batpct=%0.2f%%, charging=%s, ontime=%us\n",__func__,myData.temp,myData.hum,myData.lux,g_lux_high_sens,myData.batpct,myData.charg,myData.ontime);
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

  // add checking if target state is already there, i.e. if motion disabled, don't disable again
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
    // lux high sensitivity
    if (data_recv.command == 5) 
    {
      Serial.printf("[%s]: Received command from gateway to set lux measurement to high sensitivity\n",__func__);
      g_lux_high_sens = 1;
      do_esp_restart();
    }  
    else 
    // lux low sensitivity
    if (data_recv.command == 6) 
    {
      Serial.printf("[%s]: Received command from gateway to set lux measurement to low sensitivity\n",__func__);
      g_lux_high_sens = 0;
      do_esp_restart();
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
        do_esp_restart();
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
        do_esp_restart();
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
        do_esp_restart();
      }
    }  
    else  
    // reset ontime using charging_int = 10 - workaround as charing_int > 0 will reset ontime in save_config()
    if (data_recv.command == 24) 
    {
      Serial.printf("[%s]: Received command from gateway to reset ontime\n",__func__);
      charging_int = 10;
    }  
    else 
    // SLEEP_TIME_S=1s   
    if (data_recv.command == 30) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 1s\n",__func__);
      g_sleeptime_s = 1;
      do_esp_restart();
    }     
    else
    // SLEEP_TIME_S=3s   
    if (data_recv.command == 31) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 3s\n",__func__);
      g_sleeptime_s = 3;
      do_esp_restart();
    }     
    else
    // SLEEP_TIME_S=5s   
    if (data_recv.command == 32) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 5s\n",__func__);
      g_sleeptime_s = 5;
      do_esp_restart();
    }     
    else
    // SLEEP_TIME_S=10s   
    if (data_recv.command == 33) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 10s\n",__func__);
      g_sleeptime_s = 10;
      do_esp_restart();
    }     
    else
    // SLEEP_TIME_S=15s   
    if (data_recv.command == 34) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 15s\n",__func__);
      g_sleeptime_s = 15;
      do_esp_restart();
    }     
    else    
    // SLEEP_TIME_S=30s   
    if (data_recv.command == 35) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 30s\n",__func__);
      g_sleeptime_s = 30;
      do_esp_restart();
    }     
    else
    // SLEEP_TIME_S=60s   
    if (data_recv.command == 36) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 60s\n",__func__);
      g_sleeptime_s = 60;
      do_esp_restart();
    }     
    else 
    // SLEEP_TIME_S=90s   
    if (data_recv.command == 37) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 90s\n",__func__);
      g_sleeptime_s = 90;
      do_esp_restart();
    }     
    else  
    // SLEEP_TIME_S=120s   
    if (data_recv.command == 38) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 120s\n",__func__);
      g_sleeptime_s = 120;
      do_esp_restart();
    }     
    else 
    // SLEEP_TIME_S=180s   
    if (data_recv.command == 39) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 180s\n",__func__);
      g_sleeptime_s = 180;
      do_esp_restart();
    }     
    else 
    // SLEEP_TIME_S=300s   
    if (data_recv.command == 40) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 300s\n",__func__);
      g_sleeptime_s = 300;
      do_esp_restart();
    }     
    else 
    // SLEEP_TIME_S=600s   
    if (data_recv.command == 41) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 600s\n",__func__);
      g_sleeptime_s = 600;
      do_esp_restart();
    }     
    else
    // SLEEP_TIME_S=900s   
    if (data_recv.command == 42) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 900s\n",__func__);
      g_sleeptime_s = 900;
      do_esp_restart();
    }     
    else  
    // SLEEP_TIME_S=1800s   
    if (data_recv.command == 43) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 1800s\n",__func__);
      g_sleeptime_s = 1800;
      do_esp_restart();
    }     
    else 
    // SLEEP_TIME_S=3600s   
    if (data_recv.command == 44) 
    {
      Serial.printf("[%s]: Received command from gateway to set sleeptime to 3600s\n",__func__);
      g_sleeptime_s = 3600;
      do_esp_restart();
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
    else 
    // ota web server
    if (data_recv.command == 202) 
    {
      Serial.printf("[%s]: Received command from gateway to start web server\n",__func__);
      ota_web_server_needed =  true;
    }        
    else 
    // invalid measurements - don't update HA
    if (data_recv.command == 203) 
    {
      Serial.printf("[%s]: Received command from gateway to make measurements INVALID\n",__func__);
      g_valid =  0;
      do_esp_restart();
    }  
    // valid measurements - update HA
    if (data_recv.command == 204) 
    {
      Serial.printf("[%s]: Received command from gateway to make measurements VALID\n",__func__);
      g_valid =  1;
      do_esp_restart();
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
    #if (USE_ST7735_160_80_ALI == 1) 
      pinMode(LCD_LED_GPIO, OUTPUT);
      #ifdef LCD_3V_GPIO
        pinMode(LCD_3V_GPIO,OUTPUT);
        #ifdef DEBUG
          Serial.printf("[%s]: Enabling 3V to power LCD on GPIO=%d\n",__func__,LCD_3V_GPIO);
        #endif
        digitalWrite(LCD_3V_GPIO, HIGH);
        while (!digitalRead(LCD_3V_GPIO)){delay(1);}
      #endif

      tft.init(); // it takes 1s for ST7735 !!!
      tft.fillScreen(TFT_BLACK);
      delay(50);  // this delay solves initial flicker
      digitalWrite(LCD_LED_GPIO,HIGH);

      tft.setRotation(SCREEN_ROTATION);
      tft.setTextPadding(TFT_WIDTH);
      tft.setTextDatum(TL_DATUM);
      tft.setTextColor(TFT_RED,TFT_BLACK);
      tft.drawString("FW update", 10, 30, 4);

    #endif
    Serial.printf("[%s]: FW update requested\n",__func__);
    connect_wifi();
    do_update();
    // restart is above so the below line should never be printed
    Serial.printf("[%s]: FW update finished - this should never be printed\n",__func__);
  }

  if (ota_web_server_needed)
  {
    command_received = false;
    Serial.printf("[%s]: OTA Web server requested\n",__func__);
    if (connect_wifi())
    {
      start_cp_timer(OTA_WEB_SERVER_TIMEOUT_S);
      start_web_server();
    }
  }  

  // eventually: go to sleep if OTA web servers is not needed ;-)
  if (g_sleeptime_s > 0) sleeptime_s = g_sleeptime_s;

// LCD
#if (USE_ST7735_160_80_ALI == 1) 
  // display temperature on: reset, power on, gpio short push AND if not OTA webserver needed!
  if (((boot_reason == 1) or (boot_reason == 3) or (wakeup_gpio == FW_UPGRADE_GPIO)) and (!ota_web_server_needed))
  {
    // power savign features: 
    // before savings: 
    //  - total time: 4900ms, avg current: 84mA = 411.6
    //  - tft.init = time: 1000ms, current: 76.3mA
    //  - display  = time: 3000ms, current: 89mA
    // tail: 345ms
    // WiFi OFF reduction: 
    //  - total time: 4900ms, avg current: 44.5mA = 218
    //  - tft.init = time: 1000ms, current: 26.7mA
    //  - display  = time: 3000ms, current: 41.25mA
    // tail: 345ms
    // WiFi OFF and 10MHz CPU instead of 240MHz reduction:
    //  - total time: 5400ms, avg current: 32.5mA = 175.5   // 5300, 31.6
    //  - tft.init = time: 1000ms, current: 8.8mA           // 1000, 8.7, 
    //  - display  = time: 3000ms, current: 23mA            // 3000, 23
    // tail: 346ms, 68.4mA

    // no LCD: 43.62mC
    // with LCD no savings: 411mC
    // with LCD no savings: 177mC

    #if (POWER_SAVINGS_WIFI == 1)
      WiFi.mode(WIFI_OFF);
    #endif
    #if (POWER_SAVINGS_CPU == 1)
      power_savings_cpu_used = true;
      #ifdef DEBUG
        Serial.printf("[%s]: power_savings_cpu_used=%d\n",__func__,power_savings_cpu_used);
        Serial.printf("[%s]: getXtalFrequencyMhz()=%d\n",__func__,getXtalFrequencyMhz());
        Serial.printf("[%s]: getCpuFrequencyMhz()=%d\n",__func__,getCpuFrequencyMhz());
        Serial.printf("[%s]: getApbFrequency()=%d\n",__func__,getApbFrequency());
      #endif

      Serial.printf("[%s]: getCpuFrequencyMhz()=%d\n",__func__,getCpuFrequencyMhz());

      // Serial has to start again after CPU frequency is changed
      Serial.flush();
      Serial.end();
      setCpuFrequencyMhz(10);
      Serial.begin(115200);

      Serial.printf("[%s]: getCpuFrequencyMhz()=%d\n",__func__,getCpuFrequencyMhz());

      #ifdef DEBUG
        Serial.printf("[%s]: getXtalFrequencyMhz()=%d\n",__func__,getXtalFrequencyMhz());
        Serial.printf("[%s]: getCpuFrequencyMhz()=%d\n",__func__,getCpuFrequencyMhz());
        Serial.printf("[%s]: getApbFrequency()=%d\n",__func__,getApbFrequency());
      #endif
    #endif

    // power savign features END

    Serial.printf("[%s]: DISPLAYING TEMPERATURE for %d seconds\n",__func__,LCD_SCREEN_TIME_S);
    pinMode(LCD_LED_GPIO, OUTPUT);
    #ifdef LCD_3V_GPIO
      pinMode(LCD_3V_GPIO,OUTPUT);
      #ifdef DEBUG
        Serial.printf("[%s]: Enabling 3V to power LCD on GPIO=%d\n",__func__,LCD_3V_GPIO);
      #endif
      digitalWrite(LCD_3V_GPIO, HIGH);
      while (!digitalRead(LCD_3V_GPIO)){delay(1);}
    #endif

    char temp_char[10];
    bool temp_error = false;
    
    if ((myData.temp < -33) and (myData.temp > -34))
    {
      temp_error = true;
    } else 
    {
      snprintf(temp_char,sizeof(temp_char),"%0.1f",myData.temp);
      #ifdef DEBUG
        Serial.printf("[%s]: TEMPERATURE OK %0.2f\n",__func__,myData.temp);
      #endif
    }

    tft.init(); // it takes 1s for ST7735 !!!
    tft.fillScreen(TFT_BLACK);
    delay(50);  // this delay solves initial flicker
    digitalWrite(LCD_LED_GPIO,HIGH);

    tft.setRotation(SCREEN_ROTATION);

    int t_w = img.textWidth(temp_char,7);
    int t_h = img.fontHeight(7);

    tft.setTextPadding(TFT_WIDTH);
    tft.setTextDatum(TL_DATUM);
    if (!temp_error)
    {
      tft.setTextColor(TFT_WHITE,TFT_BLACK);
      tft.drawString(temp_char, 10, 10, 7);
      tft.setTextColor(TFT_YELLOW,TFT_BLACK);
      tft.drawString("C", t_w +10, 10, 4);
    } else 
    {
      tft.setTextColor(TFT_RED,TFT_BLACK);
      tft.drawString("ERROR", 30, 30, 4);
    }

    delay(LCD_SCREEN_TIME_S * 1000);


    digitalWrite(LCD_LED_GPIO,LOW);

    // off 3V
    #ifdef LCD_3V_GPIO
      #ifdef DEBUG
        Serial.printf("[%s]: Disabling 3V on GPIO=%d\n",__func__,LCD_3V_GPIO);
      #endif
      digitalWrite(LCD_3V_GPIO, LOW);
    #endif
  }
#endif

  if (!ota_web_server_needed)
  hibernate(false, sleeptime_s);
}


// loop is empty - sleep is the only option (or restart if ESPnow is not working)
void loop() {}
