
#include "humidity.h"

#include <avr/pgmspace.h>

const int16_t MIN_T = -40;
const int16_t MAX_T = 40;

// Water pressue table in units of 0.1 Pa (1 microbar, ub)
const int32_t WVP[MAX_T - MIN_T + 1] PROGMEM = {
  189, // from -40 Celcuis
  210,
  232,
  257,
  284,
  314,
  346,
  382,
  420,
  463,
  509,
  559,
  613,
  673,
  737,
  807,
  883,
  965,
  1054,
  1150,
  1254,
  1366,
  1487,
  1618,
  1759,
  1919,
  2075,
  2251,
  2440,
  2644,
  2862,
  3097,
  3348,
  3617,
  3906,
  4214,
  4544,
  4897,
  5275,
  5677,
  6107,
  6565,
  7054,
  7574,
  8128,
  8718,
  9345,
  10012,
  10720,
  11472,
  12271,
  13118,
  14015,
  14967,
  15975,
  17042,
  18171,
  19365,
  20628,
  21962,
  23371,
  24858,
  26428,
  28083,
  29829,
  31668,
  33606L,
  35646L,
  37793L,
  40052L,
  42427L,
  44924L,
  47548L,
  50303L,
  53196L,
  56233L,
  59418L,
  62758L,
  66260L,
  69930L,
  73773L  // to +40 Celcius
};

wvp_t waterVaporPressure(wvp_temp_t temp, wvp_rh_t rh) {
  if (!temp || !rh)
    return wvp_t::invalid();
  int16_t t = temp.floor();
  if (t < MIN_T || t >= MAX_T)
    return wvp_t::invalid();
  if (rh > 100)
    rh = fixnum16_0(100); 
  if (rh < 0)
    rh = fixnum16_0(0);
  wvp_t p0 = (wvp_t)pgm_read_dword(WVP + t - MIN_T);
  wvp_t p1 = (wvp_t)pgm_read_dword(WVP + t - MIN_T + 1); 
  wvp_temp_t t0 = temp - wvp_temp_t::scale(t);
  wvp_temp_t t1 = 1 - t0;
  return (p0 * t0 + p1 * t1) * rh / 100;
}
