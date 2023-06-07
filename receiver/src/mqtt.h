#pragma once
/*
mqtt functions
*/

// #include "config.h"
// #include "variables.h"

// mqtt common functions

// mqtt callback
void mqtt_callback(char* topic, byte* message, unsigned int length)
{
  if (skip_mqtt) 
  {
    mqtt_connected = false;
    return;
  }
  String topic_str = String(topic);
  String messageTemp;
  unsigned long sm = micros();
  unsigned long em, mt;

  if (length == 0)
  {
    // #ifdef DEBUG
      em = micros(); //END measurement time
      mt=em-sm;
      Serial.printf("[%s]: EMPTY MESSAGE - quitting, MQTT CALLBACK TIME: %dus\n",__func__,mt);
    // #endif
    return;
  }

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

  // change to String for later comparisons
  for (int i = 0; i < length; i++)
  {
    messageTemp += (char)toupper(message[i]);
  }
  
  // #ifdef DEBUG
    const char* mqtt_message = messageTemp.c_str();
    Serial.printf("[%s]: MQTT message received:\n\ttopic:\n%s\n\tmessage:\n%s\n\n",__func__,topic,mqtt_message);
  // #endif

  // customisation
  bool publish_status = false;

  char restart_cmd_topic[60];
  snprintf(restart_cmd_topic,sizeof(restart_cmd_topic),"%s/cmd/restart",HOSTNAME);

  char update_cmd_topic[60];
  snprintf(update_cmd_topic,sizeof(update_cmd_topic),"%s/cmd/update",HOSTNAME);

  char publish_cmd_topic[60];
  snprintf(publish_cmd_topic,sizeof(publish_cmd_topic),"%s/cmd/publish",HOSTNAME);

  char reset_cmd_topic[60];
  snprintf(reset_cmd_topic,sizeof(reset_cmd_topic),"%s/cmd/reset",HOSTNAME);

  // command_for_sender
  char command_for_sender_topic[60];
  snprintf(command_for_sender_topic,sizeof(command_for_sender_topic),"%s/cmd/cmd_for_sender",HOSTNAME);

  // reset_queue_cmd
  char reset_queue_cmd_topic[60];
  snprintf(reset_queue_cmd_topic,sizeof(reset_queue_cmd_topic),"%s/cmd/reset_cmd_queue",HOSTNAME);

  // LED lights
    // Sensors - RED
  char light_sensors_cmd_topic[60];
  snprintf(light_sensors_cmd_topic,sizeof(light_sensors_cmd_topic),"%s/light/%s/cmd/switch",HOSTNAME,"led_sensors");

  char brightness_sensors_cmd_topic[60];
  snprintf(brightness_sensors_cmd_topic,sizeof(brightness_sensors_cmd_topic),"%s/light/%s/cmd/brightness",HOSTNAME,"led_sensors");


  // Gateway - GREEN
  char light_gateway_cmd_topic[60];
  snprintf(light_gateway_cmd_topic,sizeof(light_gateway_cmd_topic),"%s/light/%s/cmd/switch",HOSTNAME,"led_gateway");

  char brightness_gateway_cmd_topic[60];
  snprintf(brightness_gateway_cmd_topic,sizeof(brightness_gateway_cmd_topic),"%s/light/%s/cmd/brightness",HOSTNAME,"led_gateway");
  

  // Standby - BLUE
  char light_standby_cmd_topic[60];
  snprintf(light_standby_cmd_topic,sizeof(light_standby_cmd_topic),"%s/light/%s/cmd/switch",HOSTNAME,"led_standby");

  char brightness_standby_cmd_topic[60];
  snprintf(brightness_standby_cmd_topic,sizeof(brightness_standby_cmd_topic),"%s/light/%s/cmd/brightness",HOSTNAME,"led_standby");

  #if (USE_MHZ19_CO2 == 1)
    char calibrate_cmd_topic[60];
    snprintf(calibrate_cmd_topic,sizeof(calibrate_cmd_topic),"%s/cmd/calibrate",HOSTNAME);
  #endif

  // ACTING ON RECEIVED MESSAGE

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
        erase_nvs(false);
        do_esp_restart();
      }
    }
  }
  else 
  //reset
  if (String(topic) == reset_cmd_topic)
  {
    if ((messageTemp == "ON") or (messageTemp == "1"))
    {
      const char* reset_char="";  // clean reset command before actual restart to avoid the restart loop
      if (!mqttc.publish(reset_cmd_topic,(uint8_t*)reset_char,strlen(reset_char), true))
      {
        publish_status = false;
        Serial.println("PUBLISH FAILED");
      } else
      {
        publish_status = true;
        Serial.printf("[%s]: PUBLISHED %s\n",__func__,reset_cmd_topic);
      }
      Serial.flush();
      long sm2 = millis(); while(millis() < sm2 + 100) {};
      if (publish_status)
      {
        Serial.printf("\n\n[%s]: RESETTING FLASH on MQTT command\n\n\n",__func__);
        mqtt_publish_gw_status_values("RESETTING FLASH");
        sm2 = millis(); while(millis() < sm2 + 100) {};
        erase_nvs(true);
        do_esp_restart();
      }
    }
  }

  else 
  //reset_queue_cmd
  if (String(topic) == reset_queue_cmd_topic)
  {
    if ((messageTemp == "ON") or (messageTemp == "1"))
    {
      const char* reset_queue_char="";  // clean reset_queue_cmd command before actual restart to avoid the restart loop
      if (!mqttc.publish(reset_queue_cmd_topic,(uint8_t*)reset_queue_char,strlen(reset_queue_char), true))
      {
        publish_status = false;
        Serial.println("PUBLISH FAILED");
      } else
      {
        publish_status = true;
        Serial.printf("[%s]: PUBLISHED %s\n",__func__,reset_queue_cmd_topic);
      }
      Serial.flush();
      long sm2 = millis(); while(millis() < sm2 + 100) {};
      if (publish_status)
      {
        #ifdef DEBUG
          Serial.printf("\n\n[%s]: RESETTING QUEUE on MQTT command\n\n\n",__func__);
          mqtt_publish_gw_status_values("RESETTING QUEUE");
          sm2 = millis(); while(millis() < sm2 + 100) {};
        #endif
        xQueueReset( queue_commands );
        #ifdef DEBUG
          mqtt_publish_gw_status_values("online");
        #endif
        const char* reset_queue_char="";  // clean reset_queue_char topic
        mqttc.publish(command_for_sender_topic,(uint8_t*)reset_queue_char,strlen(reset_queue_char), true);
        mqtt_publish_text_sensor_value("cmd_queue_size", "0");
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
  //command_for_sender_topic
  if (String(topic) == command_for_sender_topic)
  {
    last_cmd_received = millis();
    const char* command_char = messageTemp.c_str();
    #ifdef DEBUG
      Serial.printf("\n[%s]: COMMAND FOR SENDER RECEIVED:\n",__func__);
      Serial.printf("\n%s\n\n",command_char);
    #endif
    StaticJsonDocument<128> commands_doc;
    DeserializationError err = deserializeJson(commands_doc, command_char);
    if (err) 
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.print(err.c_str());
      Serial.println(", exitting");
      em = micros(); //END measurement time
      mt=em-sm;
      Serial.printf("[%s]: MQTT CALLBACK TIME: %dus\n",__func__,mt);
      return;
    }

    const char* mac = commands_doc["MAC"];
    if (mac == NULL)
    {
      Serial.println("MAC=NULL, exitting");
      em = micros(); //END measurement time
      mt=em-sm;
      Serial.printf("[%s]: MQTT CALLBACK TIME: %dus\n",__func__,mt);
      return;
    }

    size_t mac_length = strlen(mac);
    #ifdef DEBUG
      Serial.print("MAC=");Serial.println(mac);
      Serial.print("MAC lenght=");Serial.println(mac_length);
    #endif
    if ((mac_length < 11) or (mac_length > 17))
    {
      Serial.println("wrong MAC address, exitting");
      em = micros(); //END measurement time
      mt=em-sm;
      Serial.printf("[%s]: MQTT CALLBACK TIME: %dus\n",__func__,mt);
      return;
    }
  
    if( queue_commands_protect != NULL )
    {
      u_int8_t queue_commands_count = uxQueueMessagesWaiting(queue_commands);
      if (queue_commands_count >= MAX_QUEUE_COMMANDS_COUNT)
      {
        Serial.print("MAX_QUEUE_COMMANDS_COUNT reached: ");Serial.println(queue_commands_count);
        Serial.println("exitting...");
        em = micros(); //END measurement time
        mt=em-sm;
        Serial.printf("[%s]: MQTT CALLBACK TIME: %dus\n",__func__,mt);
        return;
      }
      // need to change to lower case due to u_int8_t converting to char MAC is always lower case
      char low_case_mac[18];
      snprintf(low_case_mac,sizeof(low_case_mac),"%s",mac);
      scanf("%s", low_case_mac);
      for(char* c=low_case_mac; *c=tolower(*c); ++c) ;

      snprintf(commands.mac,sizeof(commands.mac),"%s",low_case_mac);
      commands.command = commands_doc["COMMAND"];
      
      if( xSemaphoreTake( queue_commands_protect, ( TickType_t ) 10 ) == pdTRUE )
      {
        xQueueSend(queue_commands, &commands, portMAX_DELAY);
        xSemaphoreGive( queue_commands_protect );

        // update the sensor with queue size on HA
        char queue_size_char[4];
        snprintf(queue_size_char,sizeof(queue_size_char),"%d",queue_commands_count);
        mqtt_publish_text_sensor_value("cmd_queue_size", queue_size_char);

        #ifdef DEBUG
          Serial.println("queue updated");
          Serial.print("\tMAC=");Serial.println(commands.mac);
          Serial.print("\tCOMMAND=");Serial.println(commands.command);
        #endif
      } else 
      {
        Serial.println("cannot take semaphore");
      }
    }
    else 
    {
      Serial.println("queue NOT updated");
    }
  }


  // All LEDs controll 
  // switches
  else
  // Sensors LED
  if (String(topic) == light_sensors_cmd_topic)
  {
    bool old_led_sensors_power = led_sensors_power;
    if ((messageTemp == "ON") or (messageTemp == "1")) {
      led_sensors_power = true;
      if (led_sensors_power != old_led_sensors_power)
        led_gpio_sensors_dc = prev_led_gpio_sensors_dc;
        set_sensors_led_level(1);set_sensors_led_level(0);
        mqtt_publish_light_values("led_sensors",led_sensors_power,led_gpio_sensors_dc);
    }
    else {
    if ((messageTemp == "OFF") or (messageTemp == "0")) {
        led_sensors_power = false;
        if (led_sensors_power != old_led_sensors_power)
          prev_led_gpio_sensors_dc = led_gpio_sensors_dc;
          led_gpio_sensors_dc = 0;
          set_sensors_led_level(0);
          mqtt_publish_light_values("led_sensors",led_sensors_power,prev_led_gpio_sensors_dc);
      }
    }
  } 

  else
  // Gateway LED
  if (String(topic) == light_gateway_cmd_topic)
  {
    bool old_led_gateway_power = led_gateway_power;
    if ((messageTemp == "ON") or (messageTemp == "1")) {
      led_gateway_power = true;
      if (led_gateway_power != old_led_gateway_power)
        led_gpio_gateway_dc = prev_led_gpio_gateway_dc;
        set_gateway_led_level(1);set_gateway_led_level(0);
        mqtt_publish_light_values("led_gateway",led_gateway_power,led_gpio_gateway_dc);
    }
    else {
    if ((messageTemp == "OFF") or (messageTemp == "0")) {
        led_gateway_power = false;
        if (led_gateway_power != old_led_gateway_power)
          prev_led_gpio_gateway_dc = led_gpio_gateway_dc;
          led_gpio_gateway_dc = 0;
          set_gateway_led_level(0);
          mqtt_publish_light_values("led_gateway",led_gateway_power,prev_led_gpio_gateway_dc);
      }
    }
  }    

  else
  // Standby LED
  if (String(topic) == light_standby_cmd_topic)
  {
    bool old_led_standby_power = led_standby_power;
    if ((messageTemp == "ON") or (messageTemp == "1")) {
      led_standby_power = true;
      if (led_standby_power != old_led_standby_power)
        led_gpio_standby_dc = prev_led_gpio_standby_dc;
        set_standby_led_level(1); // set_standby_led_level(0);
        mqtt_publish_light_values("led_standby",led_standby_power,led_gpio_standby_dc);
    }
    else {
    if ((messageTemp == "OFF") or (messageTemp == "0")) {
        led_standby_power = false;
        if (led_standby_power != old_led_standby_power)
          prev_led_gpio_standby_dc = led_gpio_standby_dc;
          led_gpio_standby_dc = 0;
          set_standby_led_level(0);
          mqtt_publish_light_values("led_standby",led_standby_power,prev_led_gpio_standby_dc);
      }
    }
  }    

  // brightness
  else
  // Sensors LED
  if (String(topic) == brightness_sensors_cmd_topic)
  {
    u_int8_t old_led_gpio_sensors_dc = led_gpio_sensors_dc;
    led_gpio_sensors_dc = messageTemp.toInt();
    if (old_led_gpio_sensors_dc != led_gpio_sensors_dc)
    {
      set_sensors_led_level(1);set_sensors_led_level(0); // this one should only blink when needed
      mqtt_publish_light_values("led_sensors",true,led_gpio_sensors_dc);
    }
  }  

  else
  // Gateway LED
  if (String(topic) == brightness_gateway_cmd_topic)
  {
    u_int8_t old_led_gpio_gateway_dc = led_gpio_gateway_dc;
    led_gpio_gateway_dc = messageTemp.toInt();
    if (old_led_gpio_gateway_dc != led_gpio_gateway_dc)
    {
      set_gateway_led_level(1);set_gateway_led_level(0); // this one should only blink when needed
      mqtt_publish_light_values("led_gateway",true,led_gpio_gateway_dc);
    }
  }  

  else
  // Standby LED
  if (String(topic) == brightness_standby_cmd_topic)
  {
    u_int8_t old_led_gpio_standby_dc = led_gpio_standby_dc;
    led_gpio_standby_dc = messageTemp.toInt();
    if (old_led_gpio_standby_dc != led_gpio_standby_dc)
    {
      set_standby_led_level(1);//set_standby_led_level(0); // this one is always ON if it is turned ON
      mqtt_publish_light_values("led_standby",true,led_gpio_standby_dc);
    }
  }   
  // All LEDs controll END

  
  //calibrate
  #if (USE_MHZ19_CO2 == 1)
  
  else

    if (String(topic) == calibrate_cmd_topic)
    {
      if ((messageTemp == "ON") or (messageTemp == "1"))
      {
        const char* calibrate_char="";   //clean calibrate topic before actual calibrate to avoid the calibrate loop
        if (!mqttc.publish(calibrate_cmd_topic,(uint8_t*)calibrate_char,strlen(calibrate_char), true))
        {
          publish_status = false;
          Serial.printf("[%s]: PUBLISH FAILED",__func__);
        } else
        {
          publish_status = true;
          Serial.printf("[%s]: PUBLISHED %s\n",__func__,calibrate_cmd_topic);
        }
        long sm2 = millis(); while(millis() < sm2 + 100) {};
        if (publish_status)
        {
          Serial.printf("\n\n[%s]: CALIBRATE MHZ19 on MQTT command\n",__func__);
          calibrate_mhz19_timer_start();
        }
      }
    }  
  #endif

  else
  //publish
  if (String(topic) == publish_cmd_topic)
  {
    bool old_publish_sensors_to_ha = publish_sensors_to_ha;
    if ((messageTemp == "ON") or (messageTemp == "1"))
    {
      publish_sensors_to_ha = true;
      set_sensors_led_level(0);
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
      #ifdef DEBUG
        Serial.printf("[%s]: switch publish NOT changed\n",__func__);
      #endif
    }
  }
  #ifdef DEBUG
    em = micros(); //END measurement time
    mt=em-sm;
    Serial.printf("[%s]: MQTT CALLBACK TIME: %dus\n",__func__,mt);
  #endif
}


// mqtt reconnect
void mqtt_reconnect()
{
  unsigned long aux_pushbutton_interval;
  unsigned long press_time, released;
  if (skip_mqtt) 
  {
    mqtt_connected = false;
    return;
  }
  if (mqttc.connected())
  {
    mqtt_connected = true;
  } else
  {
    #if (USE_WEB_SERIAL == 1)
      WebSerial.println("connecting MQTT...");
    #endif
    set_gateway_led_level(1);
    int state1 = mqttc.state();
    Serial.printf("[%s]: MQTT NOT connected (ERROR:%d)\n",__func__,state1);
    long sm = millis();
    int mqtt_error = 0;
    mqtt_connected=false;
    

    while (!mqttc.connected())
    {
      // button if MQTT not connected but WiFi connected
      #ifdef PUSH_BUTTON_GPIO
        b->update();
        if (b->getState() == 1)
        {
          unsigned long now = millis();
          press_time = now - released;
          // setting LEDs as per ESPnow devices (red on green off until 3s, then change, then again at 6s and 9s)
          { 
            if (press_time < 3000)  // RED on, GREEN off
            {
              set_standby_led_level(0);
              set_gateway_led_level(1);
            } else
            if (press_time < 6000)  // RED off, GREEN on
            {
              set_standby_led_level(1);
              set_gateway_led_level(0);
            } else
            if (press_time < 9000)  // RED on, GREEN off
            {
              set_standby_led_level(0);
              set_gateway_led_level(1);
            }
            else  
            if (press_time > 9000)  // RED off, GREEN on
            {
              set_standby_led_level(1);
              set_gateway_led_level(0);
            }
          }
          // updating Serial
          if (millis() >= aux_pushbutton_interval + PUSHBUTTON_UPDATE_INTERVAL_MS)
          {
            if (press_time < 3000)  // RED on, GREEN off
            {
              Serial.printf("[%s]: click (%ums)->doing nothing?\n",__func__,press_time);
            } else
            if (press_time < 6000)  // RED off, GREEN on
            {
              Serial.printf("[%s]: click (%ums)->RESTART?\n",__func__,press_time);
            } else
            if (press_time < 9000)  // RED on, GREEN off
            {
              Serial.printf("[%s]: click (%ums)->FW update?\n",__func__,press_time);
            }
            else  
            if (press_time > 9000)  // RED off, GREEN on
            {
              Serial.printf("[%s]: click (%ums)->erase NVS?\n",__func__,press_time);
            }
            aux_pushbutton_interval = millis();
          }      
        }  else 
        {
          released = millis();
          set_standby_led_level(1);
          set_gateway_led_level(0);
        }
      #endif

      ++mqtt_error;
      #if (USE_WEB_SERIAL == 1)
        WebSerial.print("Attempting MQTT connection for ");WebSerial.print(mqtt_error);WebSerial.println(" time");
      #endif

      Serial.printf("[%s]: Attempting MQTT connection for %d time\n",__func__,mqtt_error);
      long sm1=millis(); while(millis() < sm1 + 1000) {}

      // availability topic and last will
      if (mqttc.connect(HOSTNAME,mqtt_user_str.c_str(), mqtt_pass_str.c_str(),AVAILABILITY_TOPIC, 2, true, NOT_AVAILABLE_PAYLOAD))

      {
        mqtt_connected = true;
        Serial.printf("[%s]: MQTT connected\n",__func__);
        #if (USE_WEB_SERIAL == 1)
          WebSerial.println("MQTT connected");
        #endif
        set_gateway_led_level(0);
      } else
      {
        if (skip_mqtt) return;
        if (mqtt_error >= MAX_MQTT_ERROR)
        {
          int state2 = mqttc.state();
          #if (USE_WEB_SERIAL == 1)
            WebSerial.println("FATAL MQTT ERROR !!!!!!!! - RESTARTING");
          #endif
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
    char publish_cmd_topic[60];
    snprintf(publish_cmd_topic,sizeof(publish_cmd_topic),"%s/cmd/publish",HOSTNAME);
    mqttc.subscribe(publish_cmd_topic);

    // reset
    char reset_cmd_topic[60];
    snprintf(reset_cmd_topic,sizeof(reset_cmd_topic),"%s/cmd/reset",HOSTNAME);
    mqttc.subscribe(reset_cmd_topic);

    // command_for_sender
    char command_for_sender_topic[60];
    snprintf(command_for_sender_topic,sizeof(command_for_sender_topic),"%s/cmd/cmd_for_sender",HOSTNAME);
    mqttc.subscribe(command_for_sender_topic);

    // reset_queue_cmd
    char reset_queue_cmd_topic[60];
    snprintf(reset_queue_cmd_topic,sizeof(reset_queue_cmd_topic),"%s/cmd/reset_cmd_queue",HOSTNAME);
    mqttc.subscribe(reset_queue_cmd_topic);

    // LED lights
    // RED
    char light_sensors_cmd_topic[60];
    snprintf(light_sensors_cmd_topic,sizeof(light_sensors_cmd_topic),"%s/light/%s/cmd/switch",HOSTNAME,"led_sensors");
    mqttc.subscribe(light_sensors_cmd_topic);
    char brightness_sensors_cmd_topic[60];
    snprintf(brightness_sensors_cmd_topic,sizeof(brightness_sensors_cmd_topic),"%s/light/%s/cmd/brightness",HOSTNAME,"led_sensors");
    mqttc.subscribe(brightness_sensors_cmd_topic);

    // RED
    char light_gateway_cmd_topic[60];
    snprintf(light_gateway_cmd_topic,sizeof(light_gateway_cmd_topic),"%s/light/%s/cmd/switch",HOSTNAME,"led_gateway");
    mqttc.subscribe(light_gateway_cmd_topic);
    char brightness_gateway_cmd_topic[60];
    snprintf(brightness_gateway_cmd_topic,sizeof(brightness_gateway_cmd_topic),"%s/light/%s/cmd/brightness",HOSTNAME,"led_gateway");
    mqttc.subscribe(brightness_gateway_cmd_topic);    

    // BLUE
    char light_standby_cmd_topic[60];
    snprintf(light_standby_cmd_topic,sizeof(light_standby_cmd_topic),"%s/light/%s/cmd/switch",HOSTNAME,"led_standby");
    mqttc.subscribe(light_standby_cmd_topic);
    char brightness_standby_cmd_topic[60];
    snprintf(brightness_standby_cmd_topic,sizeof(brightness_standby_cmd_topic),"%s/light/%s/cmd/brightness",HOSTNAME,"led_standby");
    mqttc.subscribe(brightness_standby_cmd_topic);

    #if (USE_MHZ19_CO2 == 1)
      char calibrate_cmd_topic[60];
      snprintf(calibrate_cmd_topic,sizeof(calibrate_cmd_topic),"%s/cmd/calibrate",HOSTNAME);
      mqttc.subscribe(calibrate_cmd_topic);
    #endif

    Serial.printf("[%s]: GW subscribed to:\n",__func__);
    {
      Serial.printf("\t%s\n",restart_cmd_topic);
      Serial.printf("\t%s\n",update_cmd_topic);
      Serial.printf("\t%s\n",publish_cmd_topic);
      Serial.printf("\t%s\n",reset_cmd_topic);
      Serial.printf("\t%s\n",command_for_sender_topic);
      Serial.printf("\t%s\n",reset_queue_cmd_topic);
      // enter LEDs here...
      #if (USE_MHZ19_CO2 == 1)
        Serial.printf("\t%s\n",calibrate_cmd_topic);
      #endif
    }

    long em = millis();
    long mt=em-sm;
    Serial.printf("[%s]: MQTT RECONNECT TIME: %dms\n",__func__,mt);
  }
}
