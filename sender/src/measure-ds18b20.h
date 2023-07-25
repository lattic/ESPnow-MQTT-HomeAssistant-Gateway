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
      Serial.printf("\n");

      return (name != NULL);
  }


  void get_all_ds18b20_addresses() //this function shows all addresses of the connected DS18B20 sensors
  { 
    Serial.printf("[%s]: ",__func__);
    
    DSTherm drv(_ow);
    Placeholder<DSTherm::Scratchpad> _scrpd;

    /* ...and read them one-by-one */
    for (const auto& id: (OneWireNg&)_ow) 
    {
      if (DSTherm::getFamilyName(id))
      {
        if (drv.readScratchpad(id, &_scrpd) == OneWireNg::EC_SUCCESS)
        {
          printId(id);
        }
        else 
        {
          Serial.printf("[%s]: Invalid CRC!\n",__func__);
        }
      }
    }
  }


  float get_ds18b20()
  {    
    float temp_float = -33.33;    // value for error
    
    //uncomment this function to shows all addresses of the connected DS18B20 sensors
    get_all_ds18b20_addresses();

    DSTherm drv(_ow);
    Placeholder<DSTherm::Scratchpad> _scrpd;
    DSTherm::Scratchpad& scrpd = _scrpd;

    drv.convertTempAll(DSTherm::SCAN_BUS, PARASITE_POWER);

    // put here the address of the sensor
    OneWireNg::Id id0 = {0x28,0xAD,0x16,0x75,0xD0,0x1,0x3C,0x1};

    if (drv.readScratchpad(id0, &_scrpd) == OneWireNg::EC_SUCCESS)
    {
      temp_float = (float) scrpd.getTemp()/1000;
    } 
    // else
    // {
    //   temp_float = (float) -33.33;
    // }

    #ifdef DEBUG
      Serial.printf("[%s]: Temperature=%0.2fC\n",__func__,temp_float);
    #endif

    return temp_float;
  }
#endif
