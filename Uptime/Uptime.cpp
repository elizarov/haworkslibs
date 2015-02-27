#include "Uptime.h"

const unsigned long DAY_LENGTH_MS = 24 * 60 * 60000L;

unsigned long daystart = 0;
int updays = 0;

long uptime() {
  unsigned long time = millis();
  while ((time - daystart) > DAY_LENGTH_MS) {
    daystart += DAY_LENGTH_MS;
    updays++;
  }
  time -= daystart;
  time /= 1000; // convert seconds
  uint8_t upsecs = time % 60;
  time /= 60; // minutes
  uint8_t upmins = time % 60;
  time /= 60; // hours
  uint8_t uphours = time;
  return ((updays * 100L + uphours) * 100L + upmins) * 100L + upsecs;
}
