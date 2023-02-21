#pragma once

void get_lux(char* lux_char)            // <20us with time = 0
{
    #ifdef DEBUG
        unsigned long function_start = micros(); 
    #endif

    snprintf(lux_char,sizeof("N/A"), "%s", "N/A");

    #if (USE_TSL2561 == 1)
        size_t nbytes;
        
        unsigned int data0, data1; //data0=infrared, data1=visible light
        double lux;              // Resulting lux value
        boolean good;            // True if neither sensor is saturated
        // boolean gain = true;     // Gain setting, 0 = X1, 1 = X16; if with gain and overshoots it goes from 0 again - better false
        unsigned int ms;         // Integration ("shutter") time in milliseconds
        // If time = 0, integration will be 13.7ms
        // If time = 1, integration will be 101ms
        // If time = 2, integration will be 402ms
        // If time = 3, use manual start / stop to perform your own integration

        /*
        for indoor measurements and not battery powered devices the proper setting is:
        - gain true - x16
        - time 2 - 402ms
        - for saturation case: reduce the gain and repeat the measurement - this will increase the time to 0.8s (2x 402ms)
        */
        // unsigned char time = 2;

        unsigned char time;
        boolean gain;

        if (light_high)
        {
            time = 1;
            gain = false;
        } else
        {
            time = 2;
            gain = true;
        }

        light.begin();
        light.setTiming(gain,time,ms);
        if (!light.setPowerUp()) 
        {
            snprintf(lux_char,sizeof("NaN"), "%s", "NaN");
            Serial.printf("[%s]: LUX error=%s, LEAVING\n",__func__,lux_char);
            return;
        }

        light.manualStart();
        delay(ms);
        light.manualStop();

        if (light.getData(data0,data1))
        {
            good = light.getLux(gain,ms,data0,data1,lux);
            if (!good)              // lets repeat the measurement with low gain=0 (x1)
            {
                Serial.printf("[%s]: LUX error in fist measurement: sensor saturated - REPEATING measurement with gain=0 and time=1\n",__func__);
                Serial.printf("[%s]: First measurement: chan0(wide spectrum)=%d, chan1(infrared)=%d, lux=%f\n",__func__,data0,data1,lux);
                gain = false; time = 1;
                light.setTiming(gain,time,ms);
                light.manualStart();
                delay(ms);
                light.manualStop();

                if (light.getData(data0,data1))
                {
                    good = light.getLux(gain,ms,data0,data1,lux);
                    Serial.printf("[%s]: Second measurement (with low gain): chan0(wide spectrum)=%d, chan1(infrared)=%d, lux=%f\n",__func__,data0,data1,lux);
                    if (good)
                    {
                        nbytes = snprintf(NULL,0,"%0.3f",lux) +1;
                        snprintf(lux_char,nbytes,"%0.3f",lux);
                        light_high = true;
                    } else
                    {
                        // saturated again!
                        Serial.printf("[%s]: LUX error - sensor saturated AGAIN (even with low gain) - providing error code 99999\n",__func__);
                        snprintf(lux_char,sizeof("99999"), "%s", "99999");
                        return;                  
                    }
                } else
                {
                    snprintf(lux_char,sizeof("NaN"), "%s", "NaN");
                    Serial.printf("[%s]: 2nd measurements: no data: LUX error=%s, LEAVING\n",__func__,lux_char);
                    return;
                }
            }
            // lux is good below:

            // more digits for low lux
            if ((lux > 0) and (lux < 10))
            {
                nbytes = snprintf(NULL,0,"%0.3f",lux) +1;
                snprintf(lux_char,nbytes,"%0.3f",lux);
            } else 
            // 1 digit after coma for high lux
            {
                nbytes = snprintf(NULL,0,"%0.0f",lux) +1;
                snprintf(lux_char,nbytes,"%0.0f",lux);
            }
            if (lux > 100) light_high = true; else light_high = false;
            #ifdef DEBUG
                Serial.printf("[%s]: LUX str=%s\n",__func__,lux_char);  
            #endif
        } else 
        // no response for the first measurement:
        {
            snprintf(lux_char,sizeof("NaN"), "%s", "NaN");
            Serial.printf("[%s]: no data: LUX error=%s, LEAVING\n",__func__,lux_char);
            return;
        }
    #endif
    #ifdef DEBUG
        Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
    #endif
}