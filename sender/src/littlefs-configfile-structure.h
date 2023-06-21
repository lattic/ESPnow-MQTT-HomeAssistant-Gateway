#ifndef littlefs_configfile_structure_h
#define littlefs_configfile_structure_h

struct Config 
{
//Internal config variables: all with "c_" to distinguish from program variables
  uint16_t  c_sleeptime_s;            // configurable
  uint32_t  c_bootCount;              // bootCount
  uint32_t  c_saved_ontime_ms;        // ontime in ms
  uint8_t   c_channel;                // WiFi channel
  uint8_t   c_wifi_ok = 0;            // WiFi successful data provided
  uint8_t   c_last_gw = 0;            // last used gateway
  char      c_ssid[33];               // WiFi ssid
  char      c_password[65];           // WiFi password
  uint8_t   c_led_pwm;                // DC for LED PWM
  uint8_t   c_motion = 1;             // motion enabled - toggle
  uint8_t   c_lux_high_sens = 0;      // lux high sensitivity - toggle
  uint16_t  c_last_working_time_ms;   // to see how long it worked for
  uint8_t   c_valid = 1;              // valid measurerements
};

// Global variables: all with "g_" to distinguish from config variables
uint16_t    g_sleeptime_s;            // configurable
uint32_t    g_bootCount;              // bootCount
uint32_t    g_saved_ontime_ms;        // ontime in ms
uint8_t     g_wifi_channel;           // WiFi channel
uint8_t     g_wifi_ok = 0;            // WiFi successful data provided
uint8_t     g_last_gw = 0;            // last used gateway
char        g_wifi_ssid[33];          // WiFi ssid
char        g_wifi_password[65];      // WiFi password
uint8_t     g_led_pwm;                // DC for LED PWM
uint8_t     g_motion = 1;             // motion enabled - toggle
uint8_t     g_lux_high_sens = 0;      // lux high sensitivity - toggle
uint16_t    g_last_working_time_ms;   // to see how long it worked for
uint8_t     g_valid = 1;              // valid measurerements

#endif /* #ifndef littlefs_configfile_structure_h */