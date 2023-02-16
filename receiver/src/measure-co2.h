#pragma once



void cb_measure_co2_timer( TimerHandle_t measure_co2_timer_handle ) // ()  //(TimerHandle_t xTimer )   // 2ms
{ 
//   configASSERT( xTimer );

  last_indoor_co2=atoi(co2);
  // reset flag
  co2_received = false;

/* this allows to communicate with the timer itself and later to receive the data with the next time expiry
  uint16_t minimum_timer_expired_times = (CO2_PREHEAT_TIME_S / CO2_UPDATE_INTERVAL_S) + 1;

  uint32_t ulCount = ( uint32_t ) pvTimerGetTimerID( measure_co2_timer_handle );
  ulCount++;
  vTimerSetTimerID( measure_co2_timer_handle, ( void * ) ulCount );

  if( ulCount < minimum_timer_expired_times)
  {
    #ifdef DEBUG
        Serial.printf("[%s]: Timer called %d time (%ds since start), we shall start when timer expired %d more times->we don't measure for first %d seconds - heating time \n",__func__,ulCount,millis()/1000, (minimum_timer_expired_times-ulCount),CO2_PREHEAT_TIME_S);
    #endif
    return;
  }
  */

 if ((millis() / 1000) < CO2_PREHEAT_TIME_S)
 {
    Serial.printf("[%s]: Too early for the measurement, time since start=%us, preheat time=%ds\n",__func__,millis()/1000,CO2_PREHEAT_TIME_S);
    return;
 }
  
  // calibration
  if (calibration_in_progress)
  {
    // #ifdef DEBUG
        Serial.printf("[%s]: CALIBRATION in progress - leaving\n",__func__);
    // #endif
    return;
  }

  #if (USE_MHZ19_CO2 == 1)
    size_t nbytes;
    #ifdef DEBUG
        unsigned long function_start = millis(); 
        Serial.printf("\n[%s]: starting...\n",__func__);
    #endif
    u_int16_t mhz19_range = myMHZ19.getRange();
    if (mhz19_range == 0) 
    {
      Serial.printf("[%s]: ERROR: MHZ19 range=%d\n",__func__,mhz19_range);
    }
    unsigned int co2_result = myMHZ19.getCO2();
    if (co2_result > MIN_CO2)
    {
        co2_received = true;
        nbytes = snprintf(NULL,0,"%d",co2_result) +1;
        snprintf(co2,nbytes,"%d",co2_result);   // set global variable
        // #ifdef DEBUG
            Serial.printf("[%s]: co2_result=%dppm, setting global co2->%s\n",__func__,co2_result,co2);
        // #endif
    } else 
    {
        // don't change co2 value on error - let the previous be displayed
        // #ifdef DEBUG
            Serial.printf("[%s]: ERROR: co2_result=%dppm, global variable remains the same: co2=%s\n",__func__,co2_result,co2);
        // #endif
    }

    #ifdef DEBUG
        Serial.printf("[%s]: co2=%sppm\n",__func__,co2);
    #endif
  #endif
  
  #ifdef DEBUG
    Serial.printf("[%s]: last_indoor_co2=%d, co2_received=%d, co2=%s\n",__func__,last_indoor_co2,co2_received,co2);
    Serial.printf("[%s]: took %ums\n",__func__,(millis()-function_start));
  #endif
}


bool measure_co2_timer_start()
{
  // create  timer if not yet created
  if (measure_co2_timer_handle  == NULL)
  {
    measure_co2_timer_handle = xTimerCreate("LCD clock update timer", pdMS_TO_TICKS(CO2_UPDATE_INTERVAL_S * 1000), pdTRUE, ( void * ) 0, cb_measure_co2_timer);
    if( xTimerStart(measure_co2_timer_handle, 10 ) != pdPASS )
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer start error\n",__func__);
        #endif
        return false;
    } else
    {
      #ifdef DEBUG
        Serial.printf("[%s]: timer STARTED with update interval=%ds\n",__func__,CO2_UPDATE_INTERVAL_S);
      #endif
      return true;
    }
  } else
  //  timer created so restart it
  {
    if( xTimerReset( measure_co2_timer_handle, 0 ) != pdPASS )
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer was not reset\n",__func__);
        #endif
        return false;
    } else
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer RE-STARTED\n",__func__);
        #endif
        return true;
    }
  }
}

bool measure_co2_timer_stop()
{
    if( xTimerStop( measure_co2_timer_handle, 0 ) != pdPASS )
    {
        #ifdef DEBUG  
            Serial.printf("[%s]: timer was NOT stopped\n",__func__);
        #endif
        return false;
    } else
    {
        #ifdef DEBUG
            Serial.printf("[%s]: timer stopped\n",__func__);
        #endif
        return true;
    }
}


void cb_calibrate_mhz19_timer(TimerHandle_t xTimer )
{
    calibration_in_progress = false;
    myMHZ19.calibrate();
    calibration_finished = true;
    // don't print anything here
}

bool calibrate_mhz19_timer_start()
{
  // create  timer if not yet created
  if (calibrate_mhz19_timer_handle  == NULL)
  {
    calibrate_mhz19_timer_handle = xTimerCreate("MHZ19 calibration timer", pdMS_TO_TICKS(CO2_CALIBRATE_TIME_S * 1000), pdFALSE, ( void * ) 0, cb_calibrate_mhz19_timer);
    if( xTimerStart(calibrate_mhz19_timer_handle, 10 ) != pdPASS )
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer start error, calibration_in_progress=%d\n",__func__,calibration_in_progress);
        #endif
        return false;
    } else
    {
        calibration_in_progress = true;
        mqtt_publish_gw_status_values("calibration");
      #ifdef DEBUG
        Serial.printf("[%s]: timer STARTED with update interval=%ds, calibration_in_progress=%d\n",__func__,CO2_CALIBRATE_TIME_S,calibration_in_progress);
      #endif
      return true;
    }
  } else
  //  timer created so restart it
  {
    if( xTimerReset( calibrate_mhz19_timer_handle, 0 ) != pdPASS )
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer was not reset\n",__func__);
        #endif
        return false;
    } else
    {
        #ifdef DEBUG
            Serial.printf("[%s]:  timer RE-STARTED\n",__func__);
        #endif
        return true;
    }
  }
}

bool calibrate_mhz19_timer_stop()           // not in use
{
    if( xTimerStop( calibrate_mhz19_timer_handle, 0 ) != pdPASS )
    {
        #ifdef DEBUG  
            Serial.printf("[%s]: timer was NOT stopped\n",__func__);
        #endif
        return false;
    } else
    {
        #ifdef DEBUG
            Serial.printf("[%s]: timer stopped\n",__func__);
        #endif
        return true;
    }
}
