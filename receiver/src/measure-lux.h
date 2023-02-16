#pragma once

void get_lux(char* lux)
{
  #ifdef DEBUG
    long sm = millis(); //start measurement time
  #endif
  size_t nbytes;
  // int temp_volts_raw=0;
  uint32_t temp_volts_raw;

  String volts[2]; // array with 2 strings: V, read_raw 
  measure_volt(MEASURE_LUX_GPIO, 11, 100, 1, false, volts);

  temp_volts_raw = (volts[1].toInt()* 1000) / LUX_MAX_RAW_READING;
  if (temp_volts_raw <= LUX_MIN_RAW_READING) temp_volts_raw = 0;

  nbytes = snprintf(NULL,0,"%d",temp_volts_raw) +1;
  snprintf(lux,nbytes,"%d",temp_volts_raw);

  #ifdef DEBUG
    long em = millis(); //END measurement time
    long mt=em-sm;
    Serial.print("LUX=");Serial.println(lux);
    Serial.print("RAW=");Serial.println(temp_volts_raw);
    Serial.println("LUX TIME:"+String(mt)+"ms");
  #endif

}


