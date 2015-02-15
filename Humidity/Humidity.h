/*
  Library for variuos humidity-related computations.
  Computes water vapor pressure at a given temperature and relative humidity.
  It is used to figure out what air has more water in it.
*/

#ifndef HUMIDITY_H_
#define HUMIDITY_H_

#include <FixNum.h>

typedef fixnum32_3 wvp_t; 

//   temp -- temperature in Celcius
//   rh   -- relative humidity in percent
// result -- presure of water vapor in mb (millibars)
wvp_t waterVaporPressure(fixnum16_2 temp, fixnum16_1 rh);

#endif
