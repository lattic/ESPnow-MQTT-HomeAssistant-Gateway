#pragma once


void measure_temp_pressure(char *pressure, char *temperature)   // 2ms
{
  // temperature sensor from BM280 is not used as it is shifted by around 0.5C vs SHT31
  
  #if (USE_BMP280 == 1)
    size_t nbytes;
    #ifdef DEBUG
        unsigned long function_start = millis(); 
    #endif
    
    // float temp = bmp.readTemperature();
    float press =((bmp.readPressure() + 900) / 10 / 10) + (AIR_PRESSURE_CALIBRATION);
    float temp  = bmp.readTemperature() + (BMP280_TEMPERATURE_CALIBRATION);
    // Serial.printf("[%s]: pressure float=%fhPa, callibrated pressure=%fhPa\n",__func__,(press-(AIR_PRESSURE_CALIBRATION)),press);

    // nbytes = snprintf(NULL,0,"%0.2f",temp) +1;
    // snprintf(temperature,nbytes,"%0.2f",temp);

    nbytes = snprintf(NULL,0,"%0.1f",press) +1;
    snprintf(pressure,nbytes,"%0.1f",press);

    nbytes = snprintf(NULL,0,"%0.1f",temp) +1;
    snprintf(temperature,nbytes,"%0.2f",temperature);


    #ifdef DEBUG
        Serial.printf("[%s]: pressure=%shPa, temperature=%sC\n",__func__,pressure,temperature);
    #endif
  #endif
  
  #ifdef DEBUG
      Serial.printf("[%s]: took %ums\n",__func__,(millis()-function_start));
  #endif
}