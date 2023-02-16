#pragma once
/*
espnow functions
*/

#include "config.h"
#include "variables.h"


void espnow_start()
{
  if (!wifi_connected) return;
  if (esp_now_init() != ESP_OK)
  {
    Serial.printf("[%s]: ESPNow NOT initialized, REBOTTING in 3s\n",__func__);
    delay(3000);
    ESP.restart();
  } else
  {
    Serial.printf("[%s]: ESPNow started - waiting for incoming messages...\n",__func__);
  }
  if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK)
  {
    Serial.printf("[%s]: registering callback failed...\n",__func__);
  } else
  {
    Serial.printf("[%s]: registering callback OK...\n",__func__);
  }
}


// OnDataRecv callback
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  int queue_count;
  if( queue_protect != NULL )
  {
    // lock both queues
    if( xSemaphoreTake( queue_protect, ( TickType_t ) 10 ) == pdTRUE )
    {
      queue_count = uxQueueMessagesWaiting(queue);
      if (queue_count < MAX_QUEUE_COUNT)
      {
        // push incoming data to myData
        memcpy(&myData, incomingData, sizeof(myData));
        // send to queue
        xQueueSend(queue, &myData, portMAX_DELAY);
        // push MAC to myData_aux (rssi, MAC)
        snprintf(myData_aux.macStr, sizeof(myData_aux.macStr), "%02x:%02x:%02x:%02x:%02x:%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        // send to queue_aux
        xQueueSend(queue_aux, &myData_aux, portMAX_DELAY);
        queue_count = uxQueueMessagesWaiting(queue);
        if (debug_mode) Serial.printf("[%s]: queue size=%d/%d, queued data from %s=%s, bat=%f\n",__func__,queue_count,MAX_QUEUE_COUNT,myData.host,myData_aux.macStr,myData.bat);
        Serial.printf("[%s]: queue size=%d/%d, queued data from %s\n",__func__,queue_count,MAX_QUEUE_COUNT,myData.host);
      } else
      {
        Serial.printf("[%s]: queue already FULL\n",__func__);
      }
      // update HA on queue status
      if (!publish_sensors_to_ha)
      {
        char queue_status[20];
        snprintf(queue_status, sizeof(queue_status), "queue: %d/%d",queue_count,MAX_QUEUE_COUNT);
        mqtt_publish_gw_last_updated_sensor_values(queue_status);
      }
      // unlock both queues
      xSemaphoreGive( queue_protect );
    }
    else
    {
      Serial.printf("[%s]: ERROR: semaphore not taken\n",__func__);
    }
  }
}
// OnDataRecv callback END
