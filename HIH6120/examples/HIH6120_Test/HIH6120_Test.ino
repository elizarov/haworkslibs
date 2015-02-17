/*
  Test code for HIH6120 temperature / humidity sensor.
  Connect sensor to I2C pins:
  
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
*/

#include <Timeout.h>
#include <HIH6120.h>
#include <TWIMaster.h>
#include <FixNum.h>
#include <Humidity.h>

const uint8_t LED_PIN = 13;

HIH6120 hih6120;
Timeout stateTimeout(1000);
Timeout ledTimeout;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(57600);
  Serial.println(F("=== HIH6120 Test ==="));
}

void loop() {
  if (hih6120.check()) {
    Serial.println(F("--- HIH6120 Test ---"));
    HIH6120::temp_t temp = hih6120.getTemp();
    HIH6120::rh_t rh = hih6120.getRH();
    wvp_t wvp = waterVaporPressure(temp, rh);
    Serial.print(F("T=")); Serial.print(temp.format()); Serial.println(F(" Celcius"));
    Serial.print(F("H=")); Serial.print(rh.format());   Serial.println(F(" %"));
    Serial.print(F("P=")); Serial.print(wvp.format());  Serial.println(F(" mbar"));
    digitalWrite(LED_PIN, 1);
    ledTimeout.reset(hih6120.getTemp() ? 1000 : 250);
  }
  // Turn off led after timeout
  if (ledTimeout.check())
    digitalWrite(LED_PIN, 0);
  // Print state periodically if there is some error or measurement in progress
  if (stateTimeout.check()) {
    if (hih6120.getState() != 0) {
      Serial.print(F("HIH6120 State = ")); Serial.println(hih6120.getState(), HEX);
    }
    stateTimeout.reset(1000);
  }
}
