#pragma once
/*

*/

// =======================================================================================================

// *************** "esp32026" - S2, hall -  ****************************
#if DEVICE_ID == 26
  #define SENSOR_TYPE                 "motion"  // "motion" "env"
  #define HOSTNAME                    "esp32026"
  #define DEVICE_NAME                 "hall"
  #define BOARD_TYPE                  2
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             9   // comment out if not in use - don't use "0" here
  #define MEASURE_VOLTS_ADC           1   // use "0" to disable
  #define VOLTS_ADC_GPIO              4   // comment out if not in use - don't use "0" here
  #define ADC_CALLIBRATION_FACTOR     (2 / 1.2128) // resistor divider and ESP board specific calibration
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define HEARTBEAT_SLEEP_DURATION_S  300 // seconds - 300
  #pragma message "compilation for: esp32026-hall"

  // *************** "esp32027" - S2, balcony -  ****************************
#elif DEVICE_ID == 27
  #define SENSOR_TYPE                 "motion"  // "motion" "env"
  #define HOSTNAME                    "esp32027"
  #define DEVICE_NAME                 "balcony"
  #define BOARD_TYPE                  2
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             9   // comment out if not in use - don't use "0" here
  #define MEASURE_VOLTS_ADC           1   // use "0" to disable
  #define VOLTS_ADC_GPIO              4   // comment out if not in use - don't use "0" here
  #define ADC_CALLIBRATION_FACTOR     (2 / 1.2175) // resistor divider and ESP board specific calibration
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define HEARTBEAT_SLEEP_DURATION_S  300 // seconds - 300
  #pragma message "compilation for: esp32027-balcony"

// *************** "esp32033" - S2, garage street -  ****************************
#elif DEVICE_ID == 33
  #define SENSOR_TYPE                 "motion"  // "motion" "env"
  #define HOSTNAME                    "esp32033"
  #define DEVICE_NAME                 "garage-street"
  #define BOARD_TYPE                  2
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             7   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for MAX17048 on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define MEASURE_VOLTS_ADC           0   // use "0" to disable
  #define VOLTS_ADC_GPIO              4   // comment out if not in use - don't use "0" here
  #define ADC_CALLIBRATION_FACTOR     (2 / 1.174) // resistor divider and ESP board specific calibration
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define HEARTBEAT_SLEEP_DURATION_S  300 // seconds - 300
  #pragma message "compilation for: esp32033-garage-street"
// ---------------------------------------------------------------------------------------------------

// *************** "esp32035" - S2, new device - garage-garden-  ****************************
#elif DEVICE_ID == 35
  #define SENSOR_TYPE                 "motion"  // "motion" "env"
  #define HOSTNAME                    "esp32035"
  #define DEVICE_NAME                 "garage-garden"  // on new board, with MAX17048
  #define BOARD_TYPE                  2
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for MAX17048 on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define MEASURE_VOLTS_ADC           0   // use "0" to disable
  // #define VOLTS_ADC_GPIO              4   // comment out if not in use - don't use "0" here
  // #define ADC_CALLIBRATION_FACTOR     (2 / 1.174) // resistor divider and ESP board specific calibration
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define HEARTBEAT_SLEEP_DURATION_S  300 // seconds - 300
  #pragma message "compilation for: esp32085-garage-garden"
// ---------------------------------------------------------------------------------------------------

// *************** "esp32036" - S2, new device - garage-garden-  ****************************
#elif DEVICE_ID == 36
  #define SENSOR_TYPE                 "motion"  // "motion" "env"
  #define HOSTNAME                    "esp32036"
  #define DEVICE_NAME                 "kitchen"  // on new board, with MAX17048
  #define BOARD_TYPE                  2
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for MAX17048 on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define MEASURE_VOLTS_ADC           0   // use "0" to disable
  // #define VOLTS_ADC_GPIO              4   // comment out if not in use - don't use "0" here
  // #define ADC_CALLIBRATION_FACTOR     (2 / 1.174) // resistor divider and ESP board specific calibration
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define HEARTBEAT_SLEEP_DURATION_S  300 // seconds - 300
  #pragma message "compilation for: esp32086-garage-garden"
// ---------------------------------------------------------------------------------------------------

// *************** "esp32086" - S2, new device -test -  ****************************
#elif DEVICE_ID == 86
  #define SENSOR_TYPE                 "motion"  // "motion" "env"
  #define HOSTNAME                    "esp32086"
  #define DEVICE_NAME                 "new-dev-test86"  // on new board, with MAX17048
  #define BOARD_TYPE                  2
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for MAX17048 on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define MEASURE_VOLTS_ADC           0   // use "0" to disable
  // #define VOLTS_ADC_GPIO              4   // comment out if not in use - don't use "0" here
  // #define ADC_CALLIBRATION_FACTOR     (2 / 1.174) // resistor divider and ESP board specific calibration
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    5  // seconds - 30
  #define HEARTBEAT_SLEEP_DURATION_S  10 // seconds - 300
  #pragma message "compilation for: esp32086-nevdev-86-test"
// ---------------------------------------------------------------------------------------------------


#else
  #error "Wrong DEVICE_ID chosen"
#endif


// assigning MODEL and checking if proper board is selected
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
    #define ESP32_IS_CHEATING (325) // +310 head, +12 tail, +3ms delay between saved and save_ontime() function start
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32"
    #endif
  #elif (BOARD_TYPE == 2)
    #define MODEL "ESP32S2"
    #define ESP32_IS_CHEATING (-228)  // -244 head, +12 tail, +4ms delay between saved and save_ontime() function start
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32S2"
    #endif
  #elif (BOARD_TYPE == 3)
    #define MODEL "ESP32S3"
    #define ESP32_IS_CHEATING 0       // not tested yet
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32S3"
    #endif
  #elif (BOARD_TYPE == 4)
    #define MODEL "ESP32C3"
    #define ESP32_IS_CHEATING 0       // not tested yet
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32C3"
    #endif
  #else
    #error BOARD_TYPE not defined
  #endif
#endif
// assigning MODEL and checking if proper board is selected END
