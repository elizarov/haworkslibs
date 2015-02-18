/*
  Test for SHT1X library with WSDL WxShield.
  This sketch prints measurements to serial and blinks led on each one.
  It also computes water vapor pressure.
*/

#include <Timeout.h>
#include <FixNum.h>
#include <SHT1X.h>
#include <BlinkLed.h>
#include <Humidity.h>

const uint8_t BLINK_LED_PIN = 4; // RED 
const uint8_t STATE_LED_PIN = 5; // GREEN

const uint8_t SHT1X_CLK_PIN = 6;
const uint8_t SHT1X_DATA_PIN = 7;

SHT1X sht1x(SHT1X_CLK_PIN, SHT1X_DATA_PIN);
Timeout stateTimeout(Timeout::SECOND);
Timeout stateLedTimeout;
BlinkLed blinkLed(BLINK_LED_PIN);

void setup() {
  pinMode(STATE_LED_PIN, OUTPUT);
  Serial.begin(57600);
  Serial.println(F("=== SHT1X Test ==="));
}

void loop() {
  if (sht1x.check()) {
    Serial.println(F("--- SHT1X Test ---"));
    SHT1X::temp_t temp = sht1x.getTemp();
    SHT1X::rh_t rh = sht1x.getRH();
    wvp_t wvp = waterVaporPressure(temp, rh);
    Serial.print(F("T=")); Serial.print(temp.format()); Serial.println(F(" Celcius"));
    Serial.print(F("H=")); Serial.print(rh.format());   Serial.println(F(" %"));
    Serial.print(F("P=")); Serial.print(wvp.format());  Serial.println(F(" mbar"));
    digitalWrite(STATE_LED_PIN, 1);
    stateLedTimeout.reset(sht1x.getTemp() ? 1000 : 250);
  }
  // Turn off led after timeout
  if (stateLedTimeout.check())
    digitalWrite(STATE_LED_PIN, 0);
  // Print state periodically if there is some error or measurement in progress
  if (stateTimeout.check()) {
    if (!sht1x.getTemp()) {
      Serial.print(F("SHT1X State = ")); Serial.println(sht1x.getState(), HEX);
    }  
    stateTimeout.reset(Timeout::SECOND);
  }
  // just blink
  blinkLed.blink(1000);
}
