#pragma once

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

void LoRaonReceive(int packetSize) 
{   
    int queue_count;
    int mess_size;
    uint8_t received_address[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if( queue_protect != NULL )
    {
        if( xSemaphoreTake( queue_protect, ( TickType_t ) 10 ) == pdTRUE )
        {
            queue_count = uxQueueMessagesWaiting(queue);
            if (queue_count < MAX_QUEUE_COUNT)
            {
                size_t addr_size = LoRa.readBytes((uint8_t *)&received_address, sizeof(received_address));
                if (!compareArrays(received_address,FixedMACAddress,6)) 
                {
                    Serial.println("this message is NOT for me");
                    #ifdef DEBUG
                        Serial.print("received address: ");
                        for (int i=0; i<6;i++)
                        {
                            Serial.print(received_address[i],HEX);Serial.print(" ");
                        }
                        Serial.println();

                        Serial.print("my address:       ");
                        for (int i=0; i<6;i++)
                        {
                            Serial.print(FixedMACAddress[i],HEX);Serial.print(" ");
                        }
                        Serial.println("\n---------------------");Serial.println();
                    #endif
                    xSemaphoreGive( queue_protect );
                    return;
                } 
                while (LoRa.available()) 
                {
                    // push incoming data to myData
                    mess_size = LoRa.readBytes((uint8_t *)&myData, sizeof(myData));
                }
                // send to queue
                xQueueSend(queue, &myData, portMAX_DELAY);
                // in case of LoRa myData_aux is not needed as: RSSI is part of the code and MAC (since nowhere) is now sent by myData.macStr
                // but it stays for compatibility
                myData_aux.rssi = LoRa.packetRssi();
                snprintf(myData_aux.macStr, sizeof(myData_aux.macStr), "%s",myData.macStr);
                // new variable
                snprintf(myData_aux.comm_type, sizeof(myData_aux.comm_type), "LoRa");
                xQueueSend(queue_aux, &myData_aux, portMAX_DELAY);
            }
            /*
            removed from here, it is in main loop()
                        // update HA on queue status
                        // if (!publish_sensors_to_ha)
                        // {
                        //   char queue_status[20];
                        //   snprintf(queue_status, sizeof(queue_status), "queue: %d/%d",queue_count,MAX_QUEUE_COUNT);
                        //   mqtt_publish_gw_last_updated_sensor_values(queue_status);
                        // }
                        // unlock both queues
            */
            xSemaphoreGive( queue_protect );
            message_received = 1;
        } else
        {
            Serial.printf("[%s]: ERROR: semaphore not taken\n",__func__);
        }
    }
}
bool start_lora()
{
    SPI.begin(LORA_GPIO_CLOCK, LORA_GPIO_MISO, LORA_GPIO_MOSI);
    LoRa.setPins(LORA_GPIO_SS, LORA_GPIO_RST, LORA_GPIO_DIO0);
    if (!LoRa.begin(866E6)) {
        Serial.printf("[%s]: LoRa FAILED to initialise\n",__func__);
        return 0;
    }
    LoRa.setTxPower(20);
    LoRa.enableCrc();

    // register the receive callback
    LoRa.onReceive(LoRaonReceive);

    // put the radio into receive mode
    LoRa.receive();

    return 1;
}

void end_lora()
{
    LoRa.end();
}