#pragma once
/*
firmware update functions
*/

// functions

void do_update()
{
  if (!perform_update_firmware)
  {
    return;
  }
  fw_update_timer_start();
  bool publish_status_update_firmware = true;
  int update_firmware_status = -1;
  if (perform_update_firmware)
  {
    // Serial.printf("[%s]: unregistering espnow callback during FW update\n",__func__);
    // esp_now_unregister_recv_cb();
    Serial.printf("[%s]: unregistering espnow for FW update\n",__func__);
    esp_now_deinit();
    perform_update_firmware=false;
    if (!skip_mqtt) publish_status_update_firmware=mqtt_publish_gw_status_values("Updating FW"); else publish_status_update_firmware = true;
    #if (USE_WEB_SERIAL == 1)
      WebSerial.println("Updating FW started...");
    #endif
    long sm2 = millis(); while(millis() < sm2 + 300) {};
    if (publish_status_update_firmware)
    {
      update_firmware_status=update_firmware_prepare();
      if (update_firmware_status == 0)
      {
        Serial.printf("[%s]: FW updated\n",__func__);
        if (!skip_mqtt) mqtt_publish_gw_status_values("FW updated");
        #if (USE_WEB_SERIAL == 1)
          WebSerial.println("Updating FW DONE");
        #endif
        sm2 = millis(); while(millis() < sm2 + 300) {};

        Serial.printf("[%s]: RESTARTING...\n\n\n",__func__);
        if (!skip_mqtt) mqtt_publish_gw_status_values("RESTARTING");
        #if (USE_WEB_SERIAL == 1)
          WebSerial.println("RESTARTING");
        #endif
        sm2 = millis(); while(millis() < sm2 + 300) {};
        do_esp_restart();
      } else
      {
        Serial.printf("[%s]: FW update failed - reason: %d\n",__func__,update_firmware_status);
        if (!skip_mqtt) mqtt_publish_gw_status_values("FW update failed");
        #if (USE_WEB_SERIAL == 1)
          WebSerial.println("Updating FW FAILED");
        #endif
        sm2 = millis(); while(millis() < sm2 + 300) {};
        set_gateway_led_level(0);
        Serial.printf("[%s]: RESTARTING...\n\n\n",__func__);
        #if (USE_WEB_SERIAL == 1)
          WebSerial.println("RESTARTING");
        #endif
        if (!skip_mqtt) mqtt_publish_gw_status_values("RESTARTING");
        sm2 = millis(); while(millis() < sm2 + 300) {};
        do_esp_restart();
      }
    }
  }
}


void updateFirmware(uint8_t *data, size_t len)
{

  if (blink_led_status) {
    blink_led_status=LOW;
    set_gateway_led_level(0);
  } else {
    blink_led_status=HIGH;
    set_gateway_led_level(1);
  }


  char update_progress_char[20];
  Update.write(data, len);
  fw_currentLength += len;
  old_update_progress=update_progress;

  update_progress=(fw_currentLength * 100) / fw_totalLength;
  if (update_progress>old_update_progress)
  {
    sprintf(update_progress_char, "FW upd: %d%%", update_progress);
    #if (USE_WEB_SERIAL == 1)
      WebSerial.println(update_progress_char);
    #endif
    if (!skip_mqtt) mqtt_publish_gw_status_values(update_progress_char);
    if (update_progress % 5 == 0)
    {
      Serial.printf("[%s]: FW update: %d%%\n",__func__,update_progress);
    }
  }

  // if current length of written firmware is not equal to total firmware size, repeat
  if(fw_currentLength != fw_totalLength)
  {
    return;
  }
  Update.end(true);
  Serial.printf("\n[%s]: Firmware Upload DONE, Total Size: %dbytes\n",__func__,fw_currentLength);
}


int update_firmware_prepare()
{
  long start_upgrade_time = millis();

  // char firmware_file[255];
  //   // 01-with_mqtt_discovery/01-Production/0-ESPnow/firmware/esp32027.bin
  // snprintf(firmware_file,sizeof(firmware_file),"%s/01-Production/0-ESPnow/firmware/%s.bin",UPDATE_FIRMWARE_HOST,HOSTNAME);

  char firmware_file[255];
  #ifndef CLIENT
    // UPDATE_FIRMWARE_HOST is in local network
    snprintf(firmware_file,sizeof(firmware_file),"%s/01-Production/0-ESPnow/firmware/%s.bin",UPDATE_FIRMWARE_HOST,HOSTNAME);
  #else
    // UPDATE_FIRMWARE_HOST is on internet
    snprintf(firmware_file,sizeof(firmware_file),"%s/%s.bin",UPDATE_FIRMWARE_HOST,HOSTNAME);
  #endif  

  fw_totalLength=0;
  fw_currentLength=0;
  Serial.printf("[%s]: uploading file: %s\n",__func__,firmware_file);
  #if (USE_WEB_SERIAL == 1)
    WebSerial.print("Uploading file: ");
    WebSerial.println(firmware_file);
  #endif
  firmware_update_client.begin(firmware_file);
  int resp = firmware_update_client.GET();
  Serial.printf("[%s]: Response: %d\n",__func__,resp);
  // If file is reachable, start downloading
  if(resp == 200)
  {
    // get length of document (is -1 when Server sends no Content-Length header)
    fw_totalLength = firmware_update_client.getSize();
    // transfer to local variable
    int len = fw_totalLength;
    // this is required to start firmware update process
    Update.begin(UPDATE_SIZE_UNKNOWN);
    Serial.printf("[%s]: FW File Size: %d bytes\n",__func__,fw_totalLength);
    // create buffer for read
    uint8_t buff[128] = { 0 };
    // get tcp stream
    WiFiClient * stream = firmware_update_client.getStreamPtr();
    // read all data from server
    Serial.printf("[%s]: Updating firmware progress:\n",__func__);
    while(firmware_update_client.connected() && (len > 0 || len == -1))
    {
       // get available data size
       size_t size = stream->available();
       if(size)
       {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          // pass to function
          updateFirmware(buff, c);
          if(len > 0)
          {
             len -= c;
          }
       }
       delay(1);
    }
  } else
  {
    Serial.printf("[%s]: Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in UPDATE_FIRMWARE_FILE.\n",__func__);
    Serial.printf("[%s]: UNSUCESSFUL - time: %lums\n",__func__,millis()-start_upgrade_time);
    return resp;
  }
  firmware_update_client.end();
  Serial.printf("[%s]: SUCESSFUL - time: %lums\n",__func__,millis()-start_upgrade_time);
  return 0;
}



void cb_fw_update_timer(TimerHandle_t xTimer )
{
  #if (USE_WEB_SERIAL == 1)
    WebSerial.println("FW UPDATE timer expired - restarting");
  #endif
  ESP.restart();
}

bool fw_update_timer_start()
{
  // create  timer if not yet created
  if (fw_update_timer_handle  == NULL)
  {
    fw_update_timer_handle = xTimerCreate("FW update timer", pdMS_TO_TICKS(FW_UPDATE_TIME_S * 1000), pdFALSE, ( void * ) 0, cb_fw_update_timer);
    if( xTimerStart(fw_update_timer_handle, 10 ) != pdPASS )
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer start error\n",__func__);
        #endif
        return false;
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: timer STARTED with update interval=%ds\n",__func__,FW_UPDATE_TIME_S);
      #endif
        #if (USE_WEB_SERIAL == 1)
          WebSerial.print("FW UPDATE timer started: ");
          WebSerial.print(FW_UPDATE_TIME_S);
          WebSerial.println(" seconds");
        #endif
      return true;
    }
  } else
  //  timer created so restart it
  {
    if( xTimerReset( fw_update_timer_handle, 0 ) != pdPASS )
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer was not reset\n",__func__);
        #endif
        return false;
    } else
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer RE-STARTED\n",__func__);
        #endif
        return true;
    }
  }
}

bool fw_update_timer_stop()           // not in use
{
    if( xTimerStop( fw_update_timer_handle, 0 ) != pdPASS )
    {
        #ifdef DEBUG  
            Serial.printf("[%s]: timer was NOT stopped\n",__func__);
        #endif
        return false;
    } else
    {
        #ifdef DEBUG
            Serial.printf("[%s]: timer stopped\n",__func__);
        #endif
        return true;
    }
}
