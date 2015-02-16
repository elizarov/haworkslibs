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
*/

#include <Timeout.h>
#include <HIH6120.h>
#include <TWIMaster.h>
#include <FixNum.h>

const uint8_t LED_PIN = 13;

HIH6120 hih6120;
Timeout stateTimeout(1000);
Timeout ledTimeout;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(57600);
  Serial.println("=== HIH6120 Test ===");
}

void loop() {
  if (hih6120.check()) {
    Serial.println("--- HIH6120 Test ---");
    Serial.print("T="); Serial.println(hih6120.getTemp().format());
    Serial.print("H="); Serial.println(hih6120.getRH().format());
    digitalWrite(LED_PIN, 1);
    ledTimeout.reset(hih6120.getTemp() ? 1000 : 250);
  }
  // Turn off led after timeout
  if (ledTimeout.check())
    digitalWrite(LED_PIN, 0);
  // Print state periodically if there is some error or measurement in progress
  if (stateTimeout.check()) {
    if (hih6120.getState() != 0) {
      Serial.print("HIH6120 State = "); Serial.println(hih6120.getState(), HEX);
    }
    stateTimeout.reset(1000);
  }
}