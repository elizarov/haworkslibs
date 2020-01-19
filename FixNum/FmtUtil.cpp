#include "FmtUtil.h"

// Replaces all digits with '9' on overflow
static void fillOverflow(char* pos, uint8_t size) {
  for (uint8_t i = 0; i < size; i++)
    if (pos[i] >= '0' && pos[i] < '9')
      pos[i] = '9';
}

// Moves to the left and fills with spaces on the right
static void moveLeft(char* pos, uint8_t size, uint8_t actualSize) {
  for (uint8_t i = 0; i < actualSize; i++)
    pos[i] = pos[i + size - actualSize];
  for (uint8_t i = actualSize; i < size; i++)
    pos[i] = ' ';
}

uint8_t formatInvalid(char* pos, uint8_t size, fmt_t fmt) {
  char sc = (fmt & FMT_SIGN) ? '+' : ' ';
  uint8_t actualSize = 0;
  uint8_t first = (fmt & FMT_PREC) ? (fmt & FMT_PREC) + 1 : 0;
  char* ptr = pos + size;
  for (uint8_t i = 0; i < size; i++) {
    ptr--;
    if (i + 1 == first) {
      *ptr = '.';
      actualSize++;
    } else if ((!fmt & FMT_ZERO) && i > first) {
      *ptr = sc;
      if (sc != ' ')
        actualSize++;
      sc = ' '; // fill the rest with spaces
    } else if ((fmt & FMT_SIGN) && i == size - 1 && sc != ' ') {
      *ptr = sc;
      actualSize++;
    } else {
      *ptr = '?';
      actualSize++;
    }
  }
  if (!(fmt & FMT_RIGHT) && actualSize < size)
    moveLeft(pos, size, actualSize);
  return actualSize;
}

// commont template for all integral types
template<typename T> inline uint8_t formatDecimalT(T x, char* pos, uint8_t size, fmt_t fmt) {
  char sc = (fmt & FMT_SIGN) ? '+' : ' ';
  if (x < 0) {
    x = -x;
    sc = '-';
  }
  uint8_t actualSize = 0;
  uint8_t first = (fmt & FMT_PREC) ? (fmt & FMT_PREC) + 1 : 0;
  char* ptr = pos + size;
  for (uint8_t i = 0; i < size; i++) {
    ptr--;
    if (i + 1 == first) {
      *ptr = '.';
      actualSize++;
    } else if (!(fmt & FMT_ZERO) && x == 0 && i > first) {
      *ptr = sc;
      if (sc != ' ')
        actualSize++;
      sc = ' '; // fill the rest with spaces
    } else if ((fmt & FMT_SIGN) && i == size - 1 && sc != ' ') {
      *ptr = sc;
      actualSize++;
    } else {
      *ptr = '0' + x % 10;
      x /= 10;
      actualSize++;
    }
  }
  if (x != 0)
    fillOverflow(pos, size);
  if (!(fmt & FMT_RIGHT) && actualSize < size)
    moveLeft(pos, size, actualSize);
  return actualSize;
}

// definition for int16
uint8_t formatDecimal(int16_t x, char* pos, uint8_t size, fmt_t fmt) {
  return formatDecimalT(x, pos, size, fmt);
}

// definition for int32
uint8_t formatDecimal(int32_t x, char* pos, uint8_t size, fmt_t fmt) {
  return formatDecimalT(x, pos, size, fmt);
}

// forward to int16
uint8_t formatDecimal(uint8_t x, char* pos, uint8_t size, fmt_t fmt) {
  return formatDecimal((int16_t)x, pos, size, fmt);
}

// forward to int16
uint8_t formatDecimal(int8_t x, char* pos, uint8_t size, fmt_t fmt) {
  return formatDecimal((int16_t)x, pos, size, fmt);
}
