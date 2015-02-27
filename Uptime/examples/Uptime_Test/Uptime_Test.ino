#include <Uptime.h>

void setup() {
  Serial.begin(57600);
}

void loop() {
  Serial.print("Uptime: ");
  Serial.println(uptime(), DEC);
  delay(1000);
}
