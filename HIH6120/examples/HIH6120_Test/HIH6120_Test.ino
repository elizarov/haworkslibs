#include <Timeout.h>
#include <HIH6120.h>
#include <TWIMaster.h>
#include <FixNum.h>

HIH6120 hih6120(TWI_10K);
Timeout stateTimeout(Timeout::SECOND);

void setup() {
  Serial.begin(57600);
  Serial.println("=== HIH6120 Test ===");
}

void loop() {
  if (hih6120.check()) {
    Serial.println("--- HIH6120 Test ---");
    Serial.print("H="); Serial.println(hih6120.getRH().format());
    Serial.print("T="); Serial.println(hih6120.getTemp().format());
  }
  // Print state periodically if there is some error or measurement in progress
  if (stateTimeout.check()) {
    if (hih6120.getState() != 0) {
      Serial.print("HIH6120 State = "); Serial.println(hih6120.getState(), HEX);
    }
    stateTimeout.reset(Timeout::SECOND);
  }
}