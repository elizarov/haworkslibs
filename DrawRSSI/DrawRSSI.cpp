#include "DrawRSSI.h"

/* Draws 10 x 8 RSSI bar
    y
    0           ##
    1           ##
    2        ## ##
    3        ## ##
    4     ## ## ##
    5     ## ## ##
    6  ## ## ## ##
    7  ## ## ## ## 
    x  01234567890
*/
void drawRSSI(Adafruit_GFX& d, int16_t x, int16_t y, int32_t rssi, uint16_t color) {
  // -80 -> 1, -75 -> 2 ... , -45 -> 8
  int32_t level = (level + 85) / 5;
  if (level < 0) level = 0;
  if (level > 8) level = 8;
  for (int16_t i = 0; i < level; i++) {
    int16_t xi = x + i + (i / 2);
    int16_t h = (i & 0xE) + 2;
    d.drawLine(xi, y + 7, xi, y + 8 - h, color);
  }
}
