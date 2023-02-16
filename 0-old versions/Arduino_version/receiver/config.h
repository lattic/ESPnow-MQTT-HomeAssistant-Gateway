#pragma once
/*
config
*/
// wifi common settings
#define WIFI_CHANNEL                8     // channel must be the same as for sender devices!
#define IP_GATEWAY                  "192.168.1.1"
#define IP_SUBNET                   "255.255.255.0"
#define IP_DNS                      "192.168.1.1"
#define WAIT_FOR_WIFI               5     // in seconds
#define OTA_ACTIVE

#include <passwords.h>              // passwords inside my library - for my personal usage
// #include "passwords.h"           // passwords in current folder - for github - see the structure inside the file

#include "variables.h"

// DEVICE CUSTOM SETTINGS END
/*
configuration template:
#if DEVICE_ID == 88                 // unique ID defining the gateway device, must be chosen in receiver.ino
  #define BOARD_TYPE                2               // 1=ESP32 2=ESP32S2 3=ESP32S3 4=ESP32C3
  #define HOSTNAME                  "esp32088"      // max 9 characters, used as name in HA
  #define IP_ADDRESS                "192.168.1.88"  // fixed IP, not in use anymore for WiFi, only for name in HA
  #define ROLE_NAME                 "gw1"           // part of the name in HA (gateway 1)
  #define SENSORS_LED_GPIO_BLUE     3               // blinking during receiving and sending sensors data to HA - blue
  #define STATUS_GW_LED_GPIO_RED    2               // blinking during sending gateway data to HA - red
  #define POWER_ON_LED_GPIO_GREEN   5               // status of device - green
  #define POWER_ON_LED_USE_PWM      1               // set to 1 if PWM to be used, 0 for fixed brightness
  #define POWER_ON_LED_DC           30              // DC for Green LED to avoid too bright
  // fixed MAC address assigned to ther receiver, to be able to use any ESP32 without changing the code of sender
  // chose any free one [i.e. from broken ESP] and match with sender.ino
  uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xff};
  #pragma message "compilation for: ESPnow_esp32029_gw1"
*/
#if DEVICE_ID == 29
  #define BOARD_TYPE                2
  #define HOSTNAME                  "esp32029"
  #define IP_ADDRESS                "192.168.1.29"
  #define ROLE_NAME                 "gw1-hall"
  #define SENSORS_LED_GPIO_BLUE     3
  #define STATUS_GW_LED_GPIO_RED    2
  #define POWER_ON_LED_GPIO_GREEN   5
  #define POWER_ON_LED_USE_PWM      1
  #define POWER_ON_LED_DC           30
  uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xFF};
  #pragma message "compilation for: ESPnow_esp32029_gw1"

#elif DEVICE_ID == 30
  #define BOARD_TYPE                2
  #define HOSTNAME                  "esp32030"
  #define IP_ADDRESS                "192.168.1.30"
  #define ROLE_NAME                 "gw2-dining"
  #define SENSORS_LED_GPIO_BLUE     2
  #define STATUS_GW_LED_GPIO_RED    3
  #define POWER_ON_LED_GPIO_GREEN   5
  #define POWER_ON_LED_USE_PWM      1
  #define POWER_ON_LED_DC           20
  uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xEE};
  #pragma message "compilation for: ESPnow_esp32030_gw2"

#elif DEVICE_ID == 27
// S2 is OK
  #define BOARD_TYPE                2
  #define HOSTNAME                  "esp32027"
  #define IP_ADDRESS                "192.168.1.27"
  #define ROLE_NAME                 "gw3"
  #define SENSORS_LED_GPIO_BLUE     2
  #define STATUS_GW_LED_GPIO_RED    3
  #define POWER_ON_LED_GPIO_GREEN   5
  #define POWER_ON_LED_USE_PWM      1
  #define POWER_ON_LED_DC           20
  // uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0x0b, 0xd9, 0xFE};
  uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0xAA, 0x00, 0x11};
  #pragma message "compilation for: ESPnow_esp32027_gw3"

#elif DEVICE_ID == 55
// S is OK
  #define BOARD_TYPE                1
  #define HOSTNAME                  "esp32055"
  #define IP_ADDRESS                "192.168.1.55"
  #define ROLE_NAME                 "gw4S"
  #define SENSORS_LED_GPIO_BLUE     25
  #define STATUS_GW_LED_GPIO_RED    26
  #define POWER_ON_LED_GPIO_GREEN   27
  #define POWER_ON_LED_USE_PWM      1
  #define POWER_ON_LED_DC           20
  uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0xAA, 0x00, 0x12};
  #pragma message "compilation for: ESPnow_esp32055_gw4_S"

#elif DEVICE_ID == 56
// ESP32-S3 is disconnecting from WiFi
  #define BOARD_TYPE                3
  #define HOSTNAME                  "esp32056"
  #define IP_ADDRESS                "192.168.1.56"
  #define ROLE_NAME                 "gw5S3"
  #define SENSORS_LED_GPIO_BLUE     7
  #define STATUS_GW_LED_GPIO_RED    5
  #define POWER_ON_LED_GPIO_GREEN   6
  #define POWER_ON_LED_USE_PWM      1
  #define POWER_ON_LED_DC           20
  uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0xAA, 0x00, 0x12};
  #pragma message "compilation for: ESPnow_esp32056_gw5_S3"

#elif DEVICE_ID == 57
// ESP32-C3 is disconnecting from WiFi
  #define BOARD_TYPE                4
  #define HOSTNAME                  "esp32057"
  #define IP_ADDRESS                "192.168.1.57"
  #define ROLE_NAME                 "gw6C3"
  #define SENSORS_LED_GPIO_BLUE     8
  #define STATUS_GW_LED_GPIO_RED    9
  #define POWER_ON_LED_GPIO_GREEN   10
  #define POWER_ON_LED_USE_PWM      1
  #define POWER_ON_LED_DC           20
  uint8_t FixedMACAddress[] = {0x7c, 0xdF, 0xa1, 0xAA, 0x00, 0x12};
  #pragma message "compilation for: ESPnow_esp32057_gw6_C3"

#else
  #error "Wrong DEVICE_ID chosen"
#endif
// DEVICE CUSTOM SETTINGS END

// for PubSubClient:
// it is used in: mqttc.setBufferSize(MQTT_PAYLOAD_MAX_SIZE); to increase max packet size
//default is 256 but it is not enough in this program - check debug in case of issue
#define MQTT_PAYLOAD_MAX_SIZE       1024

// for ArduinoJson
#define JSON_CONFIG_SIZE            1024  // config is bigger than payload due to device information
#define JSON_PAYLOAD_SIZE           512

// mqtt - how many attempts to connect to MQTT broker before restarting
#define MAX_MQTT_ERROR              15  // in seconds / times

// queue size for incomming data from sensors, whe queue is full no more data is gathered until there is free space in the queue
// set it to as much as many sensor devices you have so each one gets into the queue at least once. Unless some are very frequent (i.e. motion)
#define MAX_QUEUE_COUNT             15  //15

// how often to update HA on GW status
#define HEARTBEAT_INTERVAL_S        10    //10 in seconds

// LED PWM settings
#ifdef POWER_ON_LED_GPIO_GREEN
  #define POWER_ON_LED_PWM_CHANNEL    1
  #define POWER_ON_LED_PWM_RESOLUTION 8
  #define POWER_ON_LED_PWM_FREQ       5000
  #define POWER_ON_LED_MIN_DC         2
  #define POWER_ON_LED_MAX_DC         255
#endif

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
