#pragma once

  void measure_volt(int pin, int attennuation, int iteration, float resistors_calibration, bool with_lut, String arr_volts[]) 
  {
    #ifdef DEBUG
      long sm = millis(); //start measurement time
    #endif
    int i = 0;
    uint32_t read_digital = 0;
    uint32_t milivolts = 0;
    float volts = 0.0;
    float detailed_calibration=1.0;
    /*
    //ESP32:
    ADC_ATTEN_DB_0:   100 mV ~ 950 mV
    ADC_ATTEN_DB_2_5: 100 mV ~ 1250 mV
    ADC_ATTEN_DB_6:   150 mV ~ 1750 mV
    ADC_ATTEN_DB_11:  150 mV ~ 2450 mV
    //ESP32S2:
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

    analogSetPinAttenuation(pin, atten32);
    adcAttachPin(pin);

    for ( i = 0; i < iteration; i++) {
        read_digital += analogRead(pin);
    }
    read_digital /= iteration; //digital averaged reading
    // Serial.println("1-Averaged read_digital long=\t"+String(read_digital));

    #if   (BOARD_TYPE == 1)
      milivolts = ((3.3 * read_digital)/4095) * 1000;
    #elif (BOARD_TYPE == 2)
      milivolts = ((3.3 * read_digital)/8191) * 1000;
    #else
      #error "This functions is configured for ESP32/ESP32S2 only."
    #endif

    // Serial.println("2-Averaged milivolts long=\t"+String(milivolts));
    volts = (resistors_calibration *  milivolts) /1000 ; //only resistors here should be
    // Serial.println("3-Calibrated Volts float=\t"+String(volts,4));


    volts = volts * detailed_calibration;
    // Serial.println("5-Det. Calibrated volts float=\t"+String(volts,4)+"\n\n");
    arr_volts[0] = String(volts);
    arr_volts[1] = String(read_digital);

    #ifdef DEBUG
    long em = millis(); //END measurement time
    long mt=em-sm;
      Serial.println("VOLT MEASUREMENT TIME:"+String(mt)+"ms");
    #endif
  }


