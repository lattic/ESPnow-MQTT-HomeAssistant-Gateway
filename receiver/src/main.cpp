#include <Arduino.h>

// #define DEBUG
#define VERSION "2.26.b1"


// gateways config file
#include "config.h"

// libraries
#include <WiFi.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <esp_now.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>

#include <Preferences.h>
#include <nvs_flash.h>

// CO2 MHZ19
#if (USE_MHZ19_CO2 == 1)
  #include "MHZ19.h"
  #include <SoftwareSerial.h>                                // Remove if using HardwareSerial
  MHZ19 myMHZ19; 
  SoftwareSerial mySerial(RX_PIN, TX_PIN);  
#endif

// webserial server 
#if (USE_WEB_SERIAL == 1)
  #include <WebSerialLite.h>
#endif
// webserial server END


// Firmware update
#include <HTTPClient.h>
#include <Update.h>
#include "passwords.h" 

#if (OTA_ACTIVE == 1)
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <AsyncElegantOTA.h>
  AsyncWebServer server(8080);
  // #warning "OTA_ACTIVE defined"
#else
  #warning "OTA_ACTIVE NOT defined"
#endif

#if (USE_INFLUXDB == 1)
  #include <InfluxDbClient.h>
  #include <InfluxDbCloud.h>
  InfluxDBClient client_influxdb(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  // Data point
  Point sensorReadings("measurements");
#endif

// BMP280 temperature and pressure, I2C
#if (USE_BMP280 == 1)
  #include <Adafruit_BMP280.h>
  Adafruit_BMP280 bmp; // I2C
  #include "measure-bmp280.h"
#endif

// VARIABLES
#include "variables.h"
// VARIABLES END


// fuctions declarations

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
// bool mqtt_publish_sensors_config(const char* hostname, const char* name, const char* mac, const char* fw, const char* dev_type);
bool mqtt_publish_sensors_config(const char* hostname, const char* name, const char* mac, const char* fw, const char* dev_type, byte boardtype, uint16_t sleep_time_s);

bool mqtt_publish_sensors_values();
bool mqtt_publish_button_restart_config();
bool mqtt_publish_button_update_config();
bool mqtt_publish_switch_publish_config();
bool mqtt_publish_switch_publish_values();
bool mqtt_publish_button_config(const char* button);
bool mqtt_publish_text_sensor_config(const char* text_sensor);
bool mqtt_publish_text_sensor_value(const char* text_sensor, const char* text_sensor_value);
bool mqtt_publish_sensor_with_unit_config(const char* sensor_with_unit, const char* sensor_unit);
bool mqtt_publish_sensor_with_unit_value(const char* sensor_with_unit, const char* sensor_unit, const char* sensor_with_unit_value);

bool mqtt_publish_light_config(const char* light, bool optimistic);
bool mqtt_publish_light_values(const char* light, bool power, u_int8_t brightness);

// firmware update
void do_update();
void updateFirmware(uint8_t *data, size_t len);
int update_firmware_prepare();

void cb_fw_update_timer(TimerHandle_t xTimer );
bool fw_update_timer_start();
bool fw_update_timer_stop();

// various other here in various.h
void write_badbootcount(u_int8_t count);
void do_esp_restart();
void check_dirty_restart();
void ConvertSectoDay(unsigned long n, char * pretty_ontime);
void uptime(char *uptime);
void initiate_all_leds();
void set_sensors_led_level(u_int8_t level);
void set_gateway_led_level(u_int8_t level);
void set_standby_led_level(u_int8_t level);

// measure-bmp280.h
#if (USE_BMP280 == 1)
  void measure_temp_pressure(char *pressure, char *temperature); 
#endif

// measure-co2.h
#if (USE_MHZ19_CO2 == 1)
  void cb_measure_co2_timer( TimerHandle_t xTimer );
  bool measure_co2_timer_start();
  bool measure_co2_timer_stop();

  void cb_calibrate_mhz19_timer(TimerHandle_t xTimer );
  bool calibrate_mhz19_timer_start();
  bool calibrate_mhz19_timer_stop();
#endif

#if (MEASURE_LUX == 1)
  void measure_volt(int pin, int attennuation, int iteration, float resistors_calibration, bool with_lut, String arr_volts[]);
  void get_lux(char* lux);
#endif 

// captive_portal.h
void CaptivePortalServer();
void StartWiFiAP();
void StartWiFiSTA(String rec_ssid, String rec_password);
void StartAPandCaptivePortal();
bool connect_wifi();
void erase_nvs(bool force);
void cp_timer( TimerHandle_t cp_timer_handle );
void start_cp_timer();
void led_blink_cp(void *pvParams);
void read_wifi_credentials();
// void write_wifi_credentials(bool wifi_ok_local, String ssid_str_local, String password_str_local);
void write_wifi_credentials(bool wifi_ok_local, String ssid_str_local, String password_str_local, String mqtt_server_str_local, String mqtt_port_str_local, String mqtt_user_str_local, String mqtt_password_str_local);

void stop_cp_timer();
void change_mac();

void change_mac();

bool send_command_to_sender(u_int8_t command);
String mac_to_string(uint8_t *addr);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
// fuctions declarations END



// separate files with functions
// #include "zhwifi.h"
#include "espnow.h"
#include "mqtt.h"
#include "mqtt_publish_gw_data.h"
#include "mqtt_publish_sensors_data.h"
#include "fw_update.h"
#include "various.h"
#include "captive-portal-local.h"
#if (USE_WEB_SERIAL == 1)
  #include "web-serial-mini.h"
#endif
#if (USE_MHZ19_CO2 == 1)
  #include "measure-co2.h"
#endif

#if (MEASURE_LUX == 1)
  #include "measure-volts.h"
  #include "measure-lux.h"
#endif 
// separate files with functions END



void setup()
{
  bool debug_mode = false;
  long sm = 0;
  program_start_time = millis();

  initiate_all_leds();
  set_sensors_led_level(1);
  set_gateway_led_level(1);
  set_standby_led_level(1);

  Serial.begin(115200);
  sm = millis(); while(millis() < sm + 1000) {};

  Serial.printf("\n\n =============================================================\n");
  change_mac();
  Serial.printf("[%s]: Program started, hostname=%s, DEVICE_ID=%d, version=%s, MAC:%s\n",__func__,HOSTNAME,DEVICE_ID,VERSION, mac_new_char);

  check_badbootcount();

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


  queue_commands_protect = xSemaphoreCreateMutex();
  if (queue_commands_protect != NULL)
  {
    // if (debug_mode) 
      Serial.printf("[%s]: queue_commands_protect semaphore created\n",__func__);
  }
  queue_commands = xQueueCreate( MAX_QUEUE_COMMANDS_COUNT, sizeof( struct struct_commands ) );
  if(queue_commands == NULL)
  {
    Serial.printf("[%s]: Error creating the queue_commands, restarting in 3s\n",__func__);
    sm = millis(); while(millis() < sm + 3000) {};
    ESP.restart();
  } else 
  {
     Serial.printf("[%s]: Created queue_commands\n",__func__);
  }

  wifi_connected = connect_wifi();

  while (!wifi_connected)
  {
    Serial.printf("[%s]: WIFI NOT connected\n",__func__);
    sm = millis(); while(millis() < sm + 1000) {};
  }
  if (debug_mode) Serial.printf("[%s]: WIFI  connected\n",__func__);


  // influxdb 
  #if(USE_INFLUXDB == 1)
    timeSync(MY_TIME_ZONE, "pool.ntp.org", "time.nis.gov");
    // Check server connection
    if (client_influxdb.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client_influxdb.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client_influxdb.getLastErrorMessage());
    }
  #endif
  // influxdb END

  // webserial server
  #if (USE_WEB_SERIAL == 1)
    WebSerial.begin(&serial_web_server,"/log");
    WebSerial.onMessage(message_from_web);
    serial_web_server.begin();
  #endif
  // webserial server END

  //OTA in Setup
  #if (OTA_ACTIVE == 1)
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

  //bmp280
  #if (USE_BMP280 == 1)
    Serial.printf("[%s]: start USE_BMP280\n",__func__);
    if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
      Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    } else 
    {
      Serial.printf("[%s]: BMP280 is OK\n",__func__);
    }
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  #endif  
  //bmp280 END

  #ifdef PUSH_BUTTON_GPIO
    Serial.printf("[%s]: configuring button\n",__func__);
    // any GPIO - set in config.h, observe if active LOW or HIGH
    // GPIO_0 on S, S2 and S3 boards and GPIO_9 on C3 are active LOW (so true below)  
    if (PUSH_BUTTON_GPIO_ACT == HIGH)
    {
      Serial.printf("[%s]: GPIO=%d, active=%d so inverted=false, INPUT_PULLDOWN\n",__func__,PUSH_BUTTON_GPIO,PUSH_BUTTON_GPIO_ACT);
      b = new Button(PUSH_BUTTON_GPIO,false); // false = active HIGH, true = active LOW
      pinMode(PUSH_BUTTON_GPIO,INPUT_PULLDOWN);
    } else 
    {
      Serial.printf("[%s]: GPIO=%d, active=%d so inverted=true, INPUT_PULLUP\n",__func__,PUSH_BUTTON_GPIO,PUSH_BUTTON_GPIO_ACT);
      b = new Button(PUSH_BUTTON_GPIO,true); // false = active HIGH, true = active LOW
      pinMode(PUSH_BUTTON_GPIO,INPUT_PULLUP);
    }
    // pinMode(PUSH_BUTTON_GPIO,INPUT_PULLDOWN);

    // b->setDefaultMinPushTime(150);
    // b->setDefaultMinReleaseTime(150);
    // b->setDefaultTimeSpan(250);
    // b->setDefaultHoldTime(1000);

    // VERY IMPORTANT TIMING: minPushTime, timeSpan, minReleaseTime, interval
    
    // setOnClicked(ButtonEventFunction, uint32_t minPushTime, uint32_t minReleaseTime);
    b->setOnClicked(&OnClicked_cb,100,100); // single click as minimum 200ms otherwise double click is always second

    // setOnDoubleClicked(ButtonEventFunction, uint32_t minPushTime, uint32_t timeSpan);
    // b->setOnDoubleClicked(&OnDoubleClicked_cb,100,1000);
    
    // setOnHolding(ButtonEventFunction, uint32_t interval);
    // b->setOnHolding(&OnHolding_cb,500);
  #endif


  #if (USE_MHZ19_CO2 == 1)
    Serial.printf("[%s]: start USE_MHZ19_CO2\n",__func__);
    mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start
    myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin().
    myMHZ19.autoCalibration(MHZ19_AUTO_CALIBRATION);        // Turn auto calibration ON (OFF autoCalibration(false))
    myMHZ19.setRange(5000);
    u_int16_t mhz19_range = myMHZ19.getRange();
    Serial.printf("[%s]: MHZ19 range: %d\n",__func__,mhz19_range);
    Serial.printf("[%s]: MHZ19 ABC:   %d\n",__func__,myMHZ19.getABC());
    if (mhz19_range > 0) 
    {
      Serial.printf("[%s]: MHZ19 is OK\n",__func__);
      if (measure_co2_timer_start()) 
        Serial.printf("[%s]: CO2 measurement timer started\n",__func__);
      else 
        Serial.printf("[%s]: CO2 measurement timer NOT started\n",__func__);
    } else 
    {
      Serial.printf("[%s]: MHZ19 NOT detected ... Check your wiring power\n",__func__);
    }
  #endif

  delay(500);

  //start MQTT service
  #if (USE_WEB_SERIAL == 1)
    WebSerial.println("Starting MQTT...");
  #endif
  mqttc.setServer(mqtt_server_str.c_str(), mqtt_port_str.toInt());
  mqttc.setBufferSize(MQTT_PAYLOAD_MAX_SIZE);
  mqttc.setCallback(mqtt_callback);
  //mqtt reconnect
  mqtt_reconnect();
  while (!mqtt_connected)
  {
    #if (USE_WEB_SERIAL == 1)
      WebSerial.println("MQTT NOT connected, waiting...");
    #endif
    Serial.printf("\n[%s]: MQTT NOT connected, waiting...\n",__func__);
    sm = millis(); while(millis() < sm + 1000) {};
  }
  if (mqtt_connected)
  {
    Serial.printf("[%s]: MQTT connected\n",__func__);
    #if (USE_WEB_SERIAL == 1)
      WebSerial.println("MQTT connected");
    #endif
  } else 
  {
    Serial.printf("[%s]: MQTT NOT connected\n",__func__);
    #if (USE_WEB_SERIAL == 1)
      WebSerial.println("MQTT NOT connected");
    #endif
  }
  //start MQTT service END


  // initial MQTT config
  mqtt_publish_gw_status_values("STARTING");
  sm = millis(); while(millis() < sm + 300) {};
  mqtt_publish_gw_status_values("STARTED");
  sm = millis(); while(millis() < sm + 300) {};
  mqtt_publish_button_update_config();
  mqtt_publish_button_restart_config();
  mqtt_publish_switch_publish_config();
  mqtt_publish_switch_publish_values();
  mqtt_publish_gw_last_updated_sensor_config();
  mqtt_publish_button_config("reset");
  mqtt_publish_button_config("reset_cmd_queue");
  mqtt_publish_text_sensor_config("cmd_queue_size");

  #if (USE_MHZ19_CO2 == 1)
    mqtt_publish_button_config("calibrate");
    mqtt_publish_sensor_with_unit_config("co2", "ppm");
  #endif

  #if (MEASURE_LUX == 1)
    char lux[7];
    get_lux(lux);
    mqtt_publish_sensor_with_unit_config("lux","lx");
  #endif

  mqtt_publish_light_values("led_sensors",led_sensors_power,led_gpio_sensors_dc);
  mqtt_publish_light_values("led_gateway",led_gateway_power,led_gpio_gateway_dc);
  mqtt_publish_light_values("led_standby",led_standby_power,led_gpio_standby_dc);

  #if (USE_BMP280 == 1)
    mqtt_publish_sensor_with_unit_config("airpressure","hPa");
    char pressure_chr[10]; char tempbmp280_chr[10];
    measure_temp_pressure(pressure_chr,tempbmp280_chr);
    mqtt_publish_sensor_with_unit_value("bmp280_temperature","°C",tempbmp280_chr);
  #endif

  // initial heartbeat
  heartbeat();

  // clear the message on HA:
  mqtt_publish_gw_last_updated_sensor_values("N/A");

  // finally... ;-)
  espnow_start();

  set_sensors_led_level(0);
  set_gateway_led_level(0);
  print2web = false;

  Serial.printf("[%s]: Setup finished\n",__func__);
  Serial.printf("=============================================================\n");
}


void loop()
{
  unsigned long sm = 0;
  
  wifi_connected = connect_wifi();

  if (!skip_mqtt)
  {
    if (!mqttc.connected()) {
      mqtt_reconnect();
    }
    mqttc.loop();
  }

  // safeguarding if MQTT not connected for MAX_MQTT_ERROR seconds (180 now) - it restarts ESP unconditionally 
  if (mqttc.connected()) mqtt_last_connected = millis();

  if (millis() >= aux_mqtt_last_checked_interval + (3 * 1000)) 
  {
    if  (!mqttc.connected())
    {
      if (((millis() - mqtt_last_connected) / 1000) > MAX_MQTT_ERROR) // MAX_MQTT_ERROR)
      {
        #if (USE_WEB_SERIAL == 1)
          WebSerial.println("mqtt timeout..., restarting in 3s");
        #endif
        delay(3000);
        ESP.restart();
      }
    }
    aux_mqtt_last_checked_interval = millis();
  }



    // button
  #ifdef PUSH_BUTTON_GPIO
    b->update();
    if (b->getState() == 1)
    {
      unsigned long now = millis();
      press_time = now - released;
      // setting LEDs as per ESPnow devices (1 on 2 off until 3s, then change, then again at 6s and 9s)
      { 
        if (press_time < 3000)  
        {
          set_standby_led_level(0);
          set_gateway_led_level(1);
        } else
        if (press_time < 6000)  
        {
          set_standby_led_level(1);
          set_gateway_led_level(0);
        } else
        if (press_time < 9000)  
        {
          set_standby_led_level(0);
          set_gateway_led_level(1);
        }
        else  
        if (press_time > 9000)  
        {
          set_standby_led_level(1);
          set_gateway_led_level(0);
        }
      }

      // updating HA and Serial
      if (millis() >= aux_pushbutton_interval + PUSHBUTTON_UPDATE_INTERVAL_MS)
      {
        if (press_time < 3000)  // RED on, GREEN off
        {
          Serial.printf("[%s]: click (%ums)->doing nothing?\n",__func__,press_time);
          mqtt_publish_gw_status_values("nothing?");
        } else
        if (press_time < 6000)  // RED off, GREEN on
        {
          Serial.printf("[%s]: click (%ums)->RESTART?\n",__func__,press_time);
          mqtt_publish_gw_status_values("Restart?");
        } else
        if (press_time < 9000)  // RED on, GREEN off
        {
          Serial.printf("[%s]: click (%ums)->FW update?\n",__func__,press_time);
          mqtt_publish_gw_status_values("FW update?");
        }
        else  
        if (press_time > 9000)  // RED off, GREEN on
        {
          Serial.printf("[%s]: click (%ums)->erase NVS?\n",__func__,press_time);
          mqtt_publish_gw_status_values("erase NVS");
        }
        aux_pushbutton_interval = millis();
      }      
    } else 
    {
      released = millis();
      set_standby_led_level(1);
      set_gateway_led_level(0);
    }
  #endif

  // turn LED on if publishing to HA is OFF - it is turned ON in mqtt_callback
  if (!publish_sensors_to_ha)
  {
    set_sensors_led_level(1);
  }

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
    // influxdb_publish_sensors_values();
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
    do_esp_restart();
  }

  do_update();
}
