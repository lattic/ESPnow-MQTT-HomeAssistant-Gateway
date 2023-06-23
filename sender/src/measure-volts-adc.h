#pragma once 

#if (USE_ADC == 1)

  double get_volts(int pin, int attennuation, int iteration, float resistors_calibration) 
  {
    long sm = millis(); 
    int i = 0;
    uint32_t read_digital = 0;
    float milivolts = 0;
    float volts = 0.0;
    /*
    best measurements ranges - for accuracy
    ESP32:
    ADC_ATTEN_DB_0:   100 mV ~ 950 mV
    ADC_ATTEN_DB_2_5: 100 mV ~ 1250 mV
    ADC_ATTEN_DB_6:   150 mV ~ 1750 mV
    ADC_ATTEN_DB_11:  150 mV ~ 2450 mV
    ESP32S2:
    ADC_ATTEN_DB_0:   0 mV ~ 750 mV
    ADC_ATTEN_DB_2_5: 0 mV ~ 1050 mV
    ADC_ATTEN_DB_6:   0 mV ~ 1300 mV
    ADC_ATTEN_DB_11:  0 mV ~ 2500 mV
    */

    static adc_attenuation_t atten32;

    switch (attennuation){
      case 0:
        atten32 = ADC_0db;
        break;
      case 2:
        atten32 = ADC_2_5db;
        break;
      case 6:
        atten32 = ADC_6db;
        break;
      case 11:
        atten32 = ADC_11db;
        break;
      default:
        atten32 = ADC_11db;
    }

    #ifdef DEBUG
        Serial.printf("[%s]: attenuation=%d, atten32=%d, gpio=%d, adc_bits=%d\n",__func__,attennuation,atten32,pin,ADC_BITS);
    #endif
    analogSetPinAttenuation(pin, atten32);
    adcAttachPin(pin);
    analogReadResolution(ADC_BITS);

    // analogSetAttenuation(ADC_0db);

    for ( i = 0; i < iteration; i++) {
        read_digital += analogRead(pin);
    }
    read_digital /= iteration; //digital averaged reading
    #ifdef DEBUG
        Serial.printf("[%s]: battery digital=%d\n",__func__,read_digital);
    #endif

    milivolts = ((float)read_digital / pow(2,ADC_BITS)) * ADC_CALIBRATION * 1000;

    volts = (resistors_calibration *  milivolts) /1000 ; 

    long em = millis(); //END measurement time
    long mt=em-sm;

    #ifdef DEBUG
        Serial.printf("[%s]: battery millivolts=%f, battery volts=%0.2f\n",__func__,milivolts, volts);
        Serial.println("VOLT MEASUREMENT TIME:"+String(mt)+"ms");
    #endif

    return volts;
  }


  uint8_t get_bat_pcnt (double volts)
  {
    uint8_t b_perc;
    float arr_volts[21] = 
    // {
    //     4.2, 4.15, 4.11, 4.08, 4.02, 3.98, 3.95, 3.91, 3.87, 3.85, 3.84, 3.82, 3.8, 3.79, 3.77, 3.75, 3.73, 3.71, 3.69, 3.61, 3.27 
    // };
    {
        3.27, 3.61, 3.69, 3.71, 3.73, 3.75, 3.77, 3.79, 3.8, 3.82, 3.84, 3.85, 3.87, 3.91, 3.95, 3.98, 4.02, 4.08, 4.11, 4.15, 4.2
    };

    if (volts <= arr_volts[0]) 
    {
        b_perc = 0;
        Serial.printf("[%s]: Battery percent=%d, battery volts=%0.2f\n",__func__,b_perc,volts);
        return b_perc;
    } 
    if (volts >= arr_volts[20]) 
    { 
        b_perc = 100;
        #ifdef DEBUG
            Serial.printf("[%s]: 100-Battery percent=%d, battery volts=%0.2f\n",__func__,b_perc,volts);
        #endif
        return b_perc;
    }

    for (int i=0; i<20; i++)
    {
        if ((volts>=arr_volts[i]) and (volts<arr_volts[i+1])) 
        {
            b_perc = (i * 5);
            #ifdef DEBUG
                Serial.printf("[%s]: i=%d:Battery percent=%d, battery volts=%0.2f\n",__func__,i,b_perc,volts);
            #endif
            return b_perc;
        }
    }

    Serial.printf("[%s]: SOMETHING WENT WRONG HERE - Battery percent=%d, battery volts=%0.2f\n",__func__,b_perc,volts);
    return 0;
  }

#endif