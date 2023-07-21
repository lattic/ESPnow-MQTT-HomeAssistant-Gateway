#ifndef common_config_h
#define common_config_h

// #define DEBUG
// #define DEBUG_LIGHT


// ---------------------------

#ifdef DEBUG
  #ifndef DEBUG_LIGHT
    #define DEBUG_LIGHT
  #endif
#endif 

#define ZH_PROG_VERSION "3.2.b1"                     // version < 10 chars, description in changelog.txt


#pragma message "Compiling ZH_PROG_VERSION = " ZH_PROG_VERSION

#define FORMAT_FS                   0               // force format of file system - used only on first deployment - FS is also formatted on reset device fucntion
// #define PPK2_GPIO                   33              // comment out if not used - GPIO to test power and timings using PPK2 - for C3 GPIO 33 is out of range
#define USE_FAKE_RECEIVER           0               // 1=use this to avoid flooding receiver/HA, any other -> proper receivers - NOT WORKING IN VERSION 2.x
#define MAC_BROADCAST               1               // 1=broadcast, any other for unicast - irrelevant if USE_FAKE_RECEIVER = 1 - NOT WORKING IN VERSION 2.x
#define PRINT_COMPILER_MESSAGES                     // prints extra messages to see which board is being compiled
#define SLEEP_TIME_H_BATTERY_EMPTY  24              // sleep hours when battery empty
#define WAIT_FOR_WIFI               5               // in seconds
#define MAX17048_DELAY_ON_RESET_MS  200             // as per datasheet: needed before next reading from MAX17048 after reset, only in use when reset/battery change
#define CP_TIMEOUT_S                180             // Captive Portal will terminate after this time
#define OTA_WEB_SERVER_TIMEOUT_S    180             // Captive Portal will terminate after this time
#define OTA_WEB_SERVER_PORT         8080            // PORT
#define uS_TO_S_FACTOR              1000000ULL      // auxuliary 

#define DEBOUNCE_MS_ANY_GPIO        5 // 200        // wait time after pressing ANY GPIO
#define DEBOUNCE_MS_FW_GPIO         300  // 200     // used for FW GPIO only
#define DEBOUNCE_MS_PUSHBUTTON      100             // used for PUSH BUTTON GPIO only

// charging constants - maximum 4 characters
#define CHARGING_NC                 "NC"
#define CHARGING_ON                 "ON"
#define CHARGING_FULL               "FULL"
#define CHARGING_OFF                "OFF"
#define JSON_CONFIG_FILE_SIZE       1024            // for dealing with config file on LittleFS
#define ENCRYPT_CONFIG                              // encrypting config file
#define WAIT_FOR_COMMAND_MS         100             // timeout for command received from gateway

// using fake MAC addresses for gateways makes easier to replace device when broken without recompiling sensors sketch
#ifndef CLIENT                                      // for my sensors, this is defined here, for CLIENT's in devices_config.h
  #define NUMBER_OF_GATEWAYS          3               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x2A, 0xFF, 0x01, 0x01, 0x01, 0x29,               // gw1
    0x2A, 0xFF, 0x01, 0x01, 0x01, 0x30,               // gw2
    0x2A, 0xFF, 0x01, 0x01, 0x01, 0x27                // gw3
  };
  #define NUMBER_OF_LORA_GATEWAYS     1               // number of receivers/gateways
  uint8_t lora_receivers[NUMBER_OF_LORA_GATEWAYS][6] =  // put here the fake MAC addresses of LoRa receviers that are assigned in receiver sketch
  {
    0x2A, 0xFF, 0x01, 0x01, 0x01, 0x45                // gw1
  };
  
#endif 


// template for new fake MAC - last 2 bytes replaced later in change_mac() by DEVICE_ID as if it was in hex
uint8_t FixedMACAddress[] =         {0x2a, 0x01, 0x01, 0x01, 0x00, 0x01};

#ifdef ACT_BLUE_LED_GPIO
  #if (ACT_BLUE_LED_GPIO_USE_PWM == 1) 
    #define ACT_BLUE_LED_PWM_CHANNEL     1
    #define ACT_BLUE_LED_PWM_FREQ        5000
    #define ACT_BLUE_LED_PWM_RESOLUTION  8
  #endif
#endif

#ifdef ERROR_RED_LED_GPIO
  #if (ERROR_RED_LED_GPIO_USE_PWM == 1) 
    #define ERROR_RED_LED_PWM_CHANNEL    2
    #define ERROR_RED_LED_PWM_FREQ       5000
    #define ERROR_RED_LED_PWM_RESOLUTION 8  
  #endif
#endif



#endif /* common_config_h */
