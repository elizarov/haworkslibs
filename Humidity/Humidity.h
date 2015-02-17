/*
  Library for variuos humidity-related computations.
  Computes water vapor pressure at a given temperature and relative humidity.
  It is used to figure out what air has more water in it.
*/

#ifndef HUMIDITY_H_
#define HUMIDITY_H_

#include <FixNum.h>

typedef fixnum32_3 wvp_t; 
typedef fixnum16_2 wvp_temp_t;
typedef fixnum16_1 wvp_rh_t;

//   temp -- temperature in Celcius (from -40 to +40, result is not available / invalid if outside this range)
//   rh   -- relative humidity in percent (from 0 to 100, is trimmed to this range)
// result -- presure of water vapor in mb (millibars)
wvp_t waterVaporPressure(wvp_temp_t temp, wvp_rh_t rh);

#endif
