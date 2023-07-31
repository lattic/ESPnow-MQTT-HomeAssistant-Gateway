#pragma once

#if (USE_INA260 == 1)

  void measure_ina260(float *v1, float *i1, float *p1)
  {
    #ifdef DEBUG
        long function_start = micros(); 
    #endif // DEBUG
  
    if (!ina260_ok) 
    {
      Serial.printf("[%s]: INA260 NOT WORKING\n",__func__);
      *v1 = 0;
      *i1 = 0;
      *p1 = 0;
    } else 
    {
      *v1 = ina260.readBusVoltage()/1000;
      *i1 = ina260.readCurrent();  //  /1000;
      *p1 = ina260.readPower()/1000;
    }

     #ifdef DEBUG
      Serial.printf("[%s]: v1=%0.2fV, i1=%0.0fmA,p1=%0.0fmW \n",__func__,*v1,*i1,*p1);
      Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
    #endif // DEBUG
  }

#endif
