/*
  Test code for Honeywell HumidIcon HIHxxx-020 I2C temperature and humidity sensors driver.
  This includes HIH6000, HIH6100, HIH8000, and HIH9000 series.

  Connect sensor in SIP package to I2C pins in the following way:
  
  +-------+
  | *     |
  +-------+
   | | | |
   1 2 3 4

  1 - VDD 
  2 - GND
  3 - SCL --> A5
  4 - SDA --> A4

  This sketch prints measurements to serial and blinks led on each one.
  It also computes water vapor pressure.

  Author: Roman Elizarov
*/

#include <Timeout.h>
#include <HIH.h>
#include <TWIMaster.h>
#include <FixNum.h>
#include <Humidity.h>

const uint8_t LED_PIN = 13;

HIH hih;
Timeout stateTimeout(1000);
Timeout ledTimeout;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(57600);
  Serial.println(F("=== HIH Test ==="));
}

void loop() {
  if (hih.check()) {
    Serial.println(F("--- HIH Test ---"));
    HIH::temp_t temp = hih.getTemp();
    HIH::rh_t rh = hih.getRH();
    wvp_t wvp = waterVaporPressure(temp, rh);
    Serial.print(F("T = ")); Serial.print(temp.format()); Serial.println(F(" Celcius"));
    Serial.print(F("H = ")); Serial.print(rh.format());   Serial.println(F(" %"));
    Serial.print(F("P = ")); Serial.print(wvp.format());  Serial.println(F(" mbar"));
    digitalWrite(LED_PIN, 1);
    ledTimeout.reset(hih.getTemp() ? 1000 : 250);
  }
  // Turn off led after timeout
  if (ledTimeout.check())
    digitalWrite(LED_PIN, 0);
  // Print state periodically if there an error
  if (stateTimeout.check()) {
    if (!hih.getTemp()) {
      Serial.print(F("HIH State = ")); Serial.println(hih.getState(), HEX);
    }
    stateTimeout.reset(1000);
  }
}
