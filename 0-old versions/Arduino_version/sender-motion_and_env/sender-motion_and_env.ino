/*
sender.ino
*/

// ******************************* DEBUG ***************************************
#define DEBUG

// **** format FS on first deployment only, then change to 0 or comment out ****
// you can also format it later on long press (6s) FW_UPGRADE_GPIO
#define FORMAT_FS   0

// #define PPK2_GPIO 35           // comment out if not used - GPIO to test power and timings using PPK2
#define USE_FAKE_RECEIVER   0     // 1=use this to avoid flooding receiver/HA, any other proper receivers

// sending data to all ESP (broadcast) or to specific only (unicast)
#define MAC_BROADCAST       1     // 1=broadcast, any other for unicast - irrelevant if USE_FAKE_RECEIVER = 1
// to see which board is being compiled
#define PRINT_COMPILER_MESSAGES

// ********************* choose device - ONLY ONE! *****************************
// detailed config in the file devices_config.h


// new, multipurpose device
  // #define DEVICE_ID  26           // "esp32026" - C3, production - TRV4 Printers
  // #define DEVICE_ID  28           // "esp32028" - S2, production - Garage
  // #define DEVICE_ID  31           // "esp32031" - S2, production - Printer room
  // #define DEVICE_ID  32           // "esp32032" - S2, production - Bedroom mot.
  // #define DEVICE_ID  33           // "esp32033" - S2, production - Living room
  // #define DEVICE_ID  35           // "esp32035" - S2, production - Tailor room
  // #define DEVICE_ID  36           // "esp32036" - S2, production - Lidia room
  // #define DEVICE_ID  38           // "esp32038" - S2, production - Toilet bedroom
  // #define DEVICE_ID  47             // "esp32047" - C3, production - UPS battery 1
  // #define DEVICE_ID  48             // "esp32048" - C3, production - UPS battery 2
  // #define DEVICE_ID  100          // "esp32100" - S2, production - Office
  // #define DEVICE_ID  101          // "esp32101" - S2, production - Dining room
  // #define DEVICE_ID  102          // "esp32102" - S2, production - Toilet up
  // #define DEVICE_ID  104          // "esp32104" - S2, production - Kids room
  // #define DEVICE_ID  105          // "esp32105" - S2, production - Garden
// new, multipurpose device END

// test devices
   // #define DEVICE_ID  90           //test, C3

   #define DEVICE_ID  92           //test, S2
   // #define DEVICE_ID  93           //test, C3
   // #define DEVICE_ID  94           //test, S
   // #define DEVICE_ID  95           //test, C3
// test devices END
// ********************* choose device - ONLY ONE! *****************************

// version < 10 chars, description in changelog.txt
#define ZH_PROG_VERSION "1.23.0"

// devices configuration file
#include "devices_config.h"

// ****************  ALL BELOW ALL IS COMMON FOR ANY ESP32 *********************
#define WIFI_CHANNEL                8           // in my house
#define WAIT_FOR_WIFI               5           // in seconds, for upgrade firmware
#define MAX17048_DELAY_ON_RESET_MS  200 // as per datasheet: needed before next reading from MAX17048 after reset, only in use when reset/battery change
// ******************************  some consistency checks *************************

#if (!defined (HOSTNAME) or !defined(DEVICE_NAME) or !defined(BOARD_TYPE) )
  #error "HOSTNAME, DEVICE_NAME, BOARD_TYPE are obligatory"
#endif

// ****************  LOAD libraries and initiate variables *************************
#include <passwords.h>          // passwords inside my library - for my personal usage
// #include "passwords.h"          // passwords in current folder - for github - see the structure inside the file

// Firmware update
#include <HTTPClient.h>
#include <Update.h>
#if   (BOARD_TYPE == 1)
  #define FW_BIN_FILE "sender-motion_and_env.ino.esp32.bin"
#elif (BOARD_TYPE == 2)
  #define FW_BIN_FILE "sender-motion_and_env.ino.esp32s2.bin"
#elif (BOARD_TYPE == 3)
  #define FW_BIN_FILE "sender-motion_and_env.ino.esp32s3.bin"
#elif (BOARD_TYPE == 4)
  #define FW_BIN_FILE "sender-motion_and_env.ino.esp32c3.bin"
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

// filesystem
#define FORMAT_LITTLEFS_IF_FAILED true
#include "FS.h"
#include <LittleFS.h>

// MAX17048 - battery fuel gauge, I2C
#if (USE_MAX17048 == 1)
  #include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
  SFE_MAX1704X lipo(MAX1704X_MAX17048);
#endif

// sht31 - temperature and humidity, I2C
#if (USE_SHT31 == 1)
  #include "Adafruit_SHT31.h"
  Adafruit_SHT31 sht31 = Adafruit_SHT31();
#endif

// lux from TSL2561 - light sensor, I2C
#if (USE_TSL2561 == 1)
  #include <SparkFunTSL2561.h>
  SFE_TSL2561 light;
#endif

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

// data variables
typedef struct struct_message          // 92 bytes
{
  char host[10];        // esp32123   - 9 characters maximum (esp32123=8)
  char name[16];        // 15 characters maximum
  char ver[10];         // 123.56.89  - 9 characters maximum (123.56.89=9)
  char sender_type[10]; // "env", "motion","env+mot"
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
  byte boardtype;
} struct_message;

struct_message myData;

// this array can expand but the max size of string is 9 characters - this text goes to myData.sender_type over ESPnow
const char sender_type_char[5][10] = {"env", "motion", "env+mot","battery","undef2"};

// ESPnow variable
esp_now_peer_info_t peerInfo;

// auxuliary variables:
// sleep:
#define uS_TO_S_FACTOR 1000000ULL

// to calculate time:
unsigned long start_espnow_time = 0;
unsigned long current_ontime_l = 0;
unsigned long saved_ontime_l = 0;
long program_start_time,em,tt;

// check reasons to wake up and start
esp_sleep_wakeup_cause_t wakeup_reason;
byte boot_reason; // esp_reset_reason_t ???
uint64_t wakeup_gpio_mask;
byte wakeup_gpio;

bool fw_update = false;
bool motion = false;
unsigned bootCount = 1;  //we don't start from 0 but from 1

bool sht31ok = true;
bool tslok   = true;
bool max17ok = true;

// files to store some data
#define BOOT_COUNT_FILE     "/bootcount.dat"
#define ONTIME_FILE         "/ontime.dat"

// charging constants
#define CHARGING_NC   "NC"
#define CHARGING_ON   "ON"
#define CHARGING_FULL "FULL"
#define CHARGING_OFF  "OFF"
// global as assigned in setup() - to avoid calling again
char charging[5];

// ****************  FUNCTIONS *************************

// declaration
void update_bootCount();
bool gather_data();

void lux_with_tept(long int* lux_int);
void charging_state();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
bool readFile(fs::FS &fs, const char * path, char * data);
bool writeFile(fs::FS &fs, const char * path, const char * message);
void load_ontime();
void save_ontime();

void hibernate(bool force);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
// void send_data(bool motion_detected);
void send_data();

void do_esp_restart();
bool start_espnow();
void disable_espnow();
void setup_wifi();
void sos(int led);
void do_update();
void updateFirmware(uint8_t *data, size_t len);
int update_firmware_prepare();

// implementation
// check charging
void charging_state()
{
  #if defined(CHARGING_GPIO) and defined(POWER_GPIO)
  /*
  POWER_GPIO      pin 6, green LED on charger TP4056, HIGH on charging, LOW on full
  CHARGING_GPIO   pin 7, red   LED on charger TP4056, HIGH on full, LOW on charging
  existing LEDs on TP4506 must be still connected to pins of TP4506 otherwise pins are floating, or rewire them to new LEDs on the box
  */
    if ((digitalRead(POWER_GPIO) == 0) and (digitalRead(CHARGING_GPIO) == 0)) {snprintf(charging,sizeof(CHARGING_NC), "%s", CHARGING_NC);}
    if ((digitalRead(POWER_GPIO) == 0) and (digitalRead(CHARGING_GPIO) == 1)) {snprintf(charging,sizeof(CHARGING_FULL), "%s", CHARGING_FULL);}
    if ((digitalRead(POWER_GPIO) == 1) and (digitalRead(CHARGING_GPIO) == 0)) {snprintf(charging,sizeof(CHARGING_ON), "%s", CHARGING_ON);}
    if ((digitalRead(POWER_GPIO) == 1) and (digitalRead(CHARGING_GPIO) == 1)) {snprintf(charging,sizeof(CHARGING_OFF), "%s", CHARGING_OFF);}
    #ifdef DEBUG
      Serial.printf("[%s]: charging=%s\n",__func__,charging);
    #endif
  #endif
}

// save bootCount
void update_bootCount()
{
  char data[12];
  if(!LittleFS.begin(true))
  {
    Serial.printf("[%s]: LittleFS Mount Failed\n",__func__);
  }
  else
  {
    // format FS on first deployment
    #if (FORMAT_FS == 1)
      Serial.printf("[%s]: formatting FS...\n",__func__);
      if (LittleFS.format())
      {
        Serial.printf("[%s]: SUCCESSFULL\n",__func__);
      } else
      {
        #ifdef ERROR_RED_LED_GPIO
          digitalWrite(ERROR_RED_LED_GPIO,HIGH);
        #endif
        Serial.printf("[%s]: FAILED\n",__func__);
      }
    #endif
    // list files in DEBUG mode only
    #ifdef DEBUG
      listDir(LittleFS,"/",1);
    #endif
    // read bootCount from file
    if (readFile(LittleFS, BOOT_COUNT_FILE, data))
    {
      bootCount = atoi(data) + 1;
      #ifdef DEBUG
        Serial.printf("[%s]: bootCount=%d\n",__func__,bootCount);
      #endif
    }
    // reset bootCount on button reset or software restart
    if ((boot_reason == 1) or (boot_reason == 3)) bootCount = 1;

    // convert int to char array
    int nbytes = snprintf(NULL,0,"%d",bootCount) + 1;
    snprintf(data,nbytes,"%d",bootCount);

    // write to file
    if (writeFile(LittleFS, BOOT_COUNT_FILE, data))
    {
      #ifdef DEBUG
        Serial.printf("[%s]: bootCount saved\n",__func__);
      #endif
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: bootCount NOT saved\n",__func__);
      #endif
    }
  }
}

// list directory
void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
  Serial.printf("[%s]: Listing directory: %s\r\n",__func__,dirname);
  File root = fs.open(dirname);
  if(!root)
  {
    Serial.printf("[%s]: - failed to open directory\n",__func__);
    return;
  }
  if(!root.isDirectory())
  {
    Serial.printf("[%s]: - not a directory\n",__func__);
    return;
  }
  File file = root.openNextFile();
  while(file)
  {
    if(file.isDirectory())
    {
      Serial.printf("[%s]:  DIR : %s\n",__func__,file.name());
      if(levels)
      {
          listDir(fs, file.name(), levels -1);
      }
    } else
    {
      Serial.printf("[%s]:  FILE: %s\tSIZE: %d\n",__func__,file.name(),file.size());
    }
    file = root.openNextFile();
  }
}

// read file
bool readFile(fs::FS &fs, const char * path, char * data)
{
  File file = fs.open(path);
  if (!file)
  {
    #ifdef DEBUG
      Serial.printf("[%s]: Failed to open file: %s\n",__func__,path);
    #endif
    return false;
  }
  for(int j = 0; j < file.size(); j++)
  {
    data[j] = (char)file.read();
  }
  return true;
}

// write file
bool writeFile(fs::FS &fs, const char * path, const char * message)
{
  #ifdef DEBUG
    Serial.printf("[%s]: Writing file: %s\n",__func__,path);
  #endif
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    #ifdef DEBUG
      Serial.printf("[%s]: failed to open file for writing\n",__func__);
    #endif
    return false;
  }
  if(file.print(message)){
    #ifdef DEBUG
      Serial.printf("[%s]: file written\n",__func__);
    #endif
  } else {
    #ifdef DEBUG
      Serial.printf("[%s]: write failed\n",__func__);
    #endif
  }
  file.close();
  return true;
}

void load_ontime()
// time saved in milliseconds
{
  char saved_ontime_ch[12];
  if (readFile(LittleFS, ONTIME_FILE, saved_ontime_ch))
  {
    saved_ontime_l = atol(saved_ontime_ch);
    #ifdef DEBUG
      Serial.printf("[%s]: loaded ontime from file=%lums\n",__func__,saved_ontime_l);
      #endif
  } else
  {
    #ifdef DEBUG
      Serial.printf("[%s]: CANNOT READ SAVED ONTIME!\n",__func__);
    #endif
  }
}

void save_ontime()
// time saved in milliseconds
{
  char total_ontime_ch[12];

  // add new running time to saved_ontime_l
  current_ontime_l = millis() + ESP32_IS_CHEATING;
  saved_ontime_l = saved_ontime_l + current_ontime_l;

  // reset saved_ontime_l if device is charging
  #if defined(CHARGING_GPIO) and defined(POWER_GPIO)
    #ifdef DEBUG
      Serial.printf("charging=%s\n",charging);
    #endif
    // strcmp = 0 when strings ARE equal
    if ( (strcmp(charging, CHARGING_ON) == 0) or (strcmp(charging, CHARGING_FULL) == 0) )
    {
      saved_ontime_l = 0;
      #ifdef DEBUG
        Serial.printf("[%s]: Device is CHARGING, resetting saved_ontime_l=%lums\n",__func__,saved_ontime_l);
      #endif
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: Device is NOT CHARGING, new ontime (cummulative)=%lums\n",__func__,saved_ontime_l);
      #endif
    }
  #endif

  int nbytes = snprintf(NULL,0,"%lu",saved_ontime_l) + 1;
  snprintf(total_ontime_ch,nbytes,"%lu",saved_ontime_l);
  // write to file
  if (writeFile(LittleFS, ONTIME_FILE, total_ontime_ch))
  {
    #ifdef DEBUG
      Serial.printf("[%s]: total_ontime saved to file=%sms\n",__func__,total_ontime_ch);
    #endif
  } else
  {
    #ifdef DEBUG
      Serial.printf("[%s]: total_ontime NOT saved to file\n",__func__);
    #endif
  }

  // testing with PPK2
  #ifdef PPK2_GPIO
    digitalWrite(PPK2_GPIO,LOW);
  #endif

  Serial.printf("[%s]: Program finished after %lums.\n",__func__,current_ontime_l);
}

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
  // all protocols, including long range
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR);

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
      Serial.printf("[%s]: still NOT connected after %dms\nRESTARTING as WiFi is NOT connected\n",__func__,ttc);
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
        delay(100);
        digitalWrite(ACT_BLUE_LED_GPIO,HIGH);
        delay(30);
        digitalWrite(ACT_BLUE_LED_GPIO,LOW);
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
  Serial.printf("[%s]: firmware update prepare SUCESSFULL\n",__func__);
  return 0;
}
// update firmware END

void hibernate(bool force)
{
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

  if (force) // used when battery too low - sleep for 1h to avoid battery depletion
  {
    esp_sleep_enable_timer_wakeup(3600 * uS_TO_S_FACTOR);
    Serial.printf("[%s]: going to sleep for 1 hour to save battery\n",__func__);
    save_ontime();
    Serial.printf("[%s]: Bye...\n========= E N D =========\n",__func__);
    esp_deep_sleep_start();
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
      esp_sleep_enable_timer_wakeup(SLEEP_TIME_S * uS_TO_S_FACTOR);
      //... or on GPIO ext1 (PIR motion detected) or FW_UPGRADE_GPIO
      #ifdef FW_UPGRADE_GPIO //if FW_UPGRADE_GPIO  defined, wake up on it or on PIR - actually FW_UPGRADE_GPIO is obligatory
        bitmask_dec = pow(2,FW_UPGRADE_GPIO) + pow(2,MOTION_SENSOR_GPIO);
        #ifdef DEBUG
          Serial.printf("[%s]: bitmask_dec (FW_UPGRADE_GPIO + MOTION_SENSOR_GPIO) in dec=%ju\n",__func__,bitmask_dec);
        #endif
      #else //if FW_UPGRADE_GPIO not defined, wake up only on PIR
        bitmask_dec = pow(2,MOTION_SENSOR_GPIO);
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
      Serial.printf("[%s]:  or for %ds (heartbeat)\n",__func__,SLEEP_TIME_S);
    }
  #else
    //send ESP to deep unconditional sleep for predefined time -  wake up on timer...(heartbeat)
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_S * uS_TO_S_FACTOR);
    //... or on GPIO ext1 FW_UPGRADE_GPIO
    #ifdef FW_UPGRADE_GPIO //if FW_UPGRADE_GPIO  defined, wake up on it or on PIR - actually FW_UPGRADE_GPIO is obligatory
      bitmask_dec = pow(2,FW_UPGRADE_GPIO);
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
    Serial.printf("[%s]: going to sleep for %ds (heartbeat)\n",__func__,SLEEP_TIME_S);
  #endif

  save_ontime();
  Serial.printf("[%s]: Bye...\n========= E N D =========\n",__func__);
  esp_deep_sleep_start();
}

void do_esp_restart()
{
  save_ontime();
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
      // it never returns FAILED in case of MAC_BROADCAST used - work on it!
      #ifdef ERROR_RED_LED_GPIO
        digitalWrite(ERROR_RED_LED_GPIO,HIGH);
      #endif
      Serial.printf("[%s]: ESPnow FAILED\n",__func__);
    }
    tt=millis() - start_espnow_time;
    #ifdef DEBUG
      Serial.printf("[%s]: ESPnow took:....%dms\n",__func__,tt);
    #endif
  }
}

// gather data from sensors, returns false if (bat_volts < MINIMUM_VOLTS) so ESPnow will not start - goes to sleep instead of sending
bool gather_data()
{
  #ifdef DEBUG
    Serial.printf("[%s]:\n",__func__);
  #endif
  // sender_type
  snprintf(myData.sender_type,sizeof(myData.sender_type),"%s",sender_type_char[SENSOR_TYPE]);

  #ifdef DEBUG
    Serial.printf(" Data gatehered:\n\tsender_type=%s\n",myData.sender_type);
    // Serial.printf("\tmotion=%d\n",myData.motion);
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
      myData.temp = sht31.readTemperature();
      myData.hum = sht31.readHumidity();
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
        hibernate(true);
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
    snprintf(myData.ver,sizeof(myData.ver),"%s","FW UPG...");
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

  // bootCount
  myData.boot = bootCount;
  #ifdef DEBUG
    Serial.printf("\tboot=%d\n",myData.boot);
  #endif

  // ontime in seconds rather than ms
  myData.ontime = saved_ontime_l/1000;
  #ifdef DEBUG
    Serial.printf("\tontime=%lus\n",myData.ontime);
  #endif

  // boardtype
  myData.boardtype = BOARD_TYPE;
  #ifdef DEBUG
    Serial.printf("\tboardtype=%d\n",myData.boardtype);
  #endif

  #ifdef DEBUG
    Serial.printf(" Total struct size=%d bytes\n",sizeof(myData));
  #endif

  return true;
}

// send data to gateway over ESPnow
void send_data()
{
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  // the next code seems never to work as it goes to sleep after OnDataSent()
  if (result != ESP_OK) {
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
  start_espnow_time = millis();
  WiFi.mode(WIFI_MODE_STA);
  // all protocols, including long range
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR);
  // only long range - not much of difference with regards to range
  // esp_wifi_set_protocol( WIFI_IF_STA, WIFI_PROTOCOL_LR );

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
  program_start_time = millis();
  // testing with PPK2
  #ifdef PPK2_GPIO
    pinMode(PPK2_GPIO,OUTPUT);
    digitalWrite(PPK2_GPIO,HIGH);
  #endif

  Serial.begin(115200);
  Serial.printf("\n======= S T A R T =======\n");
  Serial.printf("[%s]: Device: %s (%s, version=%s, sensor type=%s, MCU type=%s)\n",__func__,DEVICE_NAME,HOSTNAME,ZH_PROG_VERSION, sender_type_char[SENSOR_TYPE],MODEL);

  boot_reason = esp_reset_reason();
  wakeup_reason = esp_sleep_get_wakeup_cause();

  // #ifdef DEBUG
  Serial.printf("[%s]: Boot cause=%d - ",__func__,boot_reason);
  switch(boot_reason)
  {
    // 1 = reset/power on
    case ESP_RST_POWERON:
    {
      Serial.printf("power on or reset\n");
      #ifdef ACT_BLUE_LED_GPIO
        pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
        digitalWrite(ACT_BLUE_LED_GPIO, HIGH);
      #endif
      #ifdef ERROR_RED_LED_GPIO
        pinMode(ERROR_RED_LED_GPIO, OUTPUT);
        digitalWrite(ERROR_RED_LED_GPIO, HIGH);
      #endif
      break;
    }
    // 3 = Software reset via esp_restart
    case ESP_RST_SW:
    {
      Serial.printf("Software reset via esp_restart\n");
      #ifdef ACT_BLUE_LED_GPIO
        pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
        digitalWrite(ACT_BLUE_LED_GPIO, HIGH);
      #endif
      #ifdef ERROR_RED_LED_GPIO
        pinMode(ERROR_RED_LED_GPIO, OUTPUT);
        digitalWrite(ERROR_RED_LED_GPIO, HIGH);
      #endif
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
  // #endif

  // #ifdef DEBUG
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
        #ifdef ERROR_RED_LED_GPIO
          pinMode(ERROR_RED_LED_GPIO, OUTPUT);
          digitalWrite(ERROR_RED_LED_GPIO, HIGH);
        #endif
        #ifdef FW_UPGRADE_GPIO
          if (wakeup_gpio == FW_UPGRADE_GPIO)
          {
            fw_update = true;
            Serial.printf("[%s]: woke up on FW button pressed\n",__func__);
            Serial.printf("[%s]: debouncing for 300ms\n",__func__);
            delay(300);
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
      #ifdef ACT_BLUE_LED_GPIO
        pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
        digitalWrite(ACT_BLUE_LED_GPIO, HIGH);
      #endif
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
        #ifdef ERROR_RED_LED_GPIO
          pinMode(ERROR_RED_LED_GPIO, OUTPUT);
          digitalWrite(ERROR_RED_LED_GPIO, HIGH);
        #endif
        #ifdef FW_UPGRADE_GPIO
          if (wakeup_gpio == FW_UPGRADE_GPIO)
          {
            fw_update = true;
            Serial.printf("[%s]: woke up on FW button pressed\n",__func__);
            Serial.printf("[%s]: debouncing for 300ms\n",__func__);
            delay(300);
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
  // #endif

  // update bootCount
  update_bootCount();

  // load saved ontime
  load_ontime();

  // resest on long, 3s press of FW_UPGRADE_GPIO button
  #ifdef FW_UPGRADE_GPIO
    // RED LED ON, BLUE OFF if FW_UPGRADE_GPIO was pressed
    if (fw_update)
    {
      #ifdef ACT_BLUE_LED_GPIO
        pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
        digitalWrite(ACT_BLUE_LED_GPIO, LOW);
      #endif
      #ifdef ERROR_RED_LED_GPIO
        pinMode(ERROR_RED_LED_GPIO, OUTPUT);
        // digitalWrite(ERROR_RED_LED_GPIO, HIGH);
      #endif
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

        #ifdef ERROR_RED_LED_GPIO
          pinMode(ERROR_RED_LED_GPIO, OUTPUT);
          digitalWrite(ERROR_RED_LED_GPIO, LOW);
        #endif
        delay(50);
        #ifdef ACT_BLUE_LED_GPIO
          pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
          digitalWrite(ACT_BLUE_LED_GPIO, HIGH);
        #endif
        Serial.printf("[%s]: Cancelling FW upgrade\n",__func__);
        fw_update = false;
        Serial.printf("[%s]: Next delay for 3s\n",__func__);
        // delay(50);
        delay(3000);
        if (digitalRead(FW_UPGRADE_GPIO))
        {
          // second 3s done so 6s in total
          #ifdef ERROR_RED_LED_GPIO
            pinMode(ERROR_RED_LED_GPIO, OUTPUT);
            digitalWrite(ERROR_RED_LED_GPIO, HIGH);
          #endif
          delay(50);
          #ifdef ACT_BLUE_LED_GPIO
            pinMode(ACT_BLUE_LED_GPIO, OUTPUT);
            digitalWrite(ACT_BLUE_LED_GPIO, LOW);
          #endif
          Serial.printf("[%s]: Resetting ALL stored info on ESP\n",__func__);
          LittleFS.format();
          Serial.printf("[%s]: RESTARTING ESP\n\n\n",__func__);
          ESP.restart();
        } else
        {
          // only first 3s done
          Serial.printf("[%s]: Resetting ESP on FW_UPGRADE_GPIO long press\n",__func__);
          do_esp_restart();
        }
      } else
      // FW_UPGRADE_GPIO LOW after 3s - FW upgrade here
      {
        Serial.printf("[%s]: FW_UPGRADE_GPIO LOW after 3s - continuing normal boot and applying FW upgrade\n",__func__);
      }
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: FW_UPGRADE_GPIO LOW after boot\n",__func__);
      #endif
    }
  #endif

  // it seems very RANDOM here - cancelling it
  // if after wake up from deep sleep motion is active, consider it ASAP
  // #ifdef MOTION_SENSOR_GPIO
  //   pinMode(MOTION_SENSOR_GPIO,INPUT);
  //   if (digitalRead(MOTION_SENSOR_GPIO)) motion = true;
  // #endif

  #ifdef MOTION_SENSOR_GPIO
    if (motion)
    {
      #ifdef ERROR_RED_LED_GPIO
        pinMode(ERROR_RED_LED_GPIO, OUTPUT);
        digitalWrite(ERROR_RED_LED_GPIO, HIGH);
      #endif
      Serial.printf("[%s]: motion DETECTED\n",__func__);
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: motion CLEARED\n",__func__);
      #endif
    }
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

// custom SDA & SCL
  #if (USE_CUSTOM_I2C_GPIO == 1)
    Wire.setPins(SDA_GPIO,SCL_GPIO);
  #endif

// start Wire after enabling 3.3V
  Wire.begin(); // it takes 0.6ms


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
  #ifdef DEBUG
    char firmware_file[255];
    snprintf(firmware_file,sizeof(firmware_file),"%s/01-Production/0-ESPnow/%s/%s",UPDATE_FIRMWARE_HOST,HOSTNAME,FW_BIN_FILE);
    Serial.printf("[%s]: firmware file:\n %s\n",__func__,firmware_file);
  #endif
// Firmware update END



// check if device is charging
  snprintf(charging,4,"%s","N/A");
  #if (defined(CHARGING_GPIO) and defined(POWER_GPIO))
    #ifdef DEBUG
      Serial.printf("[%s]: CHARGING_GPIO and POWER_GPIO enabled\n",__func__);
    #endif
    pinMode(CHARGING_GPIO, INPUT_PULLDOWN);  //both down: NC initially, will be changed when checked
    pinMode(POWER_GPIO, INPUT_PULLDOWN);
    charging_state();
  #else
    // snprintf(charging,4,"%s","N/A");
    #ifdef DEBUG
      Serial.printf("[%s]: checking CHARGING DISABLED\n",__func__);
    #endif
  #endif

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
  if (!gather_data())
  {
    #ifdef ERROR_RED_LED_GPIO
      digitalWrite(ERROR_RED_LED_GPIO,HIGH);
    #endif
    Serial.printf("[%s]: NOT sending ANY data - gethering data FAILED!\n",__func__);

    // needed here as well due to "return" below - without it, sleep current is 10x more (300uA)
    // turn off power for sensors
    #ifdef ENABLE_3V_GPIO
      digitalWrite(ENABLE_3V_GPIO, LOW);
    #endif

    return;
  } else
  {
    #ifdef DEBUG
      em = millis(); tt = em - program_start_time;
      Serial.printf("[%s]: gathering data took: %dms\n",__func__,tt);
    #endif
  }
// gather data END

// turn off power for sensors - all code below is only valid if gather_data() was successfull
  #ifdef ENABLE_3V_GPIO
    digitalWrite(ENABLE_3V_GPIO, LOW);
  #endif

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

  // send_data(motion);
  send_data();

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
