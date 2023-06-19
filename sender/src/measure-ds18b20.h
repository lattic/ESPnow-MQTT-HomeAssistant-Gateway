#pragma once

#if (USE_DALLAS_18B20 == 1)

  static bool printId(const OneWireNg::Id& id)      //prints addresses of connected DS18B20 sensors
  {
      const char *name = DSTherm::getFamilyName(id);

      Serial.print(id[0], HEX);
      for (size_t i = 1; i < sizeof(OneWireNg::Id); i++) {
          Serial.print(':');
          Serial.print(id[i], HEX);
      }
      if (name) {
          Serial.print(" -> ");
          Serial.print(name);
      }
      Serial.println();

      return (name != NULL);
  }


  void get_all_ds18b20_addresses(){ //this function shows all addresses of the connected DS18B20 sensors
    long sm = millis();

    Serial.println("get_ds18b20_address:");
    DSTherm drv(_ow);
    Placeholder<DSTherm::Scratchpad> _scrpd;

    /* ...and read them one-by-one */
    for (const auto& id: (OneWireNg&)_ow) {
      if (DSTherm::getFamilyName(id)){
        if (drv.readScratchpad(id, &_scrpd) == OneWireNg::EC_SUCCESS){
          printId(id);
        }
        else {
          Serial.println("  Invalid CRC!");
        }
      }
    }
    Serial.println("get_all_ds18b20_addresses TOOK: "+String(millis()-sm)+"ms");
  }


  // void get_ds18b20(long temps[]){ //in millidegrees
  float get_ds18b20()
  {    
    float temp_float;
    // #ifdef DEBUG
      unsigned long function_start = millis(); 
    // #endif
    //uncomment this function to shows all addresses of the connected DS18B20 sensors
    // get_all_ds18b20_addresses();

    DSTherm drv(_ow);
    Placeholder<DSTherm::Scratchpad> _scrpd;
    DSTherm::Scratchpad& scrpd = _scrpd;

    drv.convertTempAll(DSTherm::SCAN_BUS, PARASITE_POWER);

    // put here the address of the sensor
    OneWireNg::Id id0 = {0x28,0x30,0x22,0x75,0xD0,0x1,0x3C,0x61}; 

    if (drv.readScratchpad(id0, &_scrpd) == OneWireNg::EC_SUCCESS)
    {
      temp_float = (float) scrpd.getTemp()/1000;
    } else
    {
      temp_float = (float) -33.33;
    }
    
    // Serial.printf("[%s]: Temperature=%0.2fC\n",__func__,temp_float);

    #ifdef DEBUG
      Serial.printf("[%s]: Temperature=%0.2fC\n",__func__,temp_float);
      Serial.printf("[%s]: took %ums\n",__func__,(millis()-function_start));
    #endif

    return temp_float;
  }
#endif
