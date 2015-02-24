/*
  Sample sketch for DS3231 library that prints current date and time.
 */

#include <TWIMaster.h>
#include <Timeout.h>
#include <FixNum.h>
#include <DS3231.h>

DS3231 rtc;

void setup () {
  Serial.begin(57600);
}

void loop () {
  DateTime now = rtc.now(); //get the current date-time
  Serial.println(now.format());
  delay(1000);
}
