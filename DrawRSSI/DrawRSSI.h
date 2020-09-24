#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>

void drawRSSI(Adafruit_GFX& d, int16_t x, int16_t y, int32_t rssi, uint16_t color);
