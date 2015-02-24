/*
  Sample sketch for DS3231 library that parses and adjusts current date and time.
 */

#include <TWIMaster.h>
#include <Timeout.h>
#include <FixNum.h>
#include <DS3231.h>

DS3231 rtc;
DateTimeParser parser("[T:", "]");
Timeout printTimeout(1000);

void setup () {
  Serial.begin(57600);
  Serial.println("=== DS3231 Adjust ===");
  Serial.println("Type [T:YY-MM-DD HH:MM:SS] to adjust time");
}

void loop () {
  while (Serial.available()) {
    if (parser.parse(Serial.read())) {
      DateTime dt = parser;
      Serial.print("Adjusting to "); Serial.println(dt.format());
      rtc.adjust(dt);
    }  
  }
  if (printTimeout.check()) {
    printTimeout.reset(1000);
    Serial.print("Now "); Serial.println(rtc.now().format());
  }
}
