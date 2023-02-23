#pragma once
/*
variables
*/
/* TODO:

*/
// struct types
typedef struct struct_message             // main data struct sent from sender to gateway over ESPnow, 92 bytes with motion_enabled
{
  char host[10];                          // esp32123   - 9 characters maximum (esp32123=8)
  char name[16];                          // 15 characters maximum
  char ver[10];                           // 123.56.89  - 9 characters maximum (123.56.89=9)
  char sender_type[10];                   // "env", "motion","env+mot", "push_b"
  char charg[5];                          // "FULL","ON","NC" - 4 characters maximum
  float temp;
  float hum;
  float lux;
  float bat;
  float batpct;
  float batchr;
  byte motion;                            // 0 - no motion, 1 - motion
  unsigned int boot;
  unsigned long ontime;                   // seconds, probably unsigned int would be enough - check it
  byte boardtype;
  uint8_t wifi_ok;                        // 0 - wifi not configured, 1 - wifi configured
  uint8_t motion_enabled;                 // 0 - motion disabled, 1 - motion enabled
  uint8_t light_high_sensitivity;         // 0 - low, light meas. time 13ms, 1 - high, light meas. time 403ms
  uint8_t button_pressed = 0;             // 0 = none, >0 = the button number as per button_gpio[NUMBER_OF_BUTTONS] - NOT GPIO NUMBER! index starts from 1

} struct_message;

typedef struct struct_message_aux         // auxiliary data structure for sensors - with RSSI and MAC (of sender)
{
  int rssi;
  char macStr[18];
} struct_message_aux;

typedef struct struct_message_recv        // command to sender - only 1 type but struct ready for expansion, the same type on sensor device!
{
  uint8_t command;
} struct_message_recv;

typedef struct struct_commands            // commands for the sender: mac of the sender and the command, used locally only on receiver
{
  char mac[18];
  uint8_t command;
} struct_commands;

// queues
QueueHandle_t queue;                      // queue for messages comming - buffer
QueueHandle_t queue_aux;                  // queue_aux for aux messages comming - buffer
QueueHandle_t queue_commands;             // queue for commands to be sent to senders- buffer

// semaphores handling the queues protection
SemaphoreHandle_t queue_protect = NULL;
SemaphoreHandle_t myLocalData_protect = NULL;
SemaphoreHandle_t queue_commands_protect = NULL;

//change_mac variables used also in make_fw_version() so must be global
char mac_org_char[18];
uint8_t mac_org[6];
char mac_new_char[18];
uint8_t mac_new[6];

// wifi
bool wifi_connected = false;
WiFiClient espClient;

// mqtt
bool mqtt_connected = false;
PubSubClient mqttc(espClient);

// struct variables:
struct_message myData;                    // data from sensors
struct_message_aux myData_aux;            // aux_data from/for sensors: rssi and MAC
struct_message myLocalData;               // local copy after receiving to avoid overwriting
struct_message_aux myLocalData_aux;       // local copy after receiving to avoid overwriting
struct_message_recv data_answer;          // commands for sender - struct with uint8_t only at this moment, the same type on sensor device!
struct_commands commands;                 // local only: mac and command stored in the queue

// // critical for OnDataRecv
// portMUX_TYPE receive_cb_mutex    = portMUX_INITIALIZER_UNLOCKED;

// firmware update
HTTPClient firmware_update_client;
int fw_totalLength = 0;
int fw_currentLength = 0;
bool perform_update_firmware=false;
int update_progress=0;
int old_update_progress=0;
bool blink_led_status=false;

#ifdef OTA_ACTIVE
  char ota_user[32];
  char ota_password[64];
#endif

// global others
bool debug_mode = false;  // change to true to see tones of messages
// unsigned long aux_update_interval = 0;
bool publish_sensors_to_ha = true;
unsigned long tt, program_start_time;
unsigned long aux_heartbeat_interval = 0;

// MQTT
String mqtt_server_str;
String mqtt_user_str;
String mqtt_pass_str;
String mqtt_port_str;

bool valid_mqtt_server_received = false;
bool valid_mqtt_user_received = false;
bool valid_mqtt_password_received = false;
bool valid_mqtt_port_received = false;

bool skip_mqtt = false;
bool mqtt_published_to_ha = false;
// global others END


#ifdef PUSH_BUTTON_GPIO
  #include <SimpleButton.h>
  using namespace simplebutton;
  Button* b = NULL;
  unsigned long aux_pushbutton_interval;
  unsigned long press_time, released;
  #define PUSHBUTTON_UPDATE_INTERVAL_MS     100    // in ms
#endif



uint8_t temp_address[6];
esp_now_peer_info_t peerInfo;
unsigned long last_cmd_received;               // to clean commands from the queue if timeout expired

// LED PWM
uint8_t led_gpio_sensors_dc, led_gpio_gateway_dc, led_gpio_standby_dc;
uint8_t prev_led_gpio_sensors_dc, prev_led_gpio_gateway_dc, prev_led_gpio_standby_dc;
bool led_sensors_power, led_gateway_power, led_standby_power;

// CO2
int last_indoor_co2 = 0;
bool co2_received = false;
bool calibration_in_progress = false;
bool calibration_finished = false;
long last_co2_update_time = 0;
char co2[7] = "NaN";

// timers
TimerHandle_t measure_co2_timer_handle  = NULL;
TimerHandle_t calibrate_mhz19_timer_handle  = NULL;

// serial2web
bool print2web = true;