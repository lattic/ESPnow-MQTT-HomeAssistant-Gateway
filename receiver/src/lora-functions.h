#pragma once


void LoRaonReceive(int packetSize) 
{   
    // rather DON'T print anything here in this function as it can panic on wdt, especially with Serial.printf()
    #ifdef DEBUG
        Serial.println("[LoRaonReceive]: Message received");
    #endif
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
                    Serial.println("[LoRaonReceive]: this message is NOT for me");
                    #ifdef DEBUG
                        Serial.println("this message is NOT for me");
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
    
    // check if working SF=6 - not working, requires implicit header and packet size known, default is 7
    // LoRa.setSpreadingFactor(12); // 5 seconds!
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