#include <Timeout.h>
#include <HIH6120.h>
#include <TWIMaster.h>
#include <FixNum.h>

HIH6120 hih6120(TWI_100K);

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (hih6120.check()) {
    Serial.println("--- HIH6120 test ---");
    Serial.print("H="); Serial.println(hih6120.getRH().format());
    Serial.print("T="); Serial.println(hih6120.getTemp().format());
  }
}
