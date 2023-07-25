#pragma once

void get_lux(char* lux_char)            
{
    #if (USE_TSL2561 == 1)
        int nbytes;
        sensors_event_t event;
        double lux;

        #ifdef DEBUG
            unsigned long function_start = millis(); 
        #endif

        

        /* Adafruit implementation of TSL2561 is better than Sparkfun - auto gain is ok
        for 13ms the max time to measure is around 52ms
        for 402ms it is 1300ms
        tsl.enableAutoRange(true) makes sure there are no errors from sensor with this function:
            tsl.getEvent(&event)
        */

        /* You can also manually set the gain or enable auto-gain support */
        // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
        // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
        // tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
    
        /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
        // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
        // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
        // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */


        // very little meaurement accuracy difference between high and low sensitivity
        // but very big difference with time needed to measure
        // keep always tsl.enableAutoRange(true); to avoid errors

        tsl.enableAutoRange(true); 
        if (g_lux_high_sens)    // function time from 453ms - 1357ms - depends on amount of light
        {
            tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
        } else                  // function time from 18ms - 51ms - depends on amount of light
        {
            tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
        }

        // #if (DEVICE_ID == 94)
        //     tsl.enableAutoRange(false); 
        //     tsl.setGain(TSL2561_GAIN_1X);
        // #endif 

        tsl.getEvent(&event);

        if (event.light)
        {
            nbytes = snprintf(NULL,0,"%0.0f",event.light) +1;
            snprintf(lux_char,nbytes,"%0.0f",event.light); 
        }
        else
        {
            /* If event.light = 0 lux the sensor is probably saturated
            and no reliable data could be generated! */
            Serial.printf("[%s]: LUX overload=%d\n",__func__,event.light);
            snprintf(lux_char,sizeof("N/A"), "%s", "N/A");
            Serial.printf("[%s]: LUX str=%slx\n",__func__,lux_char);  
        }

    #endif
}