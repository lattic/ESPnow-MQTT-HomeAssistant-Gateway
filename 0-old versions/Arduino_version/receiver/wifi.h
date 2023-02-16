#pragma once
/*
wifi functions
*/
#include "config.h"
#include "variables.h"

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


// functions
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


void setup_wifi(void*z)
{
  Serial.printf("[%s]: WiFi watchdog started, active every 1s\n",__func__);
  long ttc, sm1;
  long wifi_start_time; // = millis();
  IPAddress ipaddress;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress primarydns;

  ipaddress.fromString(IP_ADDRESS);
  gateway.fromString(IP_GATEWAY);
  subnet.fromString(IP_SUBNET);
  primarydns.fromString(IP_DNS);

  while(1)
    {
      wifi_start_time = millis();
      if (WiFi.status() == WL_CONNECTED)
      {
        wifi_connected =  true;
      } else
      {
        Serial.printf("[%s]: WiFi NOT connected\n",__func__);
        mqtt_connected=false;
        WiFi.disconnect();

        wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&config);
        esp_wifi_set_ps(WIFI_PS_NONE);
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_promiscuous(0);
        esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
        esp_wifi_set_promiscuous(1);
        // all protocols, including long range
        esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR);
        esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);


        WiFi.config(ipaddress, gateway, subnet, primarydns);
        WiFi.setHostname(HOSTNAME);
        byte mac[6];
        WiFi.macAddress(mac);
        char mac1[22];
        snprintf(mac1, sizeof(mac1), "%02x:%02x:%02x:%02x:%02x:%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        Serial.printf("[%s]: OLD MAC: %s\n",__func__,mac1);

        //change MAC address to fixed one - needed only in case UNICAST is used plus for router/AP MAC filtering
        // REMOVED as all sensors send to broadcast so no need to change MAC
        // esp_wifi_set_mac(WIFI_IF_STA, &FixedMACAddress[0]);

        WiFi.begin(BT_SSID, BT_PASSWORD,WIFI_CHANNEL);
        Serial.printf("[%s]: Connecting to %s ...\n",__func__,BT_SSID);

        while (WiFi.status() != WL_CONNECTED)
        {
          ttc = millis() - wifi_start_time;
          sm1=millis(); while(millis() < sm1 + 20)
          {
            // Serial.printf("...",__func__);
            vTaskDelay(pdMS_TO_TICKS(10));
          }
          if (ttc > (WAIT_FOR_WIFI * 1000)) {
            Serial.printf("[%s]: NOT connected after %dms\n",__func__,ttc);
            Serial.printf("[%s]: FATAL WiFi ERROR !!!!!!!!\n",__func__);
            Serial.printf("[%s]: restarting ESP...\n\n",__func__);
            // delay(3000);
            ESP.restart();
          }
        }
        Serial.printf("[%s]: CONNECTED after %dms\n",__func__,ttc);
        Serial.printf("[%s]: Channel: %d\n",__func__,WiFi.channel());
        IPAddress ip = WiFi.localIP();
        char ip1[22];
        snprintf(ip1, sizeof(ip1), "%d.%d.%d.%d",ip[0], ip[1], ip[2], ip[3]);
        Serial.printf("[%s]: IP: %s\n",__func__,ip1);
        WiFi.macAddress(mac);
        snprintf(mac1, sizeof(mac1), "%02x:%02x:%02x:%02x:%02x:%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        Serial.printf("[%s]: NEW MAC: %s\n",__func__,mac1);

        #define MQTT_DEVICE_IDENTIFIER String(WiFi.macAddress())
        wifi_connected =  true;
        Serial.printf("[%s]: SUCCESSFULL\n",__func__);
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
