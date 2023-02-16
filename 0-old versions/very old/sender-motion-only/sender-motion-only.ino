/*
sender.ino
*/

// ******************************* DEBUG ***************************************
// #define DEBUG

// #define PPK2_GPIO 35           // comment out if not used - GPIO to test power and timings using PPK2
#define USE_FAKE_RECEIVER   0     // 1=use this to avoid flooding receiver/HA, any other proper receivers

// sending data to all ESP (broadcast) or to specific only (unicast)
#define MAC_BROADCAST       1     // 1=broadcast, any other for unicast - irrelevant if USE_FAKE_RECEIVER = 1
// to see which board is being compiled
#define PRINT_COMPILER_MESSAGES

// ********************* choose device - ONLY ONE! *****************************
// detailed config in the file devices_config.h
// #define DEVICE_ID  26           // "esp32026" - S2, hall
// #define DEVICE_ID  27           // "esp32027" - S2, balcony
// #define DEVICE_ID  33           // "esp32033" - S2, g-street
// #define DEVICE_ID  35           // "esp32035" - S2, g-garden
#define DEVICE_ID  36           // "esp32036" - S2, kitchen
// #define DEVICE_ID  86           // "esp32086" - S2, test, new board

// version < 10 chars, description in changelog.txt
#define VERSION "1.19.b3"

// devices configuration file
#include "devices_config.h"

// ****************  ALL BELOW ALL IS COMMON FOR ANY ESP32 *********************
#define WIFI_CHANNEL        8           // in my house
#define WAIT_FOR_WIFI       5           // in seconds, for upgrade firmware
#define MAX17048_DELAY_ON_RESET_MS  200 // as per datasheet: needed before next reading from MAX17048 after reset, only in use when reset/battery change
// ******************************  some consistency checks *************************

#if (!defined (HOSTNAME) or !defined(DEVICE_NAME) or !defined(BOARD_TYPE) )
  #error "HOSTNAME, DEVICE_NAME, BOARD_TYPE are obligatory"
#endif

#if (!defined (MOTION_SENSOR_GPIO))
  #error "MOTION_SENSOR_GPIO is obligatory"
#endif

#if (!defined(FW_UPGRADE_GPIO))
  #error "FW_UPGRADE_GPIO is obligatory"
#endif

// ****************  LOAD libraries and initiate variables *************************
#include <passwords.h>          // passwords inside my library - for my personal usage
// #include "passwords.h"          // passwords in current folder - for github - see the structure inside the file

// Firmware update
#include <HTTPClient.h>
#include <Update.h>
#if   (BOARD_TYPE == 1)
  #define FW_BIN_FILE "sender-motion-only.ino.esp32.bin"
#elif (BOARD_TYPE == 2)
  #define FW_BIN_FILE "sender-motion-only.ino.esp32s2.bin"
#elif (BOARD_TYPE == 3)
  #define FW_BIN_FILE "sender-motion-only.ino.esp32s3.bin"
#else
  #error "FW update defined only for ESP32, ESP32-S2 and ESP32-S3 boards"
#endif

HTTPClient firmware_update_client;
int fw_totalLength = 0;
int fw_currentLength = 0;
bool perform_update_firmware=false;
int update_progress=0;
int old_update_progress=0;
bool blink_led_status=false;
// Firmware update END

// other libraries and variables
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Wire.h>

// MAX17048 - battery fuel gauge, I2C
#if (USE_MAX17048 == 1)
  bool max17ok = true;
  #include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
  SFE_MAX1704X lipo(MAX1704X_MAX17048);
#endif
// MAX17048 END

// ESPnow:

// UNICAST:
// status = 1 when delivered with ack
// it will try to retransmit few times (10?) before giving status = 0
// REPLACE WITH YOUR RECEIVER MAC Address
// receiver might also use arbitrary MAC

// below: first and second receivers - no difference in any task but different MAC addresses

// first receiver  - gw1 - odd bootCount
uint8_t broadcastAddress1[]     = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xff};
// second receiver - gw2 - even bootCount
uint8_t broadcastAddress2[]     = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xee};

#if (USE_FAKE_RECEIVER == 1)
  // fake receiver - any MAC
  uint8_t broadcastAddress3[]     = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0x00};
#endif

// BROADCAST - any receiver
uint8_t broadcastAddress_all[]  = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
// sends to all devices - it works!
// but if broadcast used esp_now_send_status_t status is not received, however communication is faster (no ack needed)
// status = 1 when sent, without waiting for confirmation
// no retransmission for broadcast

// final receiver MAC address, changeable in the code depends on bootCount
uint8_t broadcastAddress[]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



typedef struct struct_message          // 88 bytes
{
  char host[10];        // esp32123   - 9 characters maximum (esp32123=8)
  char name[16];        // 15 characters maximum
  char ver[10];         // 123.56.89  - 9 characters maximum (123.56.89=9)
  char sender_type[10]; // "motion" "env", space for others as well in the future
  char charg[5];        // "FULL","ON","NC" - 4 characters maximum
  float temp;
  float hum;
  float lux;
  float bat;
  float batpct;
  float batchr;
  byte motion;          // 0 - no motion, 1 - motion
  unsigned int boot;
  unsigned long ontime; // seconds, probably unsigned int would be enough - check it
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;


// auxuliary variables
#define uS_TO_S_FACTOR 1000000ULL
unsigned long start_espnow_time;
unsigned long current_ontime_l = 0;
esp_sleep_wakeup_cause_t wakeup_reason;
uint64_t wakeup_gpio_mask;
byte wakeup_gpio;
bool fw_update = false;
// motion
bool motion = false;

// ****************  FUNCTIONS *************************

// declaration
void hibernate(bool force);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void send_data(bool motion_detected);
void do_esp_restart();
bool start_espnow();
void disable_espnow();
void setup_wifi();
void sos(int led);
void do_update();
void updateFirmware(uint8_t *data, size_t len);
int update_firmware_prepare();
void measure_battery_adc(float* volts);


// implementation


// update firmware
void disable_espnow()
{
  esp_now_unregister_send_cb();
  esp_now_deinit();
}


// connect to AP for FW upgrade
void setup_wifi()
{
  long ttc, sm1, wifi_start_time;
  WiFi.mode(WIFI_MODE_STA);
  wifi_start_time = millis();
  WiFi.begin(BT_SSID, BT_PASSWORD,WIFI_CHANNEL);

  uint8_t MAC_array[6];
  WiFi.macAddress(MAC_array);
  Serial.printf("[%s]: my MAC address:%02X:%02X:%02X:%02X:%02X:%02X\n",__func__,MAC_array[0],MAC_array[1],MAC_array[2],MAC_array[3],MAC_array[4],MAC_array[5]);

  Serial.printf("[%s]: Connecting to %s ...\n",__func__,BT_SSID);

  while (WiFi.status() != WL_CONNECTED)
  {
    ttc = millis() - wifi_start_time;
    sm1=millis(); while(millis() < sm1 + 20) {}
    if (ttc > (WAIT_FOR_WIFI * 1000)) {
      Serial.printf("[%s]: still NOT connected after %dms\nRESTARTING - WiFi not connected\n",__func__,ttc);
      do_esp_restart();
    }
  }
  Serial.printf("[%s]: connected after %dms\n",__func__,ttc);

  IPAddress ip = WiFi.localIP();
  char buf[17];
  sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("[%s]: my IP address: %s\n",__func__,buf);
}


// blink nicely - SOS on upgrade failure
void sos(int led)
{
  #define DIT_MS 75;
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
}


// FW upgrade wrapper
void do_update()
{
  Serial.printf("[%s]: FW UPGRADE starting...\n",__func__);
  int update_firmware_status = -1;
  update_firmware_status=update_firmware_prepare();
  if (update_firmware_status == 0)
  {
    Serial.printf("[%s]: RESTARTING - FW update SUCCESSFULL\n\n",__func__);
    // blink slowly when FW upgrade successfull
    for (int i=0;i<3;i++)
    {
      #ifdef ERROR_RED_LED_GPIO
        digitalWrite(ERROR_RED_LED_GPIO,LOW);
        delay(500);
        digitalWrite(ERROR_RED_LED_GPIO,HIGH);
        delay(100);
      #elif defined(ACT_BLUE_LED_GPIO)
        digitalWrite(ACT_BLUE_LED_GPIO,LOW);
        delay(500);
        digitalWrite(ACT_BLUE_LED_GPIO,HIGH);
        delay(100);
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
  do_esp_restart();
}


// real upgrade
void updateFirmware(uint8_t *data, size_t len)
{
  // blink ERROR_RED_LED_GPIO or...
  #ifdef ERROR_RED_LED_GPIO
    if (blink_led_status) {
      blink_led_status=LOW;
      digitalWrite(ERROR_RED_LED_GPIO,blink_led_status);
    } else {
      blink_led_status=HIGH;
      digitalWrite(ERROR_RED_LED_GPIO,blink_led_status);
    }
  #else
    // ...blink ACT_BLUE_LED_GPIO
    #ifdef ACT_BLUE_LED_GPIO
      if (blink_led_status) {
        blink_led_status=LOW;
        digitalWrite(ACT_BLUE_LED_GPIO,blink_led_status);
      } else {
        blink_led_status=HIGH;
        digitalWrite(ACT_BLUE_LED_GPIO,blink_led_status);
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
  snprintf(firmware_file,sizeof(firmware_file),"%s/01-Production/0-ESPnow/%s/%s",UPDATE_FIRMWARE_HOST,HOSTNAME,FW_BIN_FILE);

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
  Serial.printf("[%s]: firmware update prepare SUCESSFUL\n",__func__);
  return 0;
}
// update firmware END


// lux from ADC
#if (MEASURE_VOLTS_ADC == 1)
  void measure_battery_adc(float* volts)
  {
    uint32_t read_digital = 0;

    analogSetPinAttenuation(VOLTS_ADC_GPIO, ADC_11db);
    adcAttachPin(VOLTS_ADC_GPIO);
    for ( byte i = 0; i < 10; i++) {
        read_digital += analogRead(VOLTS_ADC_GPIO);
    }
    read_digital /= 10;

    #ifdef DEBUG
      Serial.printf("[%s]: read_digital=%d\n",__func__,read_digital);
    #endif

    #if (BOARD_TYPE == 1)
      *volts = ((float)(3.3 * read_digital)/4095) * ADC_CALLIBRATION_FACTOR;
    #elif ((BOARD_TYPE == 2) or (BOARD_TYPE == 2))
      *volts = ((float)(3.3 * read_digital)/8191) * ADC_CALLIBRATION_FACTOR;
    #else
      #error "This functions is configured for ESP32/S2/S3 only."
    #endif
  }
#endif


void hibernate(bool force)
{
  #ifdef DEBUG
    #ifdef MOTION_SENSOR_GPIO
      Serial.printf("[%s]: MOTION_SENSOR_GPIO=%d\n",__func__,MOTION_SENSOR_GPIO);
    #endif
    #ifdef FW_UPGRADE_GPIO
      Serial.printf("[%s]: FW_UPGRADE_GPIO=%d\n",__func__,FW_UPGRADE_GPIO);
    #endif
  #endif
  long int bitmask_dec;

  esp_wifi_stop();                            // wifi
  esp_deep_sleep_disable_rom_logging();       // it does not display welcome message - shorter time to wake up
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M,         ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,       ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

  //the next 2 lines are not working with ext0 but ok with ext1
  esp_sleep_config_gpio_isolate();
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);

  //not working for ext1 if RTC peripherials are OFF - using pulldown resistor for button, no need for PIR
  // esp_err_t rtc_gpio_pulldown_en(GPIO_NUM_1);
  // esp_err_t rtc_gpio_pulldown_en(GPIO_NUM_9);

  if (force)
  {
    esp_sleep_enable_timer_wakeup(3600 * uS_TO_S_FACTOR);
    Serial.printf("[%s]: going to sleep for 1 hour to save battery\n",__func__);
    esp_deep_sleep_start();
  }

  // https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/ for ext0 and ext1 examples
  if (motion)
  {
    //send ESP to deep unconditional sleep for predefined time -  wake up on timer (cooling period)
    esp_sleep_enable_timer_wakeup(COOLING_SLEEP_DURATION_S * uS_TO_S_FACTOR);
    //... or on GPIO ext1 (FW update)
    #ifdef FW_UPGRADE_GPIO //if FW_UPGRADE_GPIO  defined, wake up on it
      bitmask_dec = pow(2,FW_UPGRADE_GPIO);
      #ifdef DEBUG
        Serial.printf("[%s]: bitmask_dec (FW_UPGRADE_GPIO) in dec=%d\n",__func__,bitmask_dec);
      #endif
      esp_sleep_enable_ext1_wakeup(bitmask_dec, ESP_EXT1_WAKEUP_ANY_HIGH);
    #endif

    Serial.printf("[%s]: going to sleep for %ds cool time\n",__func__,COOLING_SLEEP_DURATION_S);
  } else
  {
    //send ESP to deep unconditional sleep for predefined time -  wake up on timer...(heartbeat)
    esp_sleep_enable_timer_wakeup(HEARTBEAT_SLEEP_DURATION_S * uS_TO_S_FACTOR);
    //... or on GPIO ext1 (PIR motion detected) or FW_UPGRADE_GPIO
    #ifdef FW_UPGRADE_GPIO //if FW_UPGRADE_GPIO  defined, wake up on it or on PIR - actually FW_UPGRADE_GPIO is obligatory
      bitmask_dec = pow(2,FW_UPGRADE_GPIO) + pow(2,MOTION_SENSOR_GPIO);
      #ifdef DEBUG
        Serial.printf("[%s]: bitmask_dec (FW_UPGRADE_GPIO + MOTION_SENSOR_GPIO) in dec=%d\n",__func__,bitmask_dec);
      #endif
    #else //if FW_UPGRADE_GPIO not defined, wake up only on PIR
      bitmask_dec = pow(2,MOTION_SENSOR_GPIO);
      #ifdef DEBUG
        Serial.printf("[%s]: bitmask_dec=2^GPIO in dec=%d\n",__func__,bitmask_dec);
      #endif
    #endif
    esp_sleep_enable_ext1_wakeup(bitmask_dec, ESP_EXT1_WAKEUP_ANY_HIGH);

    Serial.printf("[%s]: going to sleep until next motion detected\n",__func__);
    Serial.printf("[%s]:  or for %ds heartbeat time\n",__func__,HEARTBEAT_SLEEP_DURATION_S);
  }

  current_ontime_l = millis() + ESP32_IS_CHEATING;
  Serial.printf("[%s]: Program finished after %lums.\n",__func__,current_ontime_l);
  esp_deep_sleep_start();
}


void do_esp_restart()
{
  ESP.restart();
}


// on sent callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  {
    if (status == ESP_NOW_SEND_SUCCESS)
    {
      #ifdef DEBUG
        Serial.printf("[%s]: ESPnow SUCCESSFULL\n",__func__);
      #endif
    } else
    {
      // it never returns FAILED in case of MAC_BROADCAST used
      #ifdef ERROR_RED_LED_GPIO
        digitalWrite(ERROR_RED_LED_GPIO,HIGH);
      #endif
      Serial.printf("[%s]: ESPnow FAILED\n",__func__);
    }
    #ifdef DEBUG
      unsigned long tt=millis() - start_espnow_time;
      Serial.printf("[%s]: ESPnow took:....%dms\n",__func__,tt);
    #endif
  }
}


// send data to gateway over ESPnow
void send_data(bool motion_detected)
{
  if (motion_detected)
    myData.motion=1;
  else
    myData.motion=0;
  #ifdef DEBUG
    Serial.printf("[%s]: Sending data:\n",__func__);
    Serial.printf("\tmyData.host=%s\n",myData.host);
    Serial.printf("\tmyData.name=%s\n",myData.name);
    Serial.printf("\tmyData.ver=%s\n",myData.ver);
    Serial.printf("\tmyData.sender_type=%s\n",myData.sender_type);
    Serial.printf("\tmyData.motion=%d\n",motion_detected);
    Serial.printf("\tmyData.bat=%fV\n",myData.bat);
    Serial.printf("\tmyData.batpct=%f%%\n",myData.batpct);
    Serial.printf("\tmyData.batchr=%f%%\n",myData.batchr);

    start_espnow_time = millis();
  #endif
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  // the next code seems never to work as it goes to sleep after OnDataSent()
  if (result != ESP_OK)
  {
    #ifdef DEBUG
      Serial.printf("[%s]: Error sending the data\n",__func__);
    #endif
  } else
  {
    #ifdef DEBUG
      Serial.printf("[%s]: data sent over ESPnow\n",__func__);
    #endif
  }
}


// start ESPnow
bool start_espnow()
{
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR);

  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  // #ifdef DEBUG
    // WiFi.printDiag(Serial); // - it shows even password ;-)
  // #endif
  if (esp_now_init() != ESP_OK) {
    #ifdef DEBUG
      Serial.printf("[%s]: ESP NOW failed to initialize\n",__func__);
    #endif
  }
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.ifidx   = WIFI_IF_STA;
  peerInfo.encrypt = false;
  esp_now_register_send_cb(OnDataSent);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    #ifdef DEBUG
      Serial.printf("[%s]: ESP NOW pairing failure\n",__func__);
    #endif
    return false;
  } else
  {
    return true;
  }
}

// setup
void setup()
{
  // testing with PPK2
  #ifdef PPK2_GPIO
    pinMode(PPK2_GPIO,OUTPUT);
    digitalWrite(PPK2_GPIO,HIGH);
  #endif

// power for sensors from GPIO - MUST be before any I2C sensor is in use obviously!
  #ifdef ENABLE_3V_GPIO
    #ifdef DEBUG
      Serial.printf("[%s]: enabling ENABLE_3V_GPIO\n",__func__);
    #endif
    pinMode(ENABLE_3V_GPIO, OUTPUT);
    digitalWrite(ENABLE_3V_GPIO, HIGH);
    delay(10);
  #endif

// start Wire after enabling 3.3V
  Wire.begin(); // it takes 0.6ms

  Serial.begin(115200);
  Serial.printf("\n======= S T A R T =======\n");
  Serial.printf("[%s]: Device: %s (%s, version=%s)\n",__func__,DEVICE_NAME,HOSTNAME,VERSION);

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0:
    {
      Serial.printf("[%s]: Wakeup cause: external signal using RTC_IO (motion detected)\n",__func__);
      motion = true;
      #ifdef ERROR_RED_LED_GPIO
        pinMode(ERROR_RED_LED_GPIO, OUTPUT);
        digitalWrite(ERROR_RED_LED_GPIO, HIGH);
      #endif
      break;
    }
    case ESP_SLEEP_WAKEUP_EXT1:
    {
      wakeup_gpio_mask = esp_sleep_get_ext1_wakeup_status();
      wakeup_gpio = log(wakeup_gpio_mask)/log(2);

      Serial.printf("[%s]: Wakeup cause: external signal using RTC_CNTL=%d\n",__func__,wakeup_gpio);
      #ifdef ERROR_RED_LED_GPIO
        pinMode(ERROR_RED_LED_GPIO, OUTPUT);
        digitalWrite(ERROR_RED_LED_GPIO, HIGH);
      #endif
      if (wakeup_gpio == FW_UPGRADE_GPIO)
      {
        fw_update = true;
      }
      if (wakeup_gpio == MOTION_SENSOR_GPIO)
      {
        motion = true;
      }
      break;
    }
    case ESP_SLEEP_WAKEUP_TIMER:
    {
      Serial.printf("[%s]: Wakeup cause: WAKEUP_TIMER (heartbeat)\n",__func__);
      #ifdef ACT_BLUE_LED_GPIO
        pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
        digitalWrite(ACT_BLUE_LED_GPIO, HIGH);
      #endif
      break;
    }
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
    {
      Serial.printf("[%s]: Wakeup cause: WAKEUP_TOUCHPAD\n",__func__);
      break;
    }
    case ESP_SLEEP_WAKEUP_ULP:
    {
      Serial.printf("[%s]: Wakeup cause: WAKEUP_ULP\n",__func__);
      break;
    }
    default:
    {
      Serial.printf("[%s]: Wakeup was not caused by deep sleep: %d\n",__func__,wakeup_reason);
      break;
    }
  }

    if (motion)
    {
      Serial.printf("[%s]: motion DETECTED\n",__func__);
    } else
    {
      Serial.printf("[%s]: motion CLEARED\n",__func__);
    }

  /*
  myData.temp = 0.0f;
  myData.hum = 0.0f;
  myData.lux = 0.0f;
  myData.bat = 0.0f;
  myData.batpct = 0.0f;
  myData.batchr = 0.0f;
  snprintf(myData.charg,4,"%s","N/A");
  myData.boot = 1;
  myData.ontime=1;
  */

// battery
  myData.bat    = 3.33f; //fake battery voltage in case no battery measurement in config (ADC or MAX17048)
  myData.batpct = 0.0f;
  myData.batchr = 0.0f;

  // ADC voltage
  #if (MEASURE_VOLTS_ADC == 1)
    float volts;
    measure_battery_adc(&volts);
    myData.bat = volts;
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
      if (wakeup_reason == 0)
      {
        // reset MAX17048 on any abnormal restart including changing battery, not including normal work (sleep, motion)
        #ifdef DEBUG
          Serial.printf("[%s]: Resetting MAX17048, delay for %dms\n",__func__,MAX17048_DELAY_ON_RESET_MS);
        #endif
        lipo.reset();
        delay(MAX17048_DELAY_ON_RESET_MS);
      }
    }
    if (max17ok)
    {
      myData.bat = lipo.getVoltage();
      myData.batpct = lipo.getSOC();
      myData.batchr = lipo.getChangeRate();
    }

  #ifdef DEBUG
    Serial.printf("\tbat=%fV\n",myData.bat);
    Serial.printf("\tbatpct=%f%%\n",myData.batpct);
    Serial.printf("\tbatchr=%f%%\n",myData.batchr);
  #endif
  #else
    #ifdef DEBUG
      Serial.printf("[%s]: DONT USE_MAX17048\n",__func__);
    #endif
  #endif
// battery END

  // EMERGENCY - LOW BATTERY - don't start WiFi or ESPnow - hibernate ASAP for 1h
  if (myData.bat < MINIMUM_VOLTS)
  {
    Serial.printf("[%s]: battery volts=%0.2fV, that is below minimum [%0.2fV]\n",__func__,myData.bat,MINIMUM_VOLTS);
    Serial.printf("[%s]: that is below MINIMUM\n",__func__);
    hibernate(true);
  }

  snprintf(myData.host,sizeof(myData.host),"%s",HOSTNAME);
  if (fw_update)
  {
    snprintf(myData.ver,sizeof(myData.ver),"%s","FW UPG...");
  } else
  {
    snprintf(myData.ver,sizeof(myData.ver),"%s",VERSION);
  }
  snprintf(myData.name,sizeof(myData.name),"%s",DEVICE_NAME);
  snprintf(myData.sender_type,sizeof(myData.sender_type),"%s",SENSOR_TYPE);

  // sender chooses the receiver based on bootCount (not for motion sensors - only env sensors) or broadcast
  char receiver_mac[18];
  #if (USE_FAKE_RECEIVER == 1)
    #pragma message "NOT sending data to any receiver - FAKE MAC ADDRESS USED"
    memcpy(broadcastAddress, broadcastAddress3, sizeof(broadcastAddress));
    snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);
  #else
    #if (MAC_BROADCAST == 1)
      #pragma message "sending data to all ESP - BROADCAST"
      memcpy(broadcastAddress, broadcastAddress_all, sizeof(broadcastAddress));
      #ifdef DEBUG
        snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);
      #endif
    #endif
  #endif

  if (!start_espnow())
  {
    #ifdef ERROR_RED_LED_GPIO
      pinMode(ERROR_RED_LED_GPIO, OUTPUT);
      digitalWrite(ERROR_RED_LED_GPIO, HIGH);
    #endif
    Serial.printf("[%s]: espnow NOT STARTED\n",__func__);
    do_esp_restart();
  } else
  {
    #ifdef DEBUG
    Serial.printf("[%s]: espnow STARTED\n",__func__);
    #endif
  }

  // all data gathered - time to send it
  send_data(motion);

  if (fw_update)
  {
    Serial.printf("[%s]: FW update requested\n",__func__);
    disable_espnow();
    setup_wifi();
    do_update();
  }

  // eventually: go to sleep ;-)
  hibernate(false);
}


// loop is empty - sleep is the only option (or restart if ESPnow is not working)
void loop(){}
