/*
  Sample sketch for DS18B20 library.

  Author: Roman Elizarov
*/

#include <OneWire.h>
#include <FixNum.h>
#include <Timeout.h>
#include <DS18B20.h>

const uint8_t DS18B20_PIN = 8;
Timeout stateTimeout(Timeout::SECOND);

DS18B20 ds18b20(DS18B20_PIN);

void setup() {
  Serial.begin(57600);
  Serial.println("=== DS18B20 Test ===");
}

void loop() {
  if (ds18b20.check()) {
    Serial.println("--- DS18B20 Test ---");
    Serial.print("T = "); Serial.println(ds18b20.getTemp().format());
  }
  // Print state periodically if there is some error or measurement in progress
  if (stateTimeout.check()) {
    if (ds18b20.getLastError() != 0) {
      Serial.print("DS18B20 Error = "); Serial.println(ds18b20.getLastError(), HEX);
    }
    stateTimeout.reset(Timeout::SECOND);
  }
}
