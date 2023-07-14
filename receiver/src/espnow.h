#pragma once
/*
espnow functions
*/

// #include "config.h"
// #include "variables.h"


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
    Serial.printf("[%s]: esp_now_register_recv_cb failed...\n",__func__);
  } else
  {
    Serial.printf("[%s]: esp_now_register_recv_cb OK...\n",__func__);
  }

  if (esp_now_register_send_cb(OnDataSent) != ESP_OK)
  {
    Serial.printf("[%s]: esp_now_register_send_cb failed...\n",__func__);
  } else
  {
    Serial.printf("[%s]: esp_now_register_send_cb OK...\n",__func__);
  }
}


// OnDataRecv callback
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  int queue_count;
  u_int8_t command;
  if( queue_protect != NULL )
  {
    // lock queue with data from sensors
    if( xSemaphoreTake( queue_protect, ( TickType_t ) 10 ) == pdTRUE )
    {
      // sending command to sender
      #ifdef DEBUG
        Serial.println("\n******************* SENDING COMMAND ******************************");
      #endif
      memcpy(temp_address, mac, 6);

      char incoming_mac[18];
      snprintf(incoming_mac, sizeof(incoming_mac), "%02x:%02x:%02x:%02x:%02x:%02x",temp_address[0],temp_address[1],temp_address[2],temp_address[3],temp_address[4],temp_address[5]);
      #ifdef DEBUG
        Serial.printf("[%s]: incoming MAC: %s\n",__func__,incoming_mac);
      #endif

      u_int8_t queue_commands_count = uxQueueMessagesWaiting(queue_commands);
      #ifdef DEBUG
        Serial.printf("[%s]: number of ALL commands in the queue=%d\n",__func__,queue_commands_count);
      #endif
      if (queue_commands_count > 0)
      {
        struct_commands commands_local;
        // lock queue with commands from sensors
        if( xSemaphoreTake( queue_commands_protect, ( TickType_t ) 10 ) == pdTRUE )
        {
          for (u_int8_t i=0;i<queue_commands_count;i++)
          {
            if (xQueueReceive(queue_commands, &commands_local, ( TickType_t ) 10 ) == pdPASS ) 
            {
              #ifdef DEBUG
                Serial.printf("[%s]: in the queue: command: %d, MAC=%s, COMMAND=%d\n",__func__,i,commands_local.mac,commands_local.command);
              #endif
              if (strcmp(incoming_mac,commands_local.mac) == 0)
              {
                #ifdef DEBUG
                  Serial.printf("[%s]: macs the same - command received, sending to sender command=%d\n",__func__,commands_local.command);
                #endif
                send_command_to_sender(commands_local.command);
                xSemaphoreGive( queue_commands_protect );
                break;
              } else 
              {
                #ifdef DEBUG
                  Serial.println("\tmacs NOT the same - put back the command n the queue");
                #endif
                xQueueSend(queue_commands, &commands_local, portMAX_DELAY);
              }
            }
          }
          xSemaphoreGive( queue_commands_protect );
        }
      } else 
      {
        command = 0;
      }

      #ifdef DEBUG
        Serial.printf("[%s]: sending command: %d to MAC: %s\n",__func__,command,incoming_mac);
      #endif
      send_command_to_sender(command);

      #ifdef DEBUG
        queue_commands_count = uxQueueMessagesWaiting(queue_commands);
        Serial.printf("[%s]: number of LEFT commands in the queue=%d\n",__func__,queue_commands_count);
      #endif

      #ifdef DEBUG
        Serial.println("******************* SENDING COMMAND END **************************\n");
      #endif
      // sending command to sender END

      queue_count = uxQueueMessagesWaiting(queue);
      if (queue_count < MAX_QUEUE_COUNT)
      {
        // push incoming data to myData
        memcpy(&myData, incomingData, sizeof(myData));
        
        // new variable
        snprintf(myData_aux.comm_type, sizeof(myData_aux.comm_type), "ESPnow");

        // push MAC to myData_aux (rssi, MAC)
        snprintf(myData_aux.macStr, sizeof(myData_aux.macStr), "%02x:%02x:%02x:%02x:%02x:%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        if (strcmp(myData.macStr, myData_aux.macStr) != 0)
        {
          Serial.printf("[%s]: MACs are DIFFERENT, NOT PUBLISHING!\n",__func__);
          xSemaphoreGive( queue_protect );
          return;
        } 

        // send to queue
        xQueueSend(queue, &myData, portMAX_DELAY);

        // send to queue_aux
        xQueueSend(queue_aux, &myData_aux, portMAX_DELAY);
        queue_count = uxQueueMessagesWaiting(queue);
        if (debug_mode) Serial.printf("[%s]: queue size=%d/%d, queued data from %s=%s, bat=%f\n",__func__,queue_count,MAX_QUEUE_COUNT,myData.host,myData_aux.macStr,myData.bat);
        #ifdef DEBUG
          Serial.printf("[%s]: queue size=%d/%d, queued data from %s\n",__func__,queue_count,MAX_QUEUE_COUNT,myData.host);
        #endif
      } else
      {
        Serial.printf("[%s]: queue already FULL\n",__func__);
      }
      xSemaphoreGive( queue_protect );
      message_received = 1;
    }
    else
    {
      Serial.printf("[%s]: ERROR: semaphore not taken\n",__func__);
    }
  }
}
// OnDataRecv callback END

// on sent callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  // char outgoing_mac[18];
  // snprintf(outgoing_mac, sizeof(outgoing_mac), "%02x:%02x:%02x:%02x:%02x:%02x",mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);

  // if (status == ESP_NOW_SEND_SUCCESS)
  // {
  //   #ifdef DEBUG
  //   Serial.printf("[%s]: Command to sender sent %s\n",__func__,outgoing_mac);
  //   #endif
  // } else 
  // {
  //   Serial.printf("[%s]: Command to sender NOT sent %s\n",__func__,outgoing_mac);
  // }
}

// send command to sender
bool send_command_to_sender(u_int8_t command)
{
  // if( xSemaphoreTake( queue_commands_protect, ( TickType_t ) 10 ) == pdTRUE )
  // {
    memcpy(peerInfo.peer_addr, temp_address, 6);
    peerInfo.channel = WiFi.channel();  
    peerInfo.encrypt = false;
        
    if(esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.printf("[%s]: FAILED to add peer\n",__func__);
      memset(temp_address, 0, 6);
      // xSemaphoreGive( queue_commands_protect );
      return false;
    } 
    // data_answer.command = random(10);
    data_answer.command = command; // do nothing on sender side

    esp_now_send(temp_address, (uint8_t *) &data_answer, sizeof(data_answer));
    if(esp_now_del_peer(temp_address) != ESP_OK)
    {
      Serial.printf("[%s]: FAILED to delete peer\n",__func__);
      memset(temp_address, 0, 6);
      // xSemaphoreGive( queue_commands_protect );
      return false;
    } 
  
    // memcpy(last_recv_address, temp_address, 6); // do I need last_recv_address?
    memset(temp_address, 0, 6);
    // xSemaphoreGive( queue_commands_protect );
    return true;
  // } 
  // else 
  // {
  //   Serial.printf("[%s]: FAILED to lock the queue_commands \n",__func__);
  //   return false;
  // }
}

