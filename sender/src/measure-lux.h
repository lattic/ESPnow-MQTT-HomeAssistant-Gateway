#pragma once

void get_lux(char* lux_char)            
{
    snprintf(lux_char,sizeof("N/A"), "%s", "N/A");
    
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

    if (g_lux_high_sens)    // function time from 453ms - 1357ms - depends on amount of light
    {
        tsl.enableAutoRange(true); 
        tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
    } else                  // function time from 18ms - 51ms - depends on amount of light
    {
        tsl.enableAutoRange(true); 
        tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
    }



    /* Get a new sensor event */ 
    if (!tsl.getEvent(&event))
    {
        Serial.printf("[%s]: LUX fatal error=%s\n",__func__,lux_char);
    } else
    {
        lux = event.light;
        if ((lux > 0) and (lux < 65536))
        {
            nbytes = snprintf(NULL,0,"%0.0f",lux) +1;
            snprintf(lux_char,nbytes,"%0.0f",lux);
        } else
        if (lux <= 0)
        {
            /* If event.light = 0 if the sensor is saturated and the values are unreliable 
            return 0 for the sake of knowing there is no light 
            check if this ZERO is reliable !!!
            */
            snprintf(lux_char,sizeof("0"), "%s", "0");
            Serial.printf("[%s]: LUX unreliable or LUX=%s\n",__func__,lux_char);
        } else 
        if (lux >= 65536)
        {
            /* If event.light = 65536 the sensor is saturated. */
            snprintf(lux_char,sizeof("65536"), "%s", "65536");
            Serial.printf("[%s]: Sensor overload=%s\n",__func__,lux_char);
        }  
    }

#endif
    #ifdef DEBUG
        Serial.printf("[%s]: LUX str=%s\n",__func__,lux_char);  
        Serial.printf("[%s]: took %ums\n",__func__,(millis()-function_start));
    #endif
}