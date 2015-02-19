/*
  Sample sketch for SHT1X library.

  Author: Roman Elizarov
*/

#include <Timeout.h>
#include <FixNum.h>
#include <SHT1X.h>

const uint8_t SHT1X_CLK_PIN = 10;
const uint8_t SHT1X_DATA_PIN = 11;

SHT1X sht1x(SHT1X_CLK_PIN, SHT1X_DATA_PIN);
Timeout stateTimeout(Timeout::SECOND);

void setup() {
  Serial.begin(57600);
  Serial.println("=== SHT1X Test ===");
}

void loop() {
  if (sht1x.check()) {
    Serial.println("--- SHT1X Test ---");
    Serial.print("T = "); Serial.println(sht1x.getTemp().format());
    Serial.print("H = "); Serial.println(sht1x.getRH().format());
  }
  // Print state periodically if there is some error or measurement in progress
  if (stateTimeout.check()) {
    if (sht1x.getState() != 0) {
      Serial.print("SHT1X State = "); Serial.println(sht1x.getState(), HEX);
    }  
    stateTimeout.reset(Timeout::SECOND);
  }
}
