#define DEBUG

// #define DEVICE_ID  29  //hall, S2
#define DEVICE_ID  30  //dining, S2
// #define DEVICE_ID  27  //printers, S2

// test devices:
// #define DEVICE_ID  55  //test, S
// #define DEVICE_ID  56  //test, S3
// #define DEVICE_ID  57  //test, C3

/*
receiver.ino
*/
#define VERSION "1.22.1"


// libraries
#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <PubSubClient.h>   // MQTT client
#include <ArduinoJson.h>
// Firmware update
#include <HTTPClient.h>
#include <Update.h>

// fuctions declarations
// wifi.h
void setup_wifi(void*z);

// espnow.h
void espnow_start();
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

// mqtt.h
void mqtt_reconnect();
void mqtt_callback(char* topic, byte* message, unsigned int length);
bool mqtt_publish_button_update_config();

// mqtt_publish_gw_data.h - UPDATE_INTERVAL
bool mqtt_publish_gw_status_config();
bool mqtt_publish_gw_status_values(const char* status);
void heartbeat();

// on message arrival from sensors
bool mqtt_publish_gw_last_updated_sensor_config();
bool mqtt_publish_gw_last_updated_sensor_values(const char* status);

// mqtt_publish_sensors_data.h - all at once
bool mqtt_publish_sensors_config(const char* hostname, const char* name, const char* mac, const char* fw, const char* dev_type);
bool mqtt_publish_sensors_values();
bool mqtt_publish_button_restart_config();
bool mqtt_publish_button_update_config();
bool mqtt_publish_switch_publish_config();
bool mqtt_publish_switch_publish_values();

// firmware update
void do_update();
void updateFirmware(uint8_t *data, size_t len);
int update_firmware_prepare();

// various other here in various.h
void ConvertSectoDay(unsigned long n, char * pretty_ontime);
void uptime(char *uptime);

// custom files
#include "config.h"
#include "variables.h"
#include "wifi.h"
#include "espnow.h"
#include "mqtt.h"
#include "mqtt_publish_gw_data.h"
#include "mqtt_publish_sensors_data.h"
#include "fw_update.h"
#include "various.h"

#ifdef OTA_ACTIVE
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <AsyncElegantOTA.h>
  AsyncWebServer server(80);
  // #warning "OTA_ACTIVE defined"
#else
  #warning "OTA_ACTIVE NOT defined"
#endif

void setup()
{
  bool debug_mode = false;
  long sm = 0;
  program_start_time = millis();

  #ifdef STATUS_GW_LED_GPIO_RED
    pinMode(STATUS_GW_LED_GPIO_RED, OUTPUT);
  #endif

  #ifdef SENSORS_LED_GPIO_BLUE
    pinMode(SENSORS_LED_GPIO_BLUE, OUTPUT);
  #endif

  #ifdef POWER_ON_LED_GPIO_GREEN
    // PWM
    #if (POWER_ON_LED_USE_PWM == 1)
      ledcSetup(POWER_ON_LED_PWM_CHANNEL, POWER_ON_LED_PWM_FREQ, POWER_ON_LED_PWM_RESOLUTION);
      ledcAttachPin(POWER_ON_LED_GPIO_GREEN, POWER_ON_LED_PWM_CHANNEL);
      // set brightness of GREEN LED (0-255)
      ledcWrite(POWER_ON_LED_PWM_CHANNEL, POWER_ON_LED_DC);
    // or fixed
    #else
      pinMode(POWER_ON_LED_GPIO_GREEN, OUTPUT);
      digitalWrite(POWER_ON_LED_GPIO_GREEN, HIGH);
    #endif
  #endif

  Serial.begin(115200);
  sm = millis(); while(millis() < sm + 100) {};

  Serial.printf("\n\n =============================================================\n");
  Serial.printf("[%s]: GATEWAY started, DEVICE_ID=%d, version=%s\n",__func__,DEVICE_ID,VERSION);

  queue_protect = xSemaphoreCreateMutex();
  if (queue_protect != NULL)
  {
    if (debug_mode) Serial.printf("[%s]: queue_protect semaphore created\n",__func__);
  }

  myLocalData_protect = xSemaphoreCreateMutex();
  if (myLocalData_protect != NULL)
  {
    if (debug_mode) Serial.printf("[%s]: myLocalData_protect semaphore created\n",__func__);
  }

  queue = xQueueCreate( MAX_QUEUE_COUNT, sizeof( struct struct_message ) );
  if(queue == NULL)
  {
    Serial.printf("[%s]: Error creating the queue, restarting in 3s\n",__func__);
    sm = millis(); while(millis() < sm + 3000) {};
    ESP.restart();
  }

  queue_aux = xQueueCreate( MAX_QUEUE_COUNT, sizeof( struct struct_message_aux ) );
  if(queue_aux == NULL)
  {
    Serial.printf("[%s]: Error creating the queue_aux, restarting in 3s\n",__func__);
    sm = millis(); while(millis() < sm + 3000) {};
    ESP.restart();
  }


  xReturned_setup_wifi = xTaskCreatePinnedToCore(setup_wifi, "setup_wifi", 5000, NULL, 1, &h_setup_wifi, CONFIG_ARDUINO_RUNNING_CORE);
  if( xReturned_setup_wifi != pdPASS )
  {
    Serial.printf("[%s]: setup_wifi task not created - restarting in 1s\n",__func__);
    sm = millis(); while(millis() < sm + 1000) {};
    ESP.restart();
  } else
  {
    if (debug_mode) Serial.printf("[%s]: setup_wifi task created\n",__func__);
  }

  while (!wifi_connected)
  {
    Serial.printf("[%s]: WIFI NOT connected\n",__func__);
    sm = millis(); while(millis() < sm + 1000) {};
  }
  if (debug_mode) Serial.printf("[%s]: WIFI  connected\n",__func__);

  //OTA in Setup
  #ifdef OTA_ACTIVE
    strlcpy(ota_user, OTA_USER, sizeof(ota_user));
    strlcpy(ota_password, OTA_PASSWORD, sizeof(ota_password));
    if (debug_mode) Serial.printf("[%s]: Enabling OTA:...\n",__func__);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      String introtxt = "This is gateway "+String(DEVICE_ID) +", Version: " + String(VERSION);
      request->send(200, "text/plain", String(introtxt));
    });
    AsyncElegantOTA.begin(&server, ota_user,ota_password);    // Start ElegantOTA
    server.begin();
  #else
    Serial.printf("[%s]: !!! OTA NOT ENABLED !!! \n",__func__);
  #endif
  //OTA in Setup END

  //start MQTT service
  mqttc.setServer(HA_MQTT_SERVER, 1883);
  mqttc.setBufferSize(MQTT_PAYLOAD_MAX_SIZE);
  mqttc.setCallback(mqtt_callback);
  //mqtt reconnect
  mqtt_reconnect();
  while (!mqtt_connected)
  {
    Serial.printf("\n[%s]: MQTT NOT connected, waiting...\n",__func__);
    sm = millis(); while(millis() < sm + 1000) {};
  }
  if (debug_mode) Serial.printf("[%s]: MQTT connected\n",__func__);
  //start MQTT service END

  if (debug_mode) Serial.printf("[%s]: configuring GATEWAY status in HA:...",__func__);
  if (mqtt_publish_gw_status_config())
  {
    if (debug_mode) Serial.printf("done\n");
  } else
  {
    if (debug_mode) Serial.printf("FAILED\n");
    Serial.printf("[%s]: configuring GATEWAY FAILED, restarting in 3s\n",__func__);
    sm = millis(); while(millis() < sm + 3000) {};
    ESP.restart();
  }

  // initial status of GW:
  if (mqtt_publish_gw_status_values("STARTING"))
  {
    sm = millis(); while(millis() < sm + 300) {};
    mqtt_publish_gw_status_values("STARTED");
    sm = millis(); while(millis() < sm + 300) {};
  } else
  {
    Serial.printf("[%s]: start FAILED, restarting in 3s\n",__func__);
    sm = millis(); while(millis() < sm + 3000) {};
    ESP.restart();
  }

  // initial heartbeat
  heartbeat();
  // clear the message on HA:
  mqtt_publish_gw_last_updated_sensor_values("N/A");

  // finally... ;-)
  espnow_start();

  Serial.printf("[%s]: Setup finished\n",__func__);
  Serial.printf("=============================================================\n");
}


void loop()
{
  unsigned long sm = 0;
  if (!mqttc.connected()) {
    mqtt_reconnect();
  }
  mqttc.loop();

  // turn RED LED on if publishing to HA is OFF - it is turned ON in mqtt_callback
  #ifdef STATUS_GW_LED_GPIO_RED
    if (!publish_sensors_to_ha)
    {
      digitalWrite(STATUS_GW_LED_GPIO_RED, HIGH);
    }
  #endif

  if (millis() >= aux_heartbeat_interval + (HEARTBEAT_INTERVAL_S * 1000))
  {
    heartbeat();
    aux_heartbeat_interval = millis();
  }

  // check queue
  int queue_count = uxQueueMessagesWaiting(queue);
  if ((queue_count > 0) and (publish_sensors_to_ha))
  {
    mqtt_publish_sensors_values();
  }

  // check queue again after publishing
  queue_count = uxQueueMessagesWaiting(queue);
  if (queue_count >= MAX_QUEUE_COUNT)
  {
    Serial.printf("[%s]: queue FULL\n",__func__);
    mqtt_publish_gw_last_updated_sensor_values("queue FULL!");
    long sm2 = millis(); while(millis() < sm2 + 1000) {};
    Serial.printf("[%s]: RESTARTING\n",__func__);
    mqtt_publish_gw_status_values("RESTARTING");
    sm2 = millis(); while(millis() < sm2 + 1000) {};
    ESP.restart();
  }

  do_update();
}
