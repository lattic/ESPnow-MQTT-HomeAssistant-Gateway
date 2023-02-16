#pragma once
/*
various functions
*/

#include "config.h"
#include "variables.h"

// used for gw
void ConvertSectoDay(unsigned long n, char *pretty_ontime)
{
  bool debug_mode = false;
  size_t nbytes;
  int day = n / (24 * 3600);
  n = n % (24 * 3600);
  int hour = n / 3600;
  n %= 3600;
  int minutes = n / 60 ;
  n %= 60;
  int seconds = n;
  if (day>0)
  {
    nbytes = snprintf(NULL,0,"%0dd %0dh %0dm %0ds",day,hour,minutes,seconds)+1;
    snprintf(pretty_ontime,nbytes,"%0dd %0dh %0dm %0ds",day,hour,minutes,seconds);
  } else
  {
    if (hour>0)
    {
      nbytes = snprintf(NULL,0,"%0dh %0dm %0ds",hour,minutes,seconds)+1;
      snprintf(pretty_ontime,nbytes,"%0dh %0dm %0ds",hour,minutes,seconds);
    } else
    {
      if (minutes>0)
      {
        nbytes = snprintf(NULL,0,"%0dm %0ds",minutes,seconds)+1;
        snprintf(pretty_ontime,nbytes,"%0dm %0ds",minutes,seconds);
      } else
      {
        nbytes = snprintf(NULL,0,"%0ds",seconds)+1;
        snprintf(pretty_ontime,nbytes,"%0ds",seconds);
      }
    }
  }
  if (debug_mode)
    Serial.printf("[%s]: pretty_ontime=%s\n",__func__,pretty_ontime);
}

// used for sensors
void uptime(char *uptime)
{
  byte days = 0;
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;

  unsigned long sec2minutes = 60;
  unsigned long sec2hours = (sec2minutes * 60);
  unsigned long sec2days = (sec2hours * 24);

  unsigned long time_delta = (millis()) / 1000UL;

  days = (int)(time_delta / sec2days);
  hours = (int)((time_delta - days * sec2days) / sec2hours);
  minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
  seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

  if ((minutes == 0) and (hours == 0) and (days == 0)){
    sprintf(uptime, "%01ds", seconds);
  }
  else {
    if ((hours == 0) and (days == 0))
    {
      sprintf(uptime, "%01dm %01ds", minutes, seconds);
    }
    else
    {
      if (days == 0)
      {
        sprintf(uptime, "%01dh %01dm", hours, minutes);
      } else
      {
          sprintf(uptime, "%01dd %01dh %01dm", days, hours, minutes);
      }
    }
  }
  if (debug_mode)
    Serial.printf("[%s]: uptime=%s\n",__func__,uptime);

}
