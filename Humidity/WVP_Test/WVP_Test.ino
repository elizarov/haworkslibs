/*
  Simple test for water vapor pressure table.
 */

#include <FixNum.h>
#include <Humidity.h>

void setup() {
  Serial.begin(57600);
  for (int t = -41; t <= 44; t += 5) {
    Serial.print("T="); Serial.print(t, DEC);
    Serial.print(" P=");
    Serial.println(waterVaporPressure(fixnum16_0(t), fixnum8_0(100)).format());  
  }
}

void loop() {
  // nothing
}
