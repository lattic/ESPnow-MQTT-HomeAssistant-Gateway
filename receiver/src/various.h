#pragma once
/*
various functions
*/

int compareArrays(uint8_t a[], uint8_t b[], uint8_t n)
{
  for (uint8_t i=0; i<n; ++i)
  {
      if (a[i] != b[i])
      {
          return 0;
      }
  }
  return 1;
}


void write_badbootcount(u_int8_t count)
{
  Preferences preferences2;
  Serial.printf("\n[%s]: writing badbootcount=%d\n",__func__,count);
  preferences2.begin("boot-pref", false); //write
  preferences2.putShort("badbootcount", count);
  preferences2.end();
  delay(1);
}

// gracefull restart of ESP
void do_esp_restart()
{
  write_badbootcount(0);
  ESP.restart();
}

void check_badbootcount()
{
  Preferences preferences2;
  Serial.printf("[%s]: reading badbootcount...\n",__func__);

  // read
  preferences2.begin("boot-pref", true); //read
  u_int8_t badbootcount = preferences2.getShort("badbootcount", 0);
  preferences2.end();  
  Serial.printf("[%s]: badbootcount=%d\n",__func__,badbootcount);
  ++badbootcount;

  // write incremented value
  write_badbootcount(badbootcount);

  Serial.printf("[%s]: new badbootcount=%d\n",__func__,badbootcount);

  if (badbootcount > MAX_BAD_BOOT_COUNT)
  {
    Serial.printf("[%s]: badbootcount (%d) above thershold (%d), erasing nvs...\n",__func__,badbootcount,MAX_BAD_BOOT_COUNT);
    erase_nvs(true);
  }
}

// used for gw
void ConvertSectoDay(unsigned long n, char *pretty_ontime)
{
  bool debug_mode = false;
  size_t nbytes;
  int day = n / (24 * 3600);
  n = n % (24 * 3600);
  int hour = n / 3600;
  n %= 3600;
  int minutes = n / 60 ;
  n %= 60;
  int seconds = n;
  if (day>0)
  {
    nbytes = snprintf(NULL,0,"%0dd %0dh %0dm %0ds",day,hour,minutes,seconds)+1;
    snprintf(pretty_ontime,nbytes,"%0dd %0dh %0dm %0ds",day,hour,minutes,seconds);
  } else
  {
    if (hour>0)
    {
      nbytes = snprintf(NULL,0,"%0dh %0dm %0ds",hour,minutes,seconds)+1;
      snprintf(pretty_ontime,nbytes,"%0dh %0dm %0ds",hour,minutes,seconds);
    } else
    {
      if (minutes>0)
      {
        nbytes = snprintf(NULL,0,"%0dm %0ds",minutes,seconds)+1;
        snprintf(pretty_ontime,nbytes,"%0dm %0ds",minutes,seconds);
      } else
      {
        nbytes = snprintf(NULL,0,"%0ds",seconds)+1;
        snprintf(pretty_ontime,nbytes,"%0ds",seconds);
      }
    }
  }
  if (debug_mode)
    Serial.printf("[%s]: pretty_ontime=%s\n",__func__,pretty_ontime);
}

// used for sensors
void uptime(char *uptime)
{
  byte days = 0;
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;

  unsigned long sec2minutes = 60;
  unsigned long sec2hours = (sec2minutes * 60);
  unsigned long sec2days = (sec2hours * 24);

  unsigned long time_delta = (millis()) / 1000UL;

  days = (int)(time_delta / sec2days);
  hours = (int)((time_delta - days * sec2days) / sec2hours);
  minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
  seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

  if ((minutes == 0) and (hours == 0) and (days == 0)){
    sprintf(uptime, "%01ds", seconds);
  }
  else {
    if ((hours == 0) and (days == 0))
    {
      sprintf(uptime, "%01dm %01ds", minutes, seconds);
    }
    else
    {
      if (days == 0)
      {
        sprintf(uptime, "%01dh %01dm", hours, minutes);
      } else
      {
          sprintf(uptime, "%01dd %01dh %01dm", days, hours, minutes);
      }
    }
  }
  if (debug_mode)
    Serial.printf("[%s]: uptime=%s\n",__func__,uptime);

}

String mac_to_string(uint8_t *addr)
{
  String mac_str = String(addr[0], HEX) + ":" + String(addr[1], HEX) + ":" + String(addr[2], HEX) + ":"
    + String(addr[3], HEX) + ":" + String(addr[4], HEX) + ":" + String(addr[5], HEX);
  mac_str.toUpperCase();
  return mac_str;
}

#ifdef PUSH_BUTTON_GPIO
// button functions
void OnClicked_cb()
{
  long sm2;
  uint32_t pushed_ms = b->getPushTime();
  char message[255];
  snprintf(message,sizeof(message),"%dms click",pushed_ms);
  if (pushed_ms < 3000) // < 3s
  {
    Serial.printf("[%s]: click (%ums)->doing nothing\n",__func__,pushed_ms);
    mqtt_publish_gw_status_values(message);
  } 
  else
  if (pushed_ms < 6000) // 3s < x < 6s
  {
    Serial.printf("[%s]: click (%ums)->RESTARTING\n",__func__,pushed_ms);
    mqtt_publish_gw_status_values(message);
    do_esp_restart();
  } 
  else               
  if (pushed_ms < 9000) // 6s < x < 9s
  {
    Serial.printf("[%s]: click (%ums)->FW update\n",__func__,pushed_ms);
    mqtt_publish_gw_status_values(message);
    perform_update_firmware = true;
  } else                 // x > 9s
  {
    Serial.printf("[%s]: click (%ums)->erase NVS\n",__func__,pushed_ms);
    mqtt_publish_gw_status_values(message);
    erase_nvs(true);
    do_esp_restart();
  }
  mqtt_publish_gw_status_values("online");
}


// void OnDoubleClicked_cb()
// {
//   // getPushTime for doubleclick is the time when double click happened - not timing of click
//   long sm2;
//   uint32_t pushed_ms = b->getPushTime();
//   Serial.printf("[%s]: button DoubleClicked at %ums\n",__func__,pushed_ms);
//   mqtt_publish_gw_status_values("DoubleClick");
//   sm2 = millis(); while(millis() < sm2 + 300) {};
//   mqtt_publish_gw_status_values("online");
// }


// void OnHolding_cb()
// {
//   // interval for onholding is called every interval time, for for 3s it calls every 3s
//   long sm2;
//   uint32_t pushed_ms = b->getPushTime();
//   Serial.printf("[%s]: button holding for %ums\n",__func__,pushed_ms);
//   mqtt_publish_gw_status_values("Held");
//   sm2 = millis(); while(millis() < sm2 + 100) {};

//   if ((pushed_ms > 1000) and (pushed_ms < 3000))  // < 3s
//   {
//     Serial.printf("[%s]: FW update\n",__func__,pushed_ms);
//     // perform_update_firmware = true;
//   } else 
//   if ((pushed_ms > 3000) and (pushed_ms < 10000) and (b->released())) //  3s < x < 10s
//   {
//     Serial.printf("[%s]: RESTARTING\n",__func__,pushed_ms);
//     mqtt_publish_gw_status_values("RESTARTING");
//     sm2 = millis(); while(millis() < sm2 + 500) {};
//     // ESP.restart();
//   } else                // x > 10s 
//   {
//     // something else here
//   }
// }

#endif


// LED
void initiate_all_leds()
{
  #ifdef LED_GPIO_SENSORS
    // PWM
    #if (LED_GPIO_SENSORS_USE_PWM == 1)
      ledcSetup(LED_GPIO_SENSORS_PWM_CHANNEL, LED_GPIO_SENSORS_PWM_FREQ, LED_GPIO_SENSORS_PWM_RESOLUTION);
      ledcAttachPin(LED_GPIO_SENSORS, LED_GPIO_SENSORS_PWM_CHANNEL);
      led_gpio_sensors_dc = LED_GPIO_SENSORS_PWM_DC;
      prev_led_gpio_sensors_dc = led_gpio_sensors_dc;
      led_sensors_power = true;
    // or fixed
    #else
      pinMode(LED_GPIO_SENSORS, OUTPUT);
    #endif
  #endif

  #ifdef LED_GPIO_GATEWAY
    // PWM
    #if (LED_GPIO_GATEWAY_USE_PWM == 1)
      ledcSetup(LED_GPIO_GATEWAY_PWM_CHANNEL, LED_GPIO_GATEWAY_PWM_FREQ, LED_GPIO_GATEWAY_PWM_RESOLUTION);
      ledcAttachPin(LED_GPIO_GATEWAY, LED_GPIO_GATEWAY_PWM_CHANNEL);
      led_gpio_gateway_dc = LED_GPIO_GATEWAY_PWM_DC;
      prev_led_gpio_gateway_dc = led_gpio_gateway_dc;
      led_gateway_power = true;
    // or fixed
    #else
      pinMode(LED_GPIO_GATEWAY, OUTPUT);
    #endif
  #endif 

#ifdef LED_GPIO_STANDBY
    // PWM
    #if (LED_GPIO_STANDBY_USE_PWM == 1)
      ledcSetup(LED_GPIO_STANDBY_PWM_CHANNEL, LED_GPIO_STANDBY_PWM_FREQ, LED_GPIO_STANDBY_PWM_RESOLUTION);
      ledcAttachPin(LED_GPIO_STANDBY, LED_GPIO_STANDBY_PWM_CHANNEL);
      led_gpio_standby_dc = LED_GPIO_STANDBY_PWM_DC;
      prev_led_gpio_standby_dc = led_gpio_standby_dc;
      led_standby_power = true;
    // or fixed
    #else
      pinMode(LED_GPIO_STANDBY, OUTPUT);
    #endif
  #endif    
}

void set_sensors_led_level(u_int8_t level)
{
  #ifdef LED_GPIO_SENSORS
    #if (LED_GPIO_SENSORS_USE_PWM == 1)
      if (level == 1)
      {
          ledcWrite(LED_GPIO_SENSORS_PWM_CHANNEL, led_gpio_sensors_dc);
      } else 
      {
        ledcWrite(LED_GPIO_SENSORS_PWM_CHANNEL, 0);
      }
    #else 
      if (level == 1)
      {
          digitalWrite(LED_GPIO_SENSORS,HIGH);
      } else 
      {
        digitalWrite(LED_GPIO_SENSORS,LOW);
      }
    #endif
  #endif
}

void set_gateway_led_level(u_int8_t level)
{
  #ifdef LED_GPIO_GATEWAY
    #if (LED_GPIO_GATEWAY_USE_PWM == 1)
      if (level == 1)
      {
          ledcWrite(LED_GPIO_GATEWAY_PWM_CHANNEL, led_gpio_gateway_dc);
      } else 
      {
        ledcWrite(LED_GPIO_GATEWAY_PWM_CHANNEL, 0);
      }
    #else 
      if (level == 1)
      {
          digitalWrite(LED_GPIO_GATEWAY,HIGH);
      } else 
      {
        digitalWrite(LED_GPIO_GATEWAY,LOW);
      }
    #endif
  #endif
}

void set_standby_led_level(u_int8_t level)
{
  #ifdef LED_GPIO_STANDBY
    #if (LED_GPIO_STANDBY_USE_PWM == 1)
      if (level == 1)
      {
          ledcWrite(LED_GPIO_STANDBY_PWM_CHANNEL, led_gpio_standby_dc);
      } else 
      {
        ledcWrite(LED_GPIO_STANDBY_PWM_CHANNEL, 0);
      }
    #else 
      if (level == 1)
      {
          digitalWrite(LED_GPIO_STANDBY,HIGH);
      } else 
      {
        digitalWrite(LED_GPIO_STANDBY,LOW);
      }
    #endif
  #endif
}