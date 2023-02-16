#ifndef devices_config_h
#define devices_config_h
/**/


// ********************* choose device - ONLY ONE! *****************************
// new, multipurpose device
  // #define DEVICE_ID  21           // "esp32021" - C3, production - TRV5 Lidia
  // #define DEVICE_ID  22           // "esp32022" - C3, production - TRV6 Kids
  // #define DEVICE_ID  23           // "esp32023" - C3, production - TRV2 Toilet bedroom
  // #define DEVICE_ID  26           // "esp32026" - C3, production - TRV4 Printers
  // #define DEVICE_ID  28           // "esp32028" - S2, production - Garage
  // #define DEVICE_ID  31           // "esp32031" - S2, production - Printer room
  // #define DEVICE_ID  32           // "esp32032" - S2, production - Bedroom mot.
  // #define DEVICE_ID  33           // "esp32033" - S2, production - Living room
  // #define DEVICE_ID  35           // "esp32035" - S2, production - Tailor room
  // #define DEVICE_ID  36           // "esp32036" - S2, production - Lidia room
  // #define DEVICE_ID  37           // "esp32037" - C3, production - TRV1 Bedroom
  // #define DEVICE_ID  38           // "esp32038" - S2, production - Toilet bedroom
  // #define DEVICE_ID  39           // "esp32039" - S2, production - garage front mot.
  // #define DEVICE_ID  49           // "esp32049" - S2, production - kitchen
  // #define DEVICE_ID  55           // "esp32055" - C3, production - TRV3 Toilet Bedroom
  // #define DEVICE_ID  56           // "esp32056" - C3, production - TRV7 Tailor
  // #define DEVICE_ID  100          // "esp32100" - S2, production - Office
  // #define DEVICE_ID  101          // "esp32101" - S2, production - Dining room
  // #define DEVICE_ID  102          // "esp32102" - S2, production - Toilet up
  // #define DEVICE_ID  104          // "esp32104" - S2, production - Kids room
  // #define DEVICE_ID  105          // "esp32105" - S2, production - Garden
// new, multipurpose device END

// test devices
// none
// test devices END
// ********************* choosing device in platformion.ini *****************************




// ***************************************  BOARDS  CONFIGURATION  ***************************************

#if DEVICE_ID == 21
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32021"
  #define DEVICE_NAME                 "TRV5 Lidia"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                1 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32021-TRV5-Lidia"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 22
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32022"
  #define DEVICE_NAME                 "TRV6 Kids"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                1 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32022-TRV6-Kids"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 23
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32023"
  #define DEVICE_NAME                 "TRV2 Toilet b."  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                1 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32023-TRV2-Toilet-Bedroom"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 26
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32026"
  #define DEVICE_NAME                 "TRV4 Printers"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                1 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32026-TRV4-Printers"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 28
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32028"
  #define DEVICE_NAME                 "Garage"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32028-Garage"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 31
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32031"
  #define DEVICE_NAME                 "Printers room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32031-Printers room"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 32
  #define SENSOR_TYPE                 1 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32032"
  #define DEVICE_NAME                 "Bedroom mot."  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define MOTION_SENSOR_GPIO          2   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32032-bedroom motion"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 33
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32033"
  #define DEVICE_NAME                 "Living room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32033-Living room"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 35
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32035"
  #define DEVICE_NAME                 "Tailor room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable

  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32035-Tailor"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 36
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32036"
  #define DEVICE_NAME                 "Lidia room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                120 // seconds - 180

  #pragma message "compilation for: esp32036-Lidia"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 37
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32037"
  #define DEVICE_NAME                 "TRV1 Bedroom"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                1 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32037-TRV1-Bedroom"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 38
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32038"
  #define DEVICE_NAME                 "Toilet bedroom"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32038-Toilet bedroom"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 39
  #define SENSOR_TYPE                 1 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32039"
  #define DEVICE_NAME                 "Garage front m."  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                300 // seconds - 180

  #pragma message "compilation for: esp32039-Garage front m."
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 49
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32049"
  #define DEVICE_NAME                 "Kitchen"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32049-Kitchen"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 55
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32055"
  #define DEVICE_NAME                 "TRV3 Toilet up"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                1 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32055-TRV3-toilet-up"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 56
  #define SENSOR_TYPE                 3 // 0 = "env", 1 = "motion", 2 =  "env+mot", 3 = "battery"
  #define HOSTNAME                    "esp32056"
  #define DEVICE_NAME                 "TRV7 Tailor"  // 15 characters maximum
  #define BOARD_TYPE                  4   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3, 4 = ESP32-C3
  #define FW_UPGRADE_GPIO             0   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                1 * 3600 // seconds - 6 hours

  #pragma message "compilation for: esp32056-TRV7-tailor"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 100
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32100"
  #define DEVICE_NAME                 "Office"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32100-Office"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 101
  // measured current: 38.45uA without PIR (switch OFF)
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32101"
  #define DEVICE_NAME                 "Dining room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32101-Dining"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 102
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32102"
  #define DEVICE_NAME                 "Toilet up"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32102-ToiletUp"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 104
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32104"
  #define DEVICE_NAME                 "Kids room"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             2   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #pragma message "compilation for: esp32104-Milena"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 105
  #define SENSOR_TYPE                 0 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "esp32105"
  #define DEVICE_NAME                 "Garden"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       0   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          -5   // fine tuning, positive if sensor shows too low, negative if sensor shows too high


  #pragma message "compilation for: esp32105-Garden"
// ---------------------------------------------------------------------------------------------------


// ************************************* C L I E N T S ***********************************************

#elif DEVICE_ID == 1
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz001"
  #define DEVICE_NAME                 "salon"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       0 //0.15   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          0 //-0.6   // fine tuning, positive if sensor shows too low, negative if sensor shows too high

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz001"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 2
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz002"
  #define DEVICE_NAME                 "sypialnia dol"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       0 //-1.17   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          -2.7 //-0.9   // fine tuning, positive if sensor shows too low, negative if sensor shows too high

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz002"
// ---------------------------------------------------------------------------------------------------

#elif DEVICE_ID == 3
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz003"
  #define DEVICE_NAME                 "gora Igor"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       0 //0.23   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          -2.7 //-3.4   // fine tuning, positive if sensor shows too low, negative if sensor shows too high

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz003"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 4
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz004"
  #define DEVICE_NAME                 "gora Kajtek"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       0   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          0   // fine tuning, positive if sensor shows too low, negative if sensor shows too high

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz004"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 5
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz005"
  #define DEVICE_NAME                 "lazienka"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       0.08   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          -9.5   // fine tuning, positive if sensor shows too low, negative if sensor shows too high

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz005"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 6
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 2 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz006"
  #define DEVICE_NAME                 "kuchnia"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       -0.2 //-0.33   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          0 //0.3   // fine tuning, positive if sensor shows too low, negative if sensor shows too high

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };


#elif DEVICE_ID == 7
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 0 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz007"
  #define DEVICE_NAME                 "balkon"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               39  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  38  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define ENABLE_3V_GPIO              3   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - mandatory for I2C devices on new boards
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   1   // use "0" to disable
  #define USE_TSL2561                 1   // use "0" to disable
  #define SLEEP_TIME_S                180 // seconds - 180

  #define CALIBRATE_TEMPERATURE       0   // fine tuning, positive if sensor shows too low, negative if sensor shows too high
  #define CALIBRATE_HUMIDITY          -7.0   // fine tuning, positive if sensor shows too low, negative if sensor shows too high

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz007"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 8
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 1 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz008"
  #define DEVICE_NAME                 "schody"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               3  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  2  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                900 // seconds - 180

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz008"
// ---------------------------------------------------------------------------------------------------


#elif DEVICE_ID == 9
  #define CLIENT                      "003-krzych"
  #define SENSOR_TYPE                 1 // 0 = "env", 1 = "motion", 2 =  "env+mot"
  #define HOSTNAME                    "krz009"
  #define DEVICE_NAME                 "przedpokoj"  // 15 characters maximum
  #define BOARD_TYPE                  2   // 1 = ESP32-S, 2 = ESP32-S2, 3 = ESP32-S3
  #define CHARGING_GPIO               3  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define POWER_GPIO                  2  // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define MOTION_SENSOR_GPIO          1   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ACT_BLUE_LED_GPIO           6   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define ERROR_RED_LED_GPIO          5   // comment out if not in use - don't use "0" here unless you mean GPIO=0
  #define FW_UPGRADE_GPIO             4   // comment out if not in use - don't use "0" here unless you mean GPIO=0 - cannot be 8 or 9 on new boards if I2C used
  #define USE_MAX17048                1   // use "0" to disable
  #define USE_SHT31                   0   // use "0" to disable
  #define USE_TSL2561                 0   // use "0" to disable
  #define SLEEP_TIME_S                900 // seconds - 180

  // #define NUMBER_OF_GATEWAYS          2               // number of receivers/gateways
  // uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  // {
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01,               // krz101-gateway1
  //   0x3A, 0xFF, 0x01, 0x01, 0x01, 0x03                // krz103-gateway3
  // };

  #define NUMBER_OF_GATEWAYS          1               // number of receivers/gateways
  uint8_t receivers[NUMBER_OF_GATEWAYS][6] =          // put here the fake MAC addresses that are assigned in receiver sketch
  {
    0x3A, 0xFF, 0x01, 0x01, 0x01, 0x01               // krz101-gateway1
  };

  #pragma message "compilation for: krz009"
// ---------------------------------------------------------------------------------------------------


#else
  #error "Wrong DEVICE_ID chosen"
#endif

// common parameters
#ifdef ACT_BLUE_LED_GPIO
  // check if PWM is defined, i.e. = 0
  #ifndef ACT_BLUE_LED_GPIO_USE_PWM
    #define ACT_BLUE_LED_GPIO_USE_PWM   1
  #endif 
#endif

#ifdef ERROR_RED_LED_GPIO
  // check if PWM is defined, i.e. = 0
  #ifndef ERROR_RED_LED_GPIO_USE_PWM
    #define ERROR_RED_LED_GPIO_USE_PWM  1
  #endif
#endif

#ifndef MINIMUM_VOLTS
   #define MINIMUM_VOLTS               3.3 // device is going to forced sleep to conserve battery
#endif 

#ifndef COOLING_SLEEP_DURATION_S
    #define COOLING_SLEEP_DURATION_S    30  // seconds - 30
#endif
// common parameters END

// assigning MODEL and checking if proper board is selected
#ifndef BOARD_TYPE
  #error BOARD_TYPE not defined
#else
  #if (BOARD_TYPE == 1)
    #define MODEL "ESP32"
    #define ESP32_BOOT_TIME           350      // since power ON to GPIO ON, ms
    #define ESP32_TAIL_TIME           15      // since GPIO OFF to power OFF, ms
    #define ESP32_BOOT_TIME_EXTRA     20     // +180ms from power ON or hard reset    #ifdef PRINT_COMPILER_MESSAGES
    #ifdef PRINT_COMPILER_MESSAGES
      #pragma message "chosen BOARD_TYPE = ESP32"
    #endif
  #elif (BOARD_TYPE == 2)
    #define MODEL "ESP32S2"
    #define ESP32_BOOT_TIME           35      // since power ON to GPIO ON, ms
    #define ESP32_TAIL_TIME           25      // since GPIO OFF to power OFF, ms
    #define ESP32_BOOT_TIME_EXTRA     180     // +180ms from power ON or hard reset
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
    #define ESP32_BOOT_TIME           30      // since power ON to GPIO ON, ms
    #define ESP32_TAIL_TIME           10      // since GPIO OFF to power OFF, ms
    #define ESP32_BOOT_TIME_EXTRA     155     // +150ms from power ON or hard reset
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

#endif  /* devices_config_h */

