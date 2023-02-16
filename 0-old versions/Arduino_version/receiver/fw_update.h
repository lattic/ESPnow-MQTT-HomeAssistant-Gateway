#pragma once
/*
firmware update functions
*/
#include "config.h"
#include "variables.h"

// preparation
#if   (BOARD_TYPE == 1)
  #define FW_BIN_FILE "receiver.ino.esp32.bin"
#elif (BOARD_TYPE == 2)
  #define FW_BIN_FILE "receiver.ino.esp32s2.bin"
#elif (BOARD_TYPE == 3)
  #define FW_BIN_FILE "receiver.ino.esp32s3.bin"
#elif (BOARD_TYPE == 4)
  #define FW_BIN_FILE "receiver.ino.esp32c3.bin"
#else
  #error "FW update defined only for ESP32, ESP32-S2 and ESP32-S3 boards"
#endif

// functions

void do_update()
{
  if (!perform_update_firmware)
  {
    return;
  }
  bool publish_status_update_firmware = true;
  int update_firmware_status = -1;
  if (perform_update_firmware)
  {
    // Serial.printf("[%s]: unregistering espnow callback during FW update\n",__func__);
    // esp_now_unregister_recv_cb();
    Serial.printf("[%s]: unregistering espnow for FW update\n",__func__);
    esp_now_deinit();
    perform_update_firmware=false;
    publish_status_update_firmware=mqtt_publish_gw_status_values("Updating FW");
    long sm2 = millis(); while(millis() < sm2 + 1000) {};
    if (publish_status_update_firmware)
    {
      update_firmware_status=update_firmware_prepare();
      if (update_firmware_status == 0)
      {
        Serial.printf("[%s]: FW updated\n",__func__);
        mqtt_publish_gw_status_values("FW updated");
        sm2 = millis(); while(millis() < sm2 + 1000) {};

        Serial.printf("[%s]: RESTARTING in 3s...\n\n\n",__func__);
        mqtt_publish_gw_status_values("RESTARTING");
        sm2 = millis(); while(millis() < sm2 + 3000) {};

        ESP.restart();
      } else
      {
        Serial.printf("[%s]: FW update failed - reason: %d\n",__func__,update_firmware_status);
        mqtt_publish_gw_status_values("FW update failed");
        sm2 = millis(); while(millis() < sm2 + 1000) {};
        #ifdef STATUS_GW_LED_GPIO_RED
          digitalWrite(STATUS_GW_LED_GPIO_RED,LOW);
        #endif
        mqtt_publish_gw_status_values("online");
        sm2 = millis(); while(millis() < sm2 + 1000) {};
        ESP.restart();
      }
    }
  }
}


void updateFirmware(uint8_t *data, size_t len)
{
  #ifdef STATUS_GW_LED_GPIO_RED
    if (blink_led_status) {
      blink_led_status=LOW;
      digitalWrite(STATUS_GW_LED_GPIO_RED,blink_led_status);
    } else {
      blink_led_status=HIGH;
      digitalWrite(STATUS_GW_LED_GPIO_RED,blink_led_status);
    }
  #endif

  char update_progress_char[20];
  Update.write(data, len);
  fw_currentLength += len;
  old_update_progress=update_progress;

  update_progress=(fw_currentLength * 100) / fw_totalLength;
  if (update_progress>old_update_progress)
  {
    sprintf(update_progress_char, "FW upd: %d%%", update_progress);
    mqtt_publish_gw_status_values(update_progress_char);
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

  char firmware_file[255];
  snprintf(firmware_file,sizeof(firmware_file),"%s/01-Production/0-ESPnow/%s/%s",UPDATE_FIRMWARE_HOST,HOSTNAME,FW_BIN_FILE);

  fw_totalLength=0;
  fw_currentLength=0;
  Serial.printf("[%s]: uploading file: %s\n",__func__,firmware_file);
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
    // Serial.println("Updating firmware progress:");
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
