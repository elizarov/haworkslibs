/*
  Utility library that returns uptime as a time-formatted decimal number as:

   <days>HHMMSS

  Note, that this method is based on Arduinos millis() which wraps in 49 days, so
  this method has its own day tracking and must be called periodically to avoid wrap.

  Author: Roman Elizarov
*/
#ifndef UPTIME_H_
#define UPTIME_H_

#include <Arduino.h>

long uptime();

#endif