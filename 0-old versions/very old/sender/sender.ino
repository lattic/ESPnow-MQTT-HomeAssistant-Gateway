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

#define DEVICE_ID  28           // "esp32028" - S,  production - Garage
// #define DEVICE_ID  100          // "esp32100" - S2, production - Papa, new
// #define DEVICE_ID  101          // "esp32101" - S2,  production - Dining, new
// #define DEVICE_ID  102          // "esp32102" - S,  production - Toilet, new
// #define DEVICE_ID  104          // "esp32104" - S,  production - Milena, new
// #define DEVICE_ID  105          // "esp32105" - S2, production - Garden

 // #define DEVICE_ID  86           // "esp32086" - S2, new device template
// #define DEVICE_ID  92           // "esp32092" - S3, test - Ai-Thinker
// #define DEVICE_ID  93           // "esp32093" - S,  ex S
//#define DEVICE_ID  95           // "esp32095" - C3, test - Ai-Thinker


// **** format FS on first deployment only, then change to 0 or comment out ****
#define FORMAT_FS   0

// version < 10 chars, description in changelog.txt
#define VERSION "1.18.0old"

// configure device in this file, choose which one you are compiling for on top of this script: #define DEVICE_ID x
#include "devices_config.h"

// ****************  ALL BELOW ALL IS COMMON FOR ANY ESP32 *********************
#define HIBERNATE           1     // 1=hibernate, 0=deep sleep
#define WIFI_CHANNEL        8     // in my house

#define WAIT_FOR_WIFI       5     // in seconds, for upgrade firmware
#ifndef PERIODIC_FW_CHECK_HRS     // if not found custom PERIODIC_FW_CHECK_HRS in devices_config.h (per device custom)
  #define PERIODIC_FW_CHECK_HRS  14400 //168 = 7 days, 14400 = 30 days for SLEEP_TIME = 180s (3 minutes)
#endif
#ifndef MINIMUM_VOLTS
  #define MINIMUM_VOLTS     3.3   // if not found in device specific config
#endif
// ******************************  some consistency checks *************************
#if ((USE_TSL2561 == 1) and (USE_TEPT4400 == 1))
  #error "use either USE_TSL2561 or USE_TEPT4400 for measuring lux - not both"
#endif

#if ((USE_TEPT4400 == 1) and (!defined(LUX_ADC_GPIO) or !defined(LUX_MAX_RAW_READING)))
  #error "for measuring lux via TEPT4400 define LUX_ADC_GPIO and LUX_MAX_RAW_READING"
#endif

#if (!defined (HOSTNAME) or !defined(DEVICE_NAME) or !defined(BOARD_TYPE) or !defined(SLEEP_TIME))
  #error "HOSTNAME, DEVICE_NAME, BOARD_TYPE and SLEEP_TIME are obligatory"
#endif


// ****************  LOAD libraries and initiate variables *************************
#include <passwords.h>          // passwords inside my library - for my personal usage
// #include "passwords.h"          // passwords in current folder - for github - see the structure inside the file

// DRD config
// These defines must be put before #include <ESP_DoubleResetDetector.h>
// to select where to store DoubleResetDetector's variable.
#define ESP_DRD_USE_LITTLEFS        true
#ifdef DEBUG
  #define DOUBLERESETDETECTOR_DEBUG true
#endif
#include <ESP_DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector
//timeout  between 2 resets in seconds
#define DRD_TIMEOUT 1
DoubleResetDetector* drd;
bool DRD_Detected = false;
// DRD config END

// Firmware update
#include <HTTPClient.h>
#include <Update.h>
#if   (BOARD_TYPE == 1)
  #define FW_BIN_FILE "sender.ino.esp32.bin"
#elif (BOARD_TYPE == 2)
  #define FW_BIN_FILE "sender.ino.esp32s2.bin"
#elif (BOARD_TYPE == 3)
  #define FW_BIN_FILE "sender.ino.esp32s3.bin"
#elif (BOARD_TYPE == 4)
  #define FW_BIN_FILE "sender.ino.esp32c3.bin"
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

// sht31 - temperature and humidity, SDA
#if (USE_SHT31 == 1)
  #include "Adafruit_SHT31.h"
  Adafruit_SHT31 sht31 = Adafruit_SHT31();
#endif

// lux from TETP4400/ADC
#if (USE_TEPT4400 == 1)
  // TETP440 can measure up to 1000 lux so anything above is meaningless
  #define LUX_MAX_RAW_MAPPED_READING 1000
#endif

// lux from TSL2561 - light sensor, SDA
#if (USE_TSL2561 == 1)
  #include <SparkFunTSL2561.h>
  SFE_TSL2561 light;
#endif

// MAX17048 - battery fuel gauge, SDA
#if (USE_MAX17048 == 1)
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
// below: first and second receivers - no difference in any task
// first receiver - gw1 - odd bootCount
uint8_t broadcastAddress1[]     = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xff};
// second receiver - gw2 - even bootCount
uint8_t broadcastAddress2[]     = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xee};

#if (USE_FAKE_RECEIVER == 1)
  // fake receiver
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
// become false if sensor is not initiated, it sends then "0.0"
bool sht31ok = true;
bool max17ok = true;
bool tslok   = true;
long program_start_time,em,tt,start_espnow_time;
int bootCount = 1;
unsigned long saved_ontime_l = 0;
unsigned long current_ontime_l = 0;
byte boot_reason;

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
void lux_with_tept(long int* lux_int);
void charging_state();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
bool readFile(fs::FS &fs, const char * path, char * data);
bool writeFile(fs::FS &fs, const char * path, const char * message);
void load_ontime();
void save_ontime();
void hibernate();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
bool gather_data();
void send_data();
void disable_espnow();
void setup_wifi();
void sos(int led);
void do_update();
void updateFirmware(uint8_t *data, size_t len);
int update_firmware_prepare();
void do_esp_restart();


// implementation
// lux from ADC
#if (USE_TEPT4400 == 1)
  void lux_with_tept(long int* lux_int)
  {
    // it takes 5ms max
    #ifdef DEBUG
      long sm = millis();
    #endif

    uint32_t read_digital = 0;
    // size_t nbytes;

    analogSetPinAttenuation(LUX_ADC_GPIO, ADC_11db);
    adcAttachPin(LUX_ADC_GPIO);
    for ( byte i = 0; i < 10; i++) {
        read_digital += analogRead(LUX_ADC_GPIO);
    }
    read_digital /= 10;

    #ifdef DEBUG
      Serial.printf("[%s]: read_digital=%d\n",__func__,read_digital);
    #endif

    if (read_digital > LUX_MAX_RAW_READING) read_digital = LUX_MAX_RAW_READING;
    *lux_int = map(read_digital, 0, LUX_MAX_RAW_READING, 0, LUX_MAX_RAW_MAPPED_READING);
    // #ifdef DEBUG
    //   long mt=millis()-sm;
    //   Serial.printf("\t[%s]: lux_with_tept TIME: %dms\n",__func__,mt);
    // #endif
  }
#endif


// check charging
#if defined(CHARGING_GPIO) and defined(POWER_GPIO)
  void charging_state()
  {
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
  }
#endif


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
    Serial.printf("[%s]: Writing file: %s\r",__func__,path);
  #endif
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    #ifdef DEBUG
      Serial.printf(" - failed to open file for writing\n",__func__);
    #endif
    return false;
  }
  if(file.print(message)){
    #ifdef DEBUG
      Serial.printf(" - file written\n",__func__);
    #endif
  } else {
    #ifdef DEBUG
      Serial.printf(" - write failed\n",__func__);
    #endif
  }
  file.close();
  return true;
}


void load_ontime()
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
        Serial.printf("[%s]: Device is CHARGING, reset: saved_ontime_l=%lums\n",__func__,saved_ontime_l);
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


void do_esp_restart()
{
  save_ontime();
  ESP.restart();
}


// deep sleep
void hibernate()
{
  // wifi
  esp_wifi_stop();
  // all gpio isolated
  esp_sleep_config_gpio_isolate();
  // it does not display welcome message
  esp_deep_sleep_disable_rom_logging();

  #if (HIBERNATE == 1)
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    // added extra, check if no issue
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M,         ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,       ESP_PD_OPTION_OFF);
    #ifdef DEBUG
      Serial.printf("[%s]: sleep mode: hibernate\n",__func__);
    #endif
  #else
    #ifdef DEBUG
      Serial.printf("[%s]: sleep mode: deep sleep\n",__func__);
    #endif
  #endif
  save_ontime();
  Serial.printf("[%s]: Bye...\n========= E N D =========\n",__func__);
  esp_deep_sleep_start();
}


// on sent callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  {
    if (status == ESP_NOW_SEND_SUCCESS)
    {
      Serial.printf("[%s]: ESPnow SUCCESSFULL\n",__func__);
    } else
    {
      // it never returns FAILED in case of MAC_BROADCAST used - work on it!
      #ifdef ERROR_RED_LED_GPIO
        digitalWrite(ERROR_RED_LED_GPIO,HIGH);
      #endif
      Serial.printf("[%s]: ESPnow FAILED\n",__func__);
    }
    tt=millis() - start_espnow_time;
    Serial.printf("[%s]: ESPnow took:....%dms\n",__func__,tt);
  }
}


// gather data from sensors, returns false if (bat_volts < MINIMUM_VOLTS) so ESPnow will not start - goes to sleep instead of sending
bool gather_data()
{
  #ifdef DEBUG
    Serial.printf("[%s]:\n",__func__);
  #endif
  // sender_type - "env" so motion=0
  snprintf(myData.sender_type,sizeof(myData.sender_type),"%s","env");
  myData.motion = 0;    // always 0 for normal env sensor
  #ifdef DEBUG
    Serial.printf(" Data gatehered:\n\tsender_type=%s\n",myData.sender_type);
    Serial.printf("\tmotion=%d\n",myData.motion);
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
  #if (USE_TEPT4400 == 1)
    long int lux_int;
    lux_with_tept(&lux_int);
    myData.lux = (float)lux_int;

  #elif (USE_TSL2561 == 1)
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
      // testing delay 195ms as per MAX17048 datasheet - needed only on battery change - disabling it
        // long wait_ms = 195 - (millis()-program_start_time);
        // if (wait_ms > 0)
        // {
        //   Serial.printf("\n\n\ndelaying for %dms\n\n\n",wait_ms);
        //   delay(wait_ms);
        // }
      // testing delay 195ms END

      myData.bat = lipo.getVoltage();
      myData.batpct = lipo.getSOC();
      myData.batchr = lipo.getChangeRate();

      // EMERGENCY - LOW BATTERY!
      if (myData.bat < MINIMUM_VOLTS)
      {
        // #ifdef DEBUG
          Serial.printf("[%s]: battery volts=%0.2fV, that is below minimum [%0.2fV]\n",__func__,myData.bat,MINIMUM_VOLTS);
        // #endif

        if (DRD_Detected)
        {
          // #ifdef DEBUG
            tt = millis() - program_start_time;
            Serial.printf("[%s]: NOT hibernating as DRD detected %dms after boot\n",__func__,tt);
          // #endif
        } else
        {
          #ifdef DEBUG
            tt = millis() - program_start_time;
            Serial.printf("[%s]: NOT sending data! leaving [gather_data=false] %dms after boot\n",__func__,tt);
          #endif
          return false;
        }
      } else
      {
        #ifdef DEBUG
          Serial.printf("\tbattery level=OK\n");
        #endif
      }
    }
    // send MAX17048 to sleep to disable measurements while sleeping, reduce current by 3uA more
    // it does not measure battery during sleep so continuity is lost - not enabling it
    // anyway, if ESP32S measures only 4.5uA during hibernate, it means it is irrelevant for the sleep - it only disables periodic (every 45s) measuring of the battery
    // lipo.sleep();
  #endif
  #ifdef DEBUG
    Serial.printf("\tbat=%fV\n",myData.bat);
    Serial.printf("\tbatpct=%f%%\n",myData.batpct);
    Serial.printf("\tbatchr=%f%%\n",myData.batchr);
  #endif

  // version
  if (DRD_Detected)
  {
    // send to HA "FW UPG..." instead of version during FW upgrade to indicate the process
    snprintf(myData.ver,sizeof(myData.ver),"%s","FW UPG...");
  } else
  {
    strcpy(myData.ver, VERSION);
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

  // // convert MAC address to char array
  // uint8_t MAC_array[6];
  // char MAC_char[18];
  // WiFi.macAddress(MAC_array);
  // for (int i = 0; i < sizeof(MAC_array); ++i)
  // {
  //   if (i<sizeof(MAC_array)-1)
  //   {
  //     sprintf(MAC_char,"%s%02x:",MAC_char,MAC_array[i]);
  //   } else
  //   {
  //     sprintf(MAC_char,"%s%02x",MAC_char,MAC_array[i]);
  //   }
  // }
  // // convert MAC address to char array END

  uint8_t MAC_array[6];
  WiFi.macAddress(MAC_array);
  Serial.printf("[%s]: my MAC address:%02X:%02X:%02X:%02X:%02X:%02X\n",__func__,MAC_array[0],MAC_array[1],MAC_array[2],MAC_array[3],MAC_array[4],MAC_array[5]);

  // Serial.printf("\n[%s]: ESP Mac Address: %s\n",__func__,MAC_char);
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

  // print IP address from router
  IPAddress ip = WiFi.localIP();
  char buf[17];
  sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("[%s]: my IP address: %s\n",__func__,buf);
}


// blink nicely - sos for upgrade failure
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
  int update_firmware_status = -1;
  update_firmware_status=update_firmware_prepare();
  if (update_firmware_status == 0)
  {
    Serial.printf("[%s]: RESTARTING - FW update SUCCESSFULL\n\n",__func__);
    // blink nicely when FW upgrade successfull
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
    if (update_progress % 5 == 0){ //if uncomment it prints every 2%, otherwise every 5%
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


// setup
void setup()
{
  program_start_time = millis();

  // welcome screen
  Serial.begin(115200);
  Serial.printf("\n======= S T A R T =======\n");
  Serial.printf("[%s]: Device: %s (%s)\n",__func__,DEVICE_NAME,HOSTNAME);

  #ifdef DEBUG
    // print MAC to enable it on router
    uint8_t MAC_array[6];
    WiFi.macAddress(MAC_array);
    Serial.printf("[%s]: my MAC address:%02X:%02X:%02X:%02X:%02X:%02X\n",__func__,MAC_array[0],MAC_array[1],MAC_array[2],MAC_array[3],MAC_array[4],MAC_array[5]);
  #endif

  esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_S_FACTOR);

  boot_reason = esp_reset_reason();
  //1 = reset pressed or battery changed/power reconnected
  //3 = ESP.restart()
  //8 = from sleep

  // LEDS: BLUE ON, RED OFF, GND for LED LOW if needed
  #ifdef ERROR_RED_LED_GPIO
    pinMode(ERROR_RED_LED_GPIO,OUTPUT);
    digitalWrite(ERROR_RED_LED_GPIO, LOW);
  #endif
  #ifdef ACT_BLUE_LED_GPIO
    pinMode(ACT_BLUE_LED_GPIO,OUTPUT);
    digitalWrite(ACT_BLUE_LED_GPIO, HIGH);
  #endif
  #ifdef GND_GPIO_FOR_LED
    #ifdef DEBUG
      Serial.printf("[%s]: Enabling GND for LED on GPIO=%d\n",__func__,GND_GPIO_FOR_LED);
    #endif
    pinMode(GND_GPIO_FOR_LED, OUTPUT);
    digitalWrite(GND_GPIO_FOR_LED, LOW);
  #endif

// testing with PPK2
  #ifdef PPK2_GPIO
    pinMode(PPK2_GPIO,OUTPUT);
    digitalWrite(PPK2_GPIO,HIGH);
  #endif

// custom SDA & SCL
  #if (USE_CUSTOM_I2C_GPIO == 1)
    Wire.setPins(SDA_GPIO,SCL_GPIO);
  #endif

// power for sensors from GPIO - MUST be before any SDA sensor is in use obviously!
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
      // delay(1000);
      #ifdef DEBUG
        Serial.printf("[%s]: start MAX17048 OK\n",__func__);
      #endif
      // quickStart restarts measuring change rate - disabling it - test it!
      // lipo.quickStart();
      if (boot_reason == 1)
      {
        #ifdef DEBUG
          Serial.printf("[%s]: Resetting MAX17048 on reset button pressed or battery changed/power reconnected\n",__func__);
        #endif
        lipo.reset();
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

// DRD
  drd = new DoubleResetDetector(DRD_TIMEOUT, 0);
  if (drd->detectDoubleReset())
  {
    drd->stop(); //needed here? I did not figure it out yet
    // #ifdef DEBUG
      Serial.printf("[%s]: Double Reset Detected\n",__func__);
    // #endif
    DRD_Detected = true;
    #ifdef ERROR_RED_LED_GPIO
      digitalWrite(ERROR_RED_LED_GPIO, HIGH);
    #else
      #ifdef ACT_BLUE_LED_GPIO
        digitalWrite(ACT_BLUE_LED_GPIO, HIGH);
      #endif
    #endif

  }
// DRD END

// read/increase/save bootCount
  char data[12];
  if(!LittleFS.begin(true))
  {
    Serial.printf("[%s]: LittleFS Mount Failed\n",__func__);
  } else
  {
    // format FS on first deployment
    #if (FORMAT_FS == 1)
      Serial.printf("[%s]: formatting FS...",__func__);
      if (LittleFS.format())
      {
        Serial.printf("SUCCESSFULL\n",__func__);
      } else
      {
        #ifdef ERROR_RED_LED_GPIO
          digitalWrite(ERROR_RED_LED_GPIO,HIGH);
        #endif
        Serial.printf("FAILED\n",__func__);
      }
    #endif
    // list files in DEBUT mode only
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
      if (bootCount >= (3600 / (SLEEP_TIME)) * (PERIODIC_FW_CHECK_HRS))
      {
        Serial.printf("[%s]: time to check FW\n",__func__);
        // cheating here: reusing DRD_Detected to initiate the FW upgrade
        DRD_Detected = true;
        bootCount = 1;
      }
    }
    // reset bootCount on DRD_Detected or on button reset
    if ((DRD_Detected) or (boot_reason == 1)) bootCount = 1;

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
// save bootCount END

// load saved ontime
  load_ontime();

// check if device is charging
  #if (defined(CHARGING_GPIO) and defined(POWER_GPIO))
    #ifdef DEBUG
      Serial.printf("[%s]: CHARGING_GPIO and POWER_GPIO enabled\n",__func__);
    #endif
    pinMode(CHARGING_GPIO, INPUT_PULLDOWN);  //both down: NC initially, will be changed when checked
    pinMode(POWER_GPIO, INPUT_PULLDOWN);
    // checking charging in setup already
    charging_state();
  #else
    snprintf(charging,4,"%s","N/A");
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
    // #ifdef DEBUG
      Serial.printf("[%s]: NOT sending ANY data - gethering data FAILED!\n",__func__);
    // #endif

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

  // sender chooses the receiver based on bootCount (not for motion sensors - only env sensors) or broadcast
  char receiver_mac[18];
  #if (USE_FAKE_RECEIVER == 1)
    #pragma message "NOT sending data to any receiver - FAKE MAC ADDRESS USED"
    memcpy(broadcastAddress, broadcastAddress3, sizeof(broadcastAddress));
    // #ifdef DEBUG
    snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);
    Serial.printf("[%s]: bootCount=%d, receiver Fake, MAC=%s\n",__func__,bootCount,receiver_mac);
    // #endif
  #else
    #if (MAC_BROADCAST == 1)
      #pragma message "sending data to all ESP - BROADCAST"
      memcpy(broadcastAddress, broadcastAddress_all, sizeof(broadcastAddress));
      #ifdef DEBUG
        snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);
        Serial.printf("[%s]: bootCount=%d, receiver Broadcast, MAC=%s\n",__func__,bootCount,receiver_mac);
      #endif
    #else
      #pragma message "sending data to specific ESP - UNICAST"
      if (bootCount % 2 == 0)
      {
        memcpy(broadcastAddress, broadcastAddress1, sizeof(broadcastAddress));
        #ifdef DEBUG
          snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);
          Serial.printf("[%s]: bootCount=%d, receiver 1st, MAC=%s\n",__func__,bootCount,receiver_mac);
        #endif
      } else
      {
        memcpy(broadcastAddress, broadcastAddress2, sizeof(broadcastAddress));
        #ifdef DEBUG
          snprintf(receiver_mac, sizeof(receiver_mac), "%02x:%02x:%02x:%02x:%02x:%02x",broadcastAddress[0], broadcastAddress[1], broadcastAddress[2], broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);
          Serial.printf("[%s]: bootCount=%d, receiver 2nd, MAC=%s\n",__func__,bootCount,receiver_mac);
        #endif
      }
    #endif
  #endif

// ESPNow preparation
  start_espnow_time = millis();

  WiFi.mode(WIFI_MODE_STA);
  // check if WIFI_PROTOCOL_LR works
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR);
  // check if WIFI_PROTOCOL_LR works END

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
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    #ifdef DEBUG
      Serial.printf("[%s]: ESP NOW pairing failure\n",__func__);
    #endif
  } else
  {
    send_data();
  }
}


// loop
void loop()
{
  // second time print as sometimes serial does not show the beginning - just for debug
  #ifdef DEBUG
    Serial.printf("[%s]: boot_reason=%d\n",__func__,boot_reason);
  #endif

  drd->loop();
  // DRD_Detected = true; //drain battery by performing upgrade always ;-)
  if (!DRD_Detected)
  {
    drd->stop();
    Serial.printf("[%s]: Going to sleep\n",__func__);
    hibernate();
  }
  else
  {
    Serial.printf("[%s]: Update firmware scheduled\n",__func__);
    drd->stop();
    disable_espnow();
    setup_wifi();
    do_update();
  }
}
