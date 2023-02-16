#pragma once
/*

*/

// =======================================================================================================
#if DEVICE_ID == 26
// *************** "esp32026" - C3, TRV4 Printers -  ****************************
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32026"
  #define DEVICE_NAME                 "TRV4 Printers"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  // #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  // #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  // #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  // #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  // #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  // #define ENABLE_3V_GPIO              4   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  // #define USE_SHT31                   1   // use "0" to disable
  // #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  // #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                6 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32026-TRV4-Printers"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 28
// *************** "esp32028" - S2, Garage -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32028"
  #define DEVICE_NAME                 "Garage"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32028-Garage"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 31
// *************** "esp32086" - S2, test -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32031"
  #define DEVICE_NAME                 "Printers room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32031-Printers room"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 32
// *************** "esp32032" - S2, bedroom motion -  ****************************
  #define SENSOR_TYPE                 1 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32032"
  #define DEVICE_NAME                 "Bedroom mot."  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  // #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  // #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          2   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32032-bedroom motion"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 33
// *************** "esp32033" - S2, Living room -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32033"
  #define DEVICE_NAME                 "Living room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32033-Living room"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 35
// *************** "esp32035" - S2, Tailor room -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32035"
  #define DEVICE_NAME                 "Tailor room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32035-Tailor"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 36
// *************** "esp32036" - S2, Lidia room -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32036"
  #define DEVICE_NAME                 "Lidia room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                120 // seconds - 180

  #pragma message "compilation for: esp32036-Lidia"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 38
// *************** "esp320038" - S2, toilet bedroom  -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32038"
  #define DEVICE_NAME                 "Toilet bedroom"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32038-Toilet bedroom"
// ---------------------------------------------------------------------------------------------------
//
// #elif DEVICE_ID == 47
// // *************** "esp32091" - C3,UPS_1 -  ****************************
//   #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
//   #define HOSTNAME                    "esp32047"
//   #define DEVICE_NAME                 "upsbat1"  // 15 characters maximum
//   #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
//   #define CHARGING_GPIO               1  // comment out if not in use - don't use "0" here
//   #define POWER_GPIO                  2  // comment out if not in use - don't use "0" here
//   // #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
//   #define ACT_BLUE_LED_GPIO           4   // comment out if not in use - don't use "0" here
//   #define ERROR_RED_LED_GPIO          3   // comment out if not in use - don't use "0" here
//   #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
//   // #define ENABLE_3V_GPIO              4   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
//   #define USE_MAX17048                1   // use "0" to disable
//   // #define USE_SHT31                   1   // use "0" to disable
//   // #define USE_TSL2561                 1   // use "0" to disable
//   #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
//   // #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
//   #define SLEEP_TIME_S                3 // seconds
//
//   #pragma message "compilation for: esp32047-upsbat1"
// // ---------------------------------------------------------------------------------------------------
//
// #elif DEVICE_ID == 48
// // *************** "esp32091" - C3,UPS_2 -  ****************************
//   #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
//   #define HOSTNAME                    "esp32048"
//   #define DEVICE_NAME                 "upsbat2"  // 15 characters maximum
//   #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
//   #define CHARGING_GPIO               1  // comment out if not in use - don't use "0" here
//   #define POWER_GPIO                  2  // comment out if not in use - don't use "0" here
//   // #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
//   #define ACT_BLUE_LED_GPIO           4   // comment out if not in use - don't use "0" here
//   #define ERROR_RED_LED_GPIO          3   // comment out if not in use - don't use "0" here
//   #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
//   // #define ENABLE_3V_GPIO              4   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
//   #define USE_MAX17048                1   // use "0" to disable
//   // #define USE_SHT31                   1   // use "0" to disable
//   // #define USE_TSL2561                 1   // use "0" to disable
//   #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
//   // #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
//   #define SLEEP_TIME_S                3 // seconds
//
//   #pragma message "compilation for: esp32048-upsbat2"
// // ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 100
// *************** "esp32100" - S2, Office -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32100"
  #define DEVICE_NAME                 "Office"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32100-Office"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 101
// *************** "esp32101" - S2, dining room -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32101"
  #define DEVICE_NAME                 "Dining room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32101-Dining"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 102
// *************** "esp32102" - S2, Toiletup  -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32102"
  #define DEVICE_NAME                 "Toilet up"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32102-ToiletUp"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 104
// *************** "esp32102" - S2, Milena -  ****************************
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32104"
  #define DEVICE_NAME                 "Kids room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32104-Milena"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 105
// *************** "esp32105" - S2, Garden -  ****************************
  #define SENSOR_TYPE                 0 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32105"
  #define DEVICE_NAME                 "Garden"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  // #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32105-Garden"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 92
// *************** "esp32092" - S2, test -  ****************************
  #define SENSOR_TYPE                 0 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32092"
  #define DEVICE_NAME                 "92test"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  // #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  // #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
//  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                0   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                3 // seconds - 180

  #pragma message "compilation for: esp32092-test"
// ---------------------------------------------------------------------------------------------------

// =======================================================================================================
#elif DEVICE_ID == 93
// *************** "esp32093" - C3, test -  ****************************
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32093"
  #define DEVICE_NAME                 "c3tests"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  // #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  // #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  // #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  // #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  // #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  // #define ENABLE_3V_GPIO              4   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                0   // use "0" to disable
  // #define USE_SHT31                   1   // use "0" to disable
  // #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               2.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    3  // seconds - 30
  #define SLEEP_TIME_S                10 // seconds - 1 hour

  #pragma message "compilation for: " HOSTNAME DEVICE_NAME
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 94
// *************** "esp32094" - S, test -  ****************************
  #define SENSOR_TYPE                 0 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32094"
  #define DEVICE_NAME                 "94testS"  // 15 characters maximum
  #define BOARD_TYPE                  1   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  // #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here
  // #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here
  // #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  // #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here
  // #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             36   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  // #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                0   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  #define COOLING_SLEEP_DURATION_S    3  // seconds - 30
  #define SLEEP_TIME_S                10 // seconds - 180

  #pragma message "compilation for: esp32094-test"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 95
// *************** "esp32095" - C3,UPS_2 -  ****************************
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32095"
  #define DEVICE_NAME                 "test"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define CHARGING_GPIO               1  // comment out if not in use - don't use "0" here
  #define POWER_GPIO                  2  // comment out if not in use - don't use "0" here
  // #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here
  #define ACT_BLUE_LED_GPIO           4   // comment out if not in use - don't use "0" here
  #define ERROR_RED_LED_GPIO          3   // comment out if not in use - don't use "0" here
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here - cannot be 8 or 9 on new boards if I2C used
  // #define ENABLE_3V_GPIO              4   // comment out if not in use - don't use "0" here - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  // #define USE_SHT31                   1   // use "0" to disable
  // #define USE_TSL2561                 1   // use "0" to disable
  #define MINIMUM_VOLTS               3.3 // device is going to forced 1 hour sleep to conserve battery
  // #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
  #define SLEEP_TIME_S                3 // seconds

  #pragma message "compilation for: esp32095-test"
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
    #define ESP32_IS_CHEATING 20       // tested but without GPIO - just using PPK2
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32C3"
    #endif
  #else
    #error BOARD_TYPE not defined
  #endif
#endif
// assigning MODEL and checking if proper board is selected END

// SENSOR_TYPE defines what is to be sent to GW and what should wake up sensor device
#if (SENSOR_TYPE == 0)
  #ifdef MOTION_SENSOR_GPIO
    #warning "SENSOR_TYPE=0 (env) should not have MOTION_SENSOR_GPIO defined - removing it"
    #undef MOTION_SENSOR_GPIO
  #endif
#endif

// don't allow compiling if not provided MOTION_SENSOR_GPIO
#if (SENSOR_TYPE == 1) or (SENSOR_TYPE == 2)
  #ifndef MOTION_SENSOR_GPIO
    #error "MOTION_SENSOR_GPIO not defined"
  #endif
#endif

// don't allow compiling if not provided FW_UPGRADE_GPIO
#ifndef FW_UPGRADE_GPIO
  #error "FW_UPGRADE_GPIO not defined"
#endif
