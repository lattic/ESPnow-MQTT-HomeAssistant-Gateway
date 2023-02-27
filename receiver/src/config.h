#pragma once
/*
config
*/
// wifi common settings

#define WAIT_FOR_WIFI               5     // in seconds
#define MQTT_DEVICE_IDENTIFIER      String(WiFi.macAddress())


// configuration template:
#if DEVICE_ID == 29
  #define BOARD_TYPE                2             // 1=ESP32 2=ESP32S2 3=ESP32S3 4=ESP32C3
  #define HOSTNAME                  "esp32029"    // max 9 characters, used as name in HA
  #define IP_ADDRESS                "192.168.1.29"  // fixed IP, not in use anymore for WiFi, only for name in HA
  #define ROLE_NAME                 "gw1-hall"      // part of the name in HA (gateway 1)

  #define LED_GPIO_SENSORS          2   // RED
  #define LED_GPIO_GATEWAY          5   // GREEN
  #define LED_GPIO_STANDBY          3   // BLUE

  #define LED_GPIO_SENSORS_USE_PWM  1
  #define LED_GPIO_SENSORS_PWM_DC   10

  #define LED_GPIO_GATEWAY_USE_PWM  1
  #define LED_GPIO_GATEWAY_PWM_DC   10

  #define LED_GPIO_STANDBY_USE_PWM  1
  #define LED_GPIO_STANDBY_PWM_DC   10

  #define PUSH_BUTTON_GPIO          0  // to control ESP
  #define PUSH_BUTTON_GPIO_ACT    LOW  // HIGH or LOW, HIGH = 1, LOW = 0, 0 if not defined (so default)

  uint8_t FixedMACAddress[] =       {0x2A, 0xFF, 0x01, 0x01, 0x01, 0x29};
  
  #define OTA_ACTIVE                1

  #define COMMAND_QUEUE_TIMEOUT_S   2* 60 * 60  // 2h, in seconds, clear the commands queue for sender after timeout

  #define USE_WEB_SERIAL            1
  #define CP_TIMEOUT_S              180

  #pragma message "compilation for: ESPnow_esp32029_gw1"

#elif DEVICE_ID == 30
  #define BOARD_TYPE                2
  #define HOSTNAME                  "esp32030"
  #define IP_ADDRESS                "192.168.1.30"
  #define ROLE_NAME                 "gw2-dining"
  #define LED_GPIO_SENSORS          3   // RED
  #define LED_GPIO_GATEWAY          5   // GREEN
  #define LED_GPIO_STANDBY          2   // BLUE

  #define LED_GPIO_SENSORS_USE_PWM  1
  #define LED_GPIO_SENSORS_PWM_DC   10

  #define LED_GPIO_GATEWAY_USE_PWM  1
  #define LED_GPIO_GATEWAY_PWM_DC   10

  #define LED_GPIO_STANDBY_USE_PWM  1
  #define LED_GPIO_STANDBY_PWM_DC   10

  #define PUSH_BUTTON_GPIO          0  // to control ESP
  #define PUSH_BUTTON_GPIO_ACT    LOW  // HIGH or LOW, HIGH = 1, LOW = 0, 0 if not defined (so default)

  uint8_t FixedMACAddress[] =       {0x2A, 0xFF, 0x01, 0x01, 0x01, 0x30};
  
  #define OTA_ACTIVE                1

  #define COMMAND_QUEUE_TIMEOUT_S   2* 60 * 60  // 2h, in seconds, clear the commands queue for sender after timeout

  #define USE_WEB_SERIAL            1
  #define CP_TIMEOUT_S              180

  #pragma message "compilation for: ESPnow_esp32030_gw2"

#elif DEVICE_ID == 27
  #define BOARD_TYPE                2
  #define HOSTNAME                  "esp32027"
  #define ROLE_NAME                 "gw3-bedroom"
  #define LED_GPIO_SENSORS          2   // RED
  #define LED_GPIO_GATEWAY          4   // GREEN
  #define LED_GPIO_STANDBY          6   // BLUE

  #define PUSH_BUTTON_GPIO          0  // to control ESP
  #define PUSH_BUTTON_GPIO_ACT    LOW  // HIGH or LOW, HIGH = 1, LOW = 0, 0 if not defined (so default)
  
  #define LED_GPIO_SENSORS_USE_PWM  1
  #define LED_GPIO_SENSORS_PWM_DC   10

  #define LED_GPIO_GATEWAY_USE_PWM  1
  #define LED_GPIO_GATEWAY_PWM_DC   10

  #define LED_GPIO_STANDBY_USE_PWM  1
  #define LED_GPIO_STANDBY_PWM_DC   10

  // #define PUSH_BUTTON_GPIO          0  // to control ESP
  // #define PUSH_BUTTON_GPIO_ACT    LOW  // HIGH or LOW, HIGH = 1, LOW = 0, 0 if not defined (so default)

  uint8_t FixedMACAddress[] =       {0x2A, 0xFF, 0x01, 0x01, 0x01, 0x27};
  
  #define OTA_ACTIVE                1

  #define COMMAND_QUEUE_TIMEOUT_S   2* 60 * 60  // 2h, in seconds, clear the commands queue for sender after timeout

  #define USE_INFLUXDB              0
  #define INFLUXDB_URL              "http://ip-here:8086"
  #define INFLUXDB_TOKEN            "-zzgmINum9Hps5gGmkasdfaskWpXCUekv5I-WgqbdPstVNvUmvRuktyZMgFw0vHkZm23gPcnJ1TqHrLttfIw=="
  #define INFLUXDB_ORG              "test-org"
  #define INFLUXDB_BUCKET           "test_bucket"
  #define MY_TIME_ZONE              "GMT0BST,M3.5.0/1,M10.5.0" 

  #define USE_WEB_SERIAL            1
  #define CP_TIMEOUT_S              180

  #define MEASURE_LUX               1
  #define MEASURE_LUX_GPIO          11
  #define LUX_MAX_RAW_READING       7800  // resistors specific max value - variable as each sensor is done with different voltage divider
  #define LUX_MAX_RAW_MAPPED_READING 1000 // TEPT4400 can measure only up to 1000lx then it gets satturated
  #define LUX_MIN_RAW_READING       0     // set it to min shown when TEPT4400 is completely covered to get 0lx 


  #pragma message "compilation for: ESPnow_esp32027_gw3"  


// ************************************* C L I E N T S ***********************************************

#elif DEVICE_ID == 101
  #define CLIENT                    "003-krzych"
  #define BOARD_TYPE                2
  #define HOSTNAME                  "krz101"
  #define ROLE_NAME                 "gateway1"
  #define LED_GPIO_SENSORS          5   // RED
  #define LED_GPIO_GATEWAY          6   // GREEN
  #define LED_GPIO_STANDBY          4   // BLUE

  #define LED_GPIO_SENSORS_USE_PWM  1
  #define LED_GPIO_SENSORS_PWM_DC   10

  #define LED_GPIO_GATEWAY_USE_PWM  1
  #define LED_GPIO_GATEWAY_PWM_DC   10

  #define LED_GPIO_STANDBY_USE_PWM  1
  #define LED_GPIO_STANDBY_PWM_DC   10

  // #define PUSH_BUTTON_GPIO          0  // to control ESP
  // #define PUSH_BUTTON_GPIO_ACT    LOW  // HIGH or LOW, HIGH = 1, LOW = 0, 0 if not defined (so default)

  uint8_t FixedMACAddress[] =       {0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01};
  
  #define OTA_ACTIVE                1

  #define COMMAND_QUEUE_TIMEOUT_S   30 * 60  // 15min, in seconds, clear the commands queue for sender after timeout

  #define USE_INFLUXDB              0
  #define INFLUXDB_URL              "http://ip-here:8086"
  #define INFLUXDB_TOKEN            "-zzgmINum9Hps5gGmkasdfaskWpXCUekv5I-WgqbdPstVNvUmvRuktyZMgFw0vHkZm23gPcnJ1TqHrLttfIw=="
  #define INFLUXDB_ORG              "test-org"
  #define INFLUXDB_BUCKET           "test_bucket"
  #define MY_TIME_ZONE              "GMT0BST,M3.5.0/1,M10.5.0" 

  #define USE_WEB_SERIAL            1

  #define MEASURE_LUX               1
  #define MEASURE_LUX_GPIO          1
  #define LUX_MAX_RAW_READING       7800  // resistors specific max value - variable as each sensor is done with different voltage divider
  #define LUX_MAX_RAW_MAPPED_READING 1000 // TEPT4400 can measure only up to 1000lx then it gets satturated
  #define LUX_MIN_RAW_READING       0     // set it to min shown when TEPT4400 is completely covered to get 0lx 

  #pragma message "compilation for: ESPnow_krz101_gateway1"


#elif DEVICE_ID == 103
  #define CLIENT                    "003-krzych"
  #define BOARD_TYPE                2
  #define HOSTNAME                  "krz103"
  #define ROLE_NAME                 "gateway3"
  #define LED_GPIO_SENSORS          2   // RED
  #define LED_GPIO_GATEWAY          4   // GREEN
  #define LED_GPIO_STANDBY          6   // BLUE

  #define LED_GPIO_SENSORS_USE_PWM  1
  #define LED_GPIO_SENSORS_PWM_DC   10

  #define LED_GPIO_GATEWAY_USE_PWM  1
  #define LED_GPIO_GATEWAY_PWM_DC   10

  #define LED_GPIO_STANDBY_USE_PWM  1
  #define LED_GPIO_STANDBY_PWM_DC   10

  // MHZ19
  #define USE_MHZ19_CO2             1     // use "0" to disable
  #define RX_PIN                    11    // Rx pin which the MHZ19 Tx pin is attached to - BROWN
  #define TX_PIN                    10    // Tx pin which the MHZ19 Rx pin is attached to - BLACK
  #define BAUDRATE                 9600   // Device to MH-Z19 Serial baudrate (should not be changed)
  #define MHZ19_AUTO_CALIBRATION    0   // don't autocalibrate MHZ19
  #define CO2_UPDATE_INTERVAL_S     120   // 120, how often to measure CO2 - every 2m as per datasheet
  #define CO2_PREHEAT_TIME_S        180   // 180, initial preheat time as per datasheet 
  #define MIN_CO2                   330   // 330, invalid reading below 330 ppm as per datasheet (400 +-50 +-5%)
  #define CO2_CALIBRATE_TIME_S      40 * 60  // 20*60=20min as per datasheet

  // #define PUSH_BUTTON_GPIO          0  // to control ESP
  // #define PUSH_BUTTON_GPIO_ACT    LOW  // HIGH or LOW, HIGH = 1, LOW = 0, 0 if not defined (so default)

  uint8_t FixedMACAddress[] =       {0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03};

  #define OTA_ACTIVE                0
  #define COMMAND_QUEUE_TIMEOUT_S   30 * 60  // 15min, in seconds, clear the commands queue for sender after timeout

  #define USE_INFLUXDB              0
  #define INFLUXDB_URL              "http://ip-here:8086"
  #define INFLUXDB_TOKEN            "-zzgmINum9Hps5gGmkasdfaskWpXCUekv5I-WgqbdPstVNvUmvRuktyZMgFw0vHkZm23gPcnJ1TqHrLttfIw=="
  #define INFLUXDB_ORG              "test-org"
  #define INFLUXDB_BUCKET           "test_bucket"
  #define MY_TIME_ZONE              "GMT0BST,M3.5.0/1,M10.5.0" 

  #define USE_BMP280                1     // use "0" to disable
  #define BMP280_TEMPERATURE_CALIBRATION -2.3
  #define AIR_PRESSURE_CALIBRATION  -1.4  // callibration as per https://www.eldoradoweather.com/forecast/forecast-search-results.php

  #define USE_WEB_SERIAL            1

  #define MEASURE_LUX               1
  #define MEASURE_LUX_GPIO          3
  #define LUX_MAX_RAW_READING       7800  // resistors specific max value - variable as each sensor is done with different voltage divider
  #define LUX_MAX_RAW_MAPPED_READING 1000 // TEPT4400 can measure only up to 1000lx then it gets satturated
  #define LUX_MIN_RAW_READING       0     // set it to min shown when TEPT4400 is completely covered to get 0lx 

  #pragma message "compilation for: ESPnow_krz103_gateway3"    


#elif DEVICE_ID == 92
  #define BOARD_TYPE                2
  #define HOSTNAME                  "esp32092"
  #define ROLE_NAME                 "gw92-test"
  #define LED_GPIO_SENSORS          3   // RED
  #define LED_GPIO_GATEWAY          5   // GREEN
  #define LED_GPIO_STANDBY          2   // BLUE

  #define LED_GPIO_SENSORS_USE_PWM  1
  #define LED_GPIO_SENSORS_PWM_DC   10

  #define LED_GPIO_GATEWAY_USE_PWM  1
  #define LED_GPIO_GATEWAY_PWM_DC   10

  #define LED_GPIO_STANDBY_USE_PWM  1
  #define LED_GPIO_STANDBY_PWM_DC   10

  #define PUSH_BUTTON_GPIO          0  // to control ESP
  #define PUSH_BUTTON_GPIO_ACT    LOW  // HIGH or LOW, HIGH = 1, LOW = 0, 0 if not defined (so default)

  uint8_t FixedMACAddress[] =       {0x2A, 0xFF, 0x01, 0x01, 0x01, 0x92};
  
  #define OTA_ACTIVE                1

  #define COMMAND_QUEUE_TIMEOUT_S   2* 60 * 60  // 2h, in seconds, clear the commands queue for sender after timeout

  #define USE_WEB_SERIAL            1

  #pragma message "compilation for: ESPnow_esp32092-gwtest"


#else
  #error "Wrong DEVICE_ID chosen"
#endif
// DEVICE CUSTOM SETTINGS END


// 
#ifndef LED_GPIO_GATEWAY
  #error "LED_GPIO_GATEWAY must exist for both: connect_wifi() and to indicate the Captive Portal"
#endif

// for PubSubClient:
// it is used in: mqttc.setBufferSize(MQTT_PAYLOAD_MAX_SIZE); to increase max packet size
//default is 256 but it is not enough in this program - check debug in case of issue
#define MQTT_PAYLOAD_MAX_SIZE       1024

// for ArduinoJson
#define JSON_CONFIG_SIZE            1024  // config is bigger than payload due to device information
#define JSON_PAYLOAD_SIZE           512

// mqtt - how many attempts to connect to MQTT broker before restarting
#define MAX_MQTT_ERROR              180  // 180, in seconds / times, used in mqtt.reconnect as well as in loop to check if MQTT is ok
                                         // and if not it restarts the ESP

// queue size for incomming data from sensors, whe queue is full no more data is gathered until there is free space in the queue
// set it to as much as many sensor devices you have so each one gets into the queue at least once. Unless some are very frequent (i.e. motion)
#define MAX_QUEUE_COUNT             100  //100
#define MAX_QUEUE_COMMANDS_COUNT    100  

// how often to update HA on GW status
#define HEARTBEAT_INTERVAL_S        10    //10 in seconds

// FW update timeout
#define FW_UPDATE_TIME_S            180   // in seconds - 3 minutes shall be enough

// LED PWM settings
#ifdef LED_GPIO_STANDBY_USE_PWM
  #define LED_GPIO_STANDBY_PWM_CHANNEL    1
  #define LED_GPIO_STANDBY_PWM_RESOLUTION 8
  #define LED_GPIO_STANDBY_PWM_FREQ       5000
  #define LED_GPIO_STANDBY_MIN_DC         0
  #define LED_GPIO_STANDBY_MAX_DC         255
#endif

#ifdef LED_GPIO_SENSORS_USE_PWM
  #define LED_GPIO_SENSORS_PWM_CHANNEL    2
  #define LED_GPIO_SENSORS_PWM_RESOLUTION 8
  #define LED_GPIO_SENSORS_PWM_FREQ       5000
  #define LED_GPIO_SENSORS_MIN_DC         0
  #define LED_GPIO_SENSORS_MAX_DC         255
#endif

#ifdef LED_GPIO_GATEWAY_USE_PWM
  #define LED_GPIO_GATEWAY_PWM_CHANNEL    3
  #define LED_GPIO_GATEWAY_PWM_RESOLUTION 8
  #define LED_GPIO_GATEWAY_PWM_FREQ       5000
  #define LED_GPIO_GATEWAY_MIN_DC         0
  #define LED_GPIO_GATEWAY_MAX_DC         255
#endif

#define MAX_BAD_BOOT_COUNT                30   // after it is reached, it will erase nvs and then start Captive Portal with null values


// assigning MODEL and checking if proper board is selected
#define PRINT_COMPILER_MESSAGES // comment out to disable messages in precompiler
#ifndef BOARD_TYPE
  #error BOARD_TYPE not defined
#else
  #if (BOARD_TYPE == 1) and (!defined(CONFIG_IDF_TARGET_ESP32))
    #error wrong board selected in Arduino - choose ESP32DEV
  #endif
  #if (BOARD_TYPE == 2) and (!defined(CONFIG_IDF_TARGET_ESP32S2))
    #error wrong board selected in Arduino - choose S2
  #endif
  #if (BOARD_TYPE == 3) and (!defined(CONFIG_IDF_TARGET_ESP32S3))
    #error wrong board selected in Arduino - choose S3
  #endif
  #if (BOARD_TYPE == 4) and (!defined(CONFIG_IDF_TARGET_ESP32C3))
    #error wrong board selected in Arduino - choose C3
  #endif

  #if (BOARD_TYPE == 1)
    #define MODEL "ESP32"
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32"
    #endif
  #elif (BOARD_TYPE == 2)
    #define MODEL "ESP32S2"
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32S2"
    #endif
  #elif (BOARD_TYPE == 3)
    #define MODEL "ESP32S3"
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32S3"
    #endif
  #elif (BOARD_TYPE == 4)
    #define MODEL "ESP32C3"
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32C3"
    #endif
  #else
    #error BOARD_TYPE not defined
  #endif


#endif
// assigning MODEL and checking if proper board is selected END
