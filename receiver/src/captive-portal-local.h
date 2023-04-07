
#ifndef captive_portal_h
#define captive_portal_h

#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>
#include <nvs_flash.h>

#ifndef CP_TIMEOUT_S
    #define CP_TIMEOUT_S                    300             // Captive Portal will terminate after this time and ESP will restart
#endif
#ifndef WIF_STA_CONNECTION_TIMEOUT_S
    #define WIF_STA_CONNECTION_TIMEOUT_S    10              // WiFi STA will give up after this time and CP will start
#endif
#ifndef HOSTNAME
  #define HOSTNAME                          "ESP-CAPTIVE-PORTAL"
#endif
  
Preferences preferences;
DNSServer dnsServer;
AsyncWebServer cp_server(80);

String ssid_str;
String password_str;

bool is_setup_done = false;
bool valid_ssid_received = false;
bool valid_password_received = false;

bool cp_wifi_timeout = false;

// timers
TimerHandle_t cp_timer_handle     = NULL;
int id                            = 1;

// tasks
TaskHandle_t led_blink_handle     = NULL;
BaseType_t xReturned_led_blink;

// rssi thingis - to see the rssi of the sender - gateway measures always the last rssi: being from GW to AP or from sender to GW
typedef struct
{
  unsigned frame_ctrl: 16;
  unsigned duration_id: 16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl: 16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct
{
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;




const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Captive Portal Demo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h3>Captive Portal Demo</h3>
  <br><br>
  <form action="/get">
    <br>
    WiFi SSID: <input type="text" name="ssid">
    <br>
    WiFi Password: <input type="password" name="password">
    <br>
    MQTT Server: <input type="text" name="mqtt_server">
    <br>
    MQTT Port: <input type="text" name="mqtt_port">
    <br>
    MQTT User: <input type="text" name="mqtt_user">
    <br>
    MQTT Password: <input type="password" name="mqtt_password">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", index_html);
    }
};

void CaptivePortalServer() 
{
  cp_server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
    Serial.printf("[%s]: Client Connected\n",__func__);
  });

  cp_server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputParam;

    if (request->hasParam("ssid")) {
      inputMessage = request->getParam("ssid")->value();
      inputParam = "ssid";
      ssid_str = inputMessage;
      Serial.print("ssid=");
      Serial.println(inputMessage);
      valid_ssid_received = true;
    }

    if (request->hasParam("password")) {
      inputMessage = request->getParam("password")->value();
      inputParam = "password";
      password_str = inputMessage;
      Serial.print("password=");
      Serial.println(inputMessage);
      valid_password_received = true;
    }

    if (request->hasParam("mqtt_server")) {
      inputMessage = request->getParam("mqtt_server")->value();
      inputParam = "mqtt_server";
      mqtt_server_str = inputMessage;
      Serial.print("mqtt_server=");
      Serial.println(inputMessage);
      valid_mqtt_server_received = true;
    }

    if (request->hasParam("mqtt_port")) {
      inputMessage = request->getParam("mqtt_port")->value();
      inputParam = "mqtt_port";
      mqtt_port_str = inputMessage;
      Serial.print("mqtt_port=");
      Serial.println(inputMessage);
      valid_mqtt_port_received = true;
    }

    if (request->hasParam("mqtt_user")) {
      inputMessage = request->getParam("mqtt_user")->value();
      inputParam = "mqtt_user";
      mqtt_user_str = inputMessage;
      Serial.print("mqtt_user=");
      Serial.println(inputMessage);
      valid_mqtt_user_received = true;
    }

    if (request->hasParam("mqtt_password")) {
      inputMessage = request->getParam("mqtt_password")->value();
      inputParam = "mqtt_password";
      mqtt_pass_str = inputMessage;
      Serial.print("mqtt_password=");
      Serial.println(inputMessage);
      valid_mqtt_password_received = true;
    }

    request->send(200, "text/html", "The values entered by you have been successfully sent to the device. It will now attempt WiFi connection");
  });
}

void StartWiFiAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(HOSTNAME);
  Serial.printf("[%s]: SSID: ",__func__); Serial.println(HOSTNAME);
  Serial.printf("[%s]: AP IP address: ",__func__); Serial.println(WiFi.softAPIP());
    // blink LEDs in task, if not yet blinking
  if (led_blink_handle == NULL)
  {
    xReturned_led_blink = xTaskCreate(led_blink_cp, "led_blink_cp", 2000, NULL, 1, &led_blink_handle);
    if( xReturned_led_blink != pdPASS )
    {
      Serial.printf("[%s]: CANNOT create led_blink task\n",__func__);
    } else
    {
      // #ifdef DEBUG
        Serial.printf("[%s]: Task led_blink created, blinking LED GPIO=%d\n",__func__);
      // #endif
    }
  } else
  // blink LEDs already blinking
  {
    Serial.printf("[%s]: Task led_blink ALREADY created\n",__func__);
  }
}

void StartWiFiSTA(String rec_ssid, String rec_password)
{
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&config);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_promiscuous(0);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  esp_wifi_set_promiscuous(1);

  cp_wifi_timeout = false;
  WiFi.mode(WIFI_STA);
  char ssid_arr[20];
  char password_arr[20];
  rec_ssid.toCharArray(ssid_arr, rec_ssid.length() + 1);
  rec_password.toCharArray(password_arr, rec_password.length() + 1);

  // THIS SETS PROPERLY THE HOSTNAME - WiFi.setHostname(HOSTNAME); is NOT WORKING - receiver
  esp_netif_t *esp_netif = NULL;
  esp_netif = esp_netif_next(esp_netif);
  esp_netif_set_hostname(esp_netif, HOSTNAME);

  WiFi.begin(ssid_arr, password_arr);

  uint32_t t1 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(20);
    Serial.print(".");
    if (millis() - t1 > (WIF_STA_CONNECTION_TIMEOUT_S * 1000)) 
    {
      Serial.printf("\n[%s]: Timeout connecting to WiFi. The SSID and Password seem incorrect\n",__func__);
      valid_ssid_received = false;
      valid_password_received = false;
      bool old_is_setup_done = is_setup_done;
      is_setup_done = false;
      if (is_setup_done != old_is_setup_done)
      {
        Serial.printf("[%s]: saving WiFi credentials (is_setup_done=%d): WiFi NOT ok\n",__func__,is_setup_done);
        preferences.begin("my-pref", false); //write
        preferences.putBool("is_setup_done", is_setup_done);
        preferences.end();
        delay(20);
      }
      StartAPandCaptivePortal();
      cp_wifi_timeout = true;
      break;
    }
  }
  if (!cp_wifi_timeout)
  {
    Serial.printf("\n[%s]: WiFi connected to: ",__func__); Serial.println(rec_ssid);
    Serial.printf  ("[%s]: IP address: ",__func__);  Serial.println(WiFi.localIP());
    bool old_is_setup_done = is_setup_done;
    is_setup_done = true;
    if (is_setup_done != old_is_setup_done)
    {
      Serial.printf("[%s]: WiFi OK\n",__func__);
      write_wifi_credentials(true,  rec_ssid, rec_password, mqtt_server_str, mqtt_port_str, mqtt_user_str, mqtt_pass_str);
      // stop blinking
      if( led_blink_handle != NULL )
      {
        Serial.printf("[%s]: Disabling blinking LED\n",__func__);
        vTaskDelete( led_blink_handle );
        delay(5);
      } else
      {
        Serial.printf("[%s]: LED is still blinking or was never blinking\n",__func__);
      }
    }

    if(!MDNS.begin(HOSTNAME)) 
    {
      Serial.printf("[%s]: Error starting MDNS\n",__func__);
    }
  }
}

void StartAPandCaptivePortal() {
  Serial.printf("[%s]: Setting up AP Mode\n",__func__);
  StartWiFiAP();
  Serial.printf("[%s]: Setting up Async WebServer\n",__func__);
  CaptivePortalServer();
  Serial.printf("[%s]: Starting DNS Server\n",__func__);
  dnsServer.start(53, "*", WiFi.softAPIP());
  cp_server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  cp_server.begin();
  dnsServer.processNextRequest();
}

void erase_nvs(bool force)
// if force=true then only delete - don't rewrite preferences
{
  if (!force) read_wifi_credentials();
  Serial.printf("[%s]: nvs_flash_erase...\n",__func__);
  if (nvs_flash_erase() != ESP_OK) 
  {
    Serial.printf("[%s]: nvs_flash_erase FAILED\n",__func__);
  } else 
  {
    Serial.printf("[%s]: nvs_flash_erase SUCCEESFUL\n",__func__);
  }
  delay(20);
  if (nvs_flash_init() != ESP_OK) 
  {
      Serial.printf("[%s]: nvs_flash_init FAILED\n",__func__);
  } else 
  {
    Serial.printf("[%s]: nvs_flash_init SUCCEESFUL\n",__func__);
  }
 if (!force) write_wifi_credentials(is_setup_done, ssid_str, password_str, mqtt_server_str, mqtt_port_str, mqtt_user_str, mqtt_pass_str);
}

void cp_timer( TimerHandle_t cp_timer_handle )
{
  Serial.printf("[%s]: Captive Portal timer expired\n",__func__);
  erase_nvs(false);
  Serial.printf("[%s]: RESTARTING...\n",__func__);
  ESP.restart();
}

void start_cp_timer()
{
      // create CP timer if not yet created
  if (cp_timer_handle  == NULL)
  {
    cp_timer_handle = xTimerCreate("MyTimer", pdMS_TO_TICKS(CP_TIMEOUT_S * 1000), pdTRUE, ( void * )id, &cp_timer);
    if( xTimerStart(cp_timer_handle, 10 ) != pdPASS )
    {
      Serial.printf("[%s]: CP timer start error\n",__func__);
    } else
    {
    //   #ifdef DEBUG
        Serial.printf("[%s]: CP timer STARTED\n",__func__);
    //   #endif
    }
  } else
  // CP timer created so restart it
  {
    if( xTimerReset( cp_timer_handle, 0 ) != pdPASS )
    {
      Serial.printf("[%s]: CP timer was not reset\n",__func__);
    } else
    {
      Serial.printf("[%s]: CP timer RE-STARTED\n",__func__);
    }
  }
}

void stop_cp_timer()
{
      if( xTimerStop( cp_timer_handle, 0 ) != pdPASS )
    {
      Serial.printf("[%s]: CP timer was NOT stopped\n",__func__);
    } else
    {
      Serial.printf("[%s]: CP timer stopped\n",__func__);
    }
}

void led_blink_cp(void *pvParams)
{
  int delay_ms = 50;
    while(1)
    {
      //  low
      set_gateway_led_level(0);

      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);

      //  high
      set_gateway_led_level(1);

      // delay
      vTaskDelay(delay_ms/portTICK_RATE_MS);
    }
}

void read_wifi_credentials()
{
  Serial.printf("[%s]: reading WiFi credentials...\n",__func__);
  preferences.begin("my-pref", true); //read only
  is_setup_done = preferences.getBool("is_setup_done", false);
  ssid_str = preferences.getString("rec_ssid", "Sample_SSID");
  password_str = preferences.getString("rec_password", "abcdefgh");

  mqtt_server_str   = preferences.getString("mqtt_server_str",    "Sample_server");
  mqtt_port_str     = preferences.getString("mqtt_port_str",      "Sample_port");
  mqtt_user_str     = preferences.getString("mqtt_user_str",      "Sample_user");
  mqtt_pass_str = preferences.getString("mqtt_pass_str",  "Sample_pass");

  Serial.print(" ssid=");Serial.println(ssid_str);
  Serial.print(" password_str=");Serial.println(password_str);
  Serial.print(" is_setup_done=");Serial.println(is_setup_done);

  Serial.print(" mqtt_server_str=");Serial.println(mqtt_server_str);
  Serial.print(" mqtt_port_str=");Serial.println(mqtt_port_str);
  Serial.print(" mqtt_user_str=");Serial.println(mqtt_user_str);
  Serial.print(" mqtt_pass_str=");Serial.println(mqtt_pass_str);


  preferences.end();
}

void write_wifi_credentials(bool wifi_ok_local, String ssid_str_local, String password_str_local, String mqtt_server_str_local, String mqtt_port_str_local, String mqtt_user_str_local, String mqtt_password_str_local)
{
  Serial.printf("[%s]: saving WiFi credentials...\n",__func__);
  preferences.begin("my-pref", false); //write
  preferences.putBool("is_setup_done", wifi_ok_local);
  preferences.putString("rec_ssid", ssid_str_local);
  preferences.putString("rec_password", password_str_local);

  preferences.putString("mqtt_server_str",    mqtt_server_str_local);
  preferences.putString("mqtt_port_str",      mqtt_port_str_local);
  preferences.putString("mqtt_user_str",      mqtt_user_str_local);
  preferences.putString("mqtt_pass_str",  mqtt_password_str_local);

  preferences.end();
  delay(20);
}

void change_mac()
{
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac_org);

  snprintf(mac_org_char, sizeof(mac_org_char), "%02x:%02x:%02x:%02x:%02x:%02x",mac_org[0], mac_org[1], mac_org[2], mac_org[3], mac_org[4], mac_org[5]);
  Serial.printf("[%s]: OLD MAC: %s\n",__func__,mac_org_char);

  Serial.printf("[%s]: changing MAC...",__func__);
  if (esp_wifi_set_mac(WIFI_IF_STA, &FixedMACAddress[0]) == ESP_OK) Serial.println("SUCCESSFULL"); else Serial.println("FAILED");

  WiFi.macAddress(mac_new);
  snprintf(mac_new_char, sizeof(mac_new_char), "%02x:%02x:%02x:%02x:%02x:%02x",mac_new[0], mac_new[1], mac_new[2], mac_new[3], mac_new[4], mac_new[5]);
  Serial.printf("[%s]: NEW MAC: %s\n",__func__,mac_new_char);

  // #define MQTT_DEVICE_IDENTIFIER String(mac_new_char)  // already in config.h

}

void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type)
{
  // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
  if (type != WIFI_PKT_MGMT)
    return;
  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  // push rssi into myData_aux.rssi - as it is not with semaphores I am not sure how accurate it is ;-(
  myData_aux.rssi = ppkt->rx_ctrl.rssi;
  // this prints a lot! commented out until needed
  #ifdef DEBUG
    // Serial.printf("[%s]: RSSI: %ddBm\n",__func__,myData_aux.rssi);
  #endif
}
// rssi thingis END

bool connect_wifi()
{   
  if (WiFi.status() == WL_CONNECTED)
  {
     return true;
  }

  // start timer
  start_cp_timer();

  read_wifi_credentials();
  Serial.printf("[%s]: Saved SSID is:......... ",__func__);Serial.println(ssid_str);
  Serial.printf("[%s]: Saved Password is:..... ",__func__);Serial.println(password_str);
  Serial.printf("[%s]: Saved wifi status is:.. ",__func__);Serial.println(is_setup_done);
  
  // lets try first connection anyway
  StartWiFiSTA(ssid_str, password_str);

  if (!is_setup_done)
  {
    StartAPandCaptivePortal();
  }
  else
  {
    Serial.printf("[%s]: Connecting to WiFi using saved credentials...\n",__func__);
    StartWiFiSTA(ssid_str, password_str);
  }

  while (!is_setup_done)
  {
    dnsServer.processNextRequest();
    delay(10);
    if (valid_ssid_received && valid_password_received)
    {
      Serial.printf("[%s]: Attempting WiFi Connection...\n",__func__);
      StartWiFiSTA(ssid_str, password_str);
    }
  }
  stop_cp_timer();
  return true;
}



#endif