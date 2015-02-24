/*
  Sample sketch for DS3231 library that prints temperature
 */

#include <TWIMaster.h>
#include <Timeout.h>
#include <FixNum.h>
#include <DS3231.h>

Timeout stateTimeout(Timeout::SECOND);

DS3231Temp rtc;

void setup () {
  Serial.begin(57600);
  Serial.println("=== DS3231 Temperature ===");
}

void loop() {
  if (rtc.check()) {
    Serial.println("--- DS3231 Temperature ---");
    Serial.print("T = "); Serial.println(rtc.getTemp().format());
  }
  // Print state periodically if there is some error or measurement in progress
  if (stateTimeout.check()) {
    if (rtc.getLastError() != 0) {
      Serial.print("DS3231 Error = "); Serial.println(rtc.getLastError(), HEX);
    }
    stateTimeout.reset(Timeout::SECOND);
  }
}
