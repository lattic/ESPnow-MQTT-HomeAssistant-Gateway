#pragma once
/*
mqtt functions
*/

#include "config.h"
#include "variables.h"

// mqtt common functions

// mqtt callback
void mqtt_callback(char* topic, byte* message, unsigned int length)
{
  long sm = micros();
  if (!wifi_connected)
  {
    Serial.printf("[%s]: WiFi not connected, leaving...\n",__func__);
    return;
  }

  if (!mqtt_connected)
  {
    Serial.printf("[%s]: MQTT not connected, leaving...\n",__func__);
    return;
  }

  bool publish_status = false;

  char restart_cmd_topic[30];
  snprintf(restart_cmd_topic,sizeof(restart_cmd_topic),"%s/cmd/restart",HOSTNAME);

  char update_cmd_topic[60];
  snprintf(update_cmd_topic,sizeof(update_cmd_topic),"%s/cmd/update",HOSTNAME);

  char publish_cmd_topic[30];
  snprintf(publish_cmd_topic,sizeof(publish_cmd_topic),"%s/cmd/publish",HOSTNAME);

  String messageTemp;
  for (int i = 0; i < length; i++)
  {
    messageTemp += (char)toupper(message[i]);
  }
  Serial.printf("[%s]: MQTT message received:\n\t%s\n\tmessage:\t%s\n",__func__,topic,messageTemp);

  String topic_str = String(topic);

  //restart
  if (String(topic) == restart_cmd_topic)
  {
    if ((messageTemp == "ON") or (messageTemp == "1"))
    {
      const char* restart_char="";  // clean restart command before actual restart to avoid the restart loop
      if (!mqttc.publish(restart_cmd_topic,(uint8_t*)restart_char,strlen(restart_char), true))
      {
        publish_status = false;
        Serial.println("PUBLISH FAILED");
      } else
      {
        publish_status = true;
        Serial.printf("[%s]: PUBLISHED %s\n",__func__,restart_cmd_topic);
      }
      Serial.flush();
      long sm2 = millis(); while(millis() < sm2 + 100) {};
      if (publish_status)
      {
        Serial.printf("\n\n[%s]: RESTARTING on MQTT command\n\n\n",__func__);
        mqtt_publish_gw_status_values("RESTARTING");
        sm2 = millis(); while(millis() < sm2 + 100) {};
        ESP.restart();
      }
    }
  }

  else
  //update
  if (String(topic) == update_cmd_topic)
  {
    if ((messageTemp == "ON") or (messageTemp == "1"))
    {
      const char* update_char="";   //clean update topic before actual update to avoid the update loop
      if (!mqttc.publish(update_cmd_topic,(uint8_t*)update_char,strlen(update_char), true))
      {
        publish_status = false;
        Serial.println("PUBLISH FAILED");
      } else
      {
        publish_status = true;
        Serial.printf("[%s]: PUBLISHED %s\n",__func__,update_cmd_topic);
      }
      long sm2 = millis(); while(millis() < sm2 + 100) {};
      if (publish_status)
      {
        Serial.printf("\n\n[%s]: UPDATING FIRMWARE on MQTT command\n\n\n",__func__);
        perform_update_firmware=true;
      }
    }
  }

  else
  //publish
  if (String(topic) == publish_cmd_topic)
  {
    bool old_publish_sensors_to_ha = publish_sensors_to_ha;
    if ((messageTemp == "ON") or (messageTemp == "1"))
    {
      publish_sensors_to_ha = true;
      #ifdef STATUS_GW_LED_GPIO_RED
        digitalWrite(STATUS_GW_LED_GPIO_RED, LOW);
      #endif
    }
    else
    {
      if ((messageTemp == "OFF") or (messageTemp == "0"))
      {
        publish_sensors_to_ha = false;
      }
    }
    if (publish_sensors_to_ha != old_publish_sensors_to_ha)
    {
      if (mqtt_publish_switch_publish_values())
      {
        Serial.printf("[%s]: switch [publish] changed on MQTT message from: %d to %d\n",__func__,old_publish_sensors_to_ha,publish_sensors_to_ha);
      } else
      {
        Serial.printf("[%s]: switch [publish_sensors_to_ha] NOT SENT TO HA\n",__func__);
        publish_sensors_to_ha = old_publish_sensors_to_ha;
      }
    } else
    {
      Serial.printf("[%s]: switch publish NOT changed\n",__func__);
    }
  }
  long em = micros(); //END measurement time
  long mt=em-sm;
  Serial.printf("[%s]: MQTT CALLBACK TIME: %dus\n",__func__,mt);
}


// mqtt reconnect
void mqtt_reconnect()
{
  if (mqttc.connected())
  {
    mqtt_connected = true;
  } else
  {
    int state1 = mqttc.state();
    Serial.printf("[%s]: MQTT NOT connected (ERROR:%d)\n",__func__,state1);
    long sm = millis();
    int mqtt_error = 0;
    mqtt_connected=false;

    while (!mqttc.connected())
    {
      ++mqtt_error;
      Serial.printf("[%s]: Attempting MQTT connection for %d time\n",__func__,mqtt_error);
      long sm1=millis(); while(millis() < sm1 + 1000) {}

      if (mqttc.connect(HOSTNAME,HA_MQTT_USER, HA_MQTT_PASSWORD))
      {
        mqtt_connected = true;
        Serial.printf("[%s]: MQTT connected\n",__func__);
      } else
      {
        if (mqtt_error >= MAX_MQTT_ERROR)
        {
          int state2 = mqttc.state();
          Serial.printf("[%s]: FATAL MQTT ERROR !!!!!!!! (ERROR:%d)\n",__func__,state2);
          Serial.println("restarting ESP...\n\n");
          ESP.restart();
        }
      }
    }

    // restart
    char restart_cmd_topic[60];
    snprintf(restart_cmd_topic,sizeof(restart_cmd_topic),"%s/cmd/restart",HOSTNAME);
    mqttc.subscribe(restart_cmd_topic);

    // update
    char update_cmd_topic[60];
    snprintf(update_cmd_topic,sizeof(update_cmd_topic),"%s/cmd/update",HOSTNAME);
    mqttc.subscribe(update_cmd_topic);

    // publish
    char publish_cmd_topic[30];
    snprintf(publish_cmd_topic,sizeof(publish_cmd_topic),"%s/cmd/publish",HOSTNAME);
    mqttc.subscribe(publish_cmd_topic);

    Serial.printf("[%s]: GW subscribed to:\n",__func__);
    {
      Serial.printf("\t%s\n",restart_cmd_topic);
      Serial.printf("\t%s\n",update_cmd_topic);
      Serial.printf("\t%s\n",publish_cmd_topic);
    }

    long em = millis();
    long mt=em-sm;
    Serial.printf("[%s]: MQTT RECONNECT TIME: %dms\n",__func__,mt);
  }
}
