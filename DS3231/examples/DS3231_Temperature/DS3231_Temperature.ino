/*
  Sample sketch for DS3231 library that prints temperature
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
  rtc.convertTemperature();            //convert current temperature into registers
  Serial.print(rtc.getTemperature());  //read registers and display the temperature
  Serial.println(" deg C");
  delay(1000);
}
