/*
  Utility routines to format decimal numbers represented in integral types with fixed decimal point.
  Only methods for int16 and int32 are provied to converse code size.
  Should not be used directly. Use FixNum class for convenience.

  Author: Roman Elizarov
*/

#ifndef FMT_UTIL_H_
#define FMT_UTIL_H_

#include <inttypes.h>

typedef uint8_t fmt_t;

const fmt_t FMT_NONE    = 0;
const fmt_t FMT_PREC    = 0x0f;  // define number precision in lower bits
const fmt_t FMT_SIGN    = 0x10;  // always print sign at the first position
const fmt_t FMT_ZERO    = 0x20;  // fill with extra leading zeroes
const fmt_t FMT_RIGHT   = 0x40;  // right-align the result

uint8_t formatInvalid(char* pos, uint8_t size, fmt_t fmt = FMT_NONE);

uint8_t formatDecimal(uint8_t x, char* pos, uint8_t size, fmt_t fmt = FMT_NONE);
uint8_t formatDecimal(int8_t x, char* pos, uint8_t size, fmt_t fmt = FMT_NONE);
uint8_t formatDecimal(int16_t x, char* pos, uint8_t size, fmt_t fmt = FMT_NONE);
uint8_t formatDecimal(int32_t x, char* pos, uint8_t size, fmt_t fmt = FMT_NONE);

#endif
