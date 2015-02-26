/*
  Test sketch that is supposed to transmit 4 bytes via I2C acting as a master
  to TWISlave_Test sketch that acts as a slave.
*/

#include <TWIMaster.h>
#include <BlinkLed.h>
#include <Timeout.h>

const char ADDR = 'T';

const uint8_t BLINK_LED_PIN = 13;

BlinkLed blinkLed(BLINK_LED_PIN);
Timeout timeout(0);

long data;
uint8_t status = 0;

void setup() {
  Serial.begin(57600);
  Serial.println(F("=== TWIMaster Transmit Test ==="));
  Serial.print(F("Transmitting to TWI address ")); Serial.println(ADDR, HEX);
}

void loop() {
  if (timeout.check()) {
    data = millis();
    status = TWIMaster.transmit(ADDR, data);
    Serial.print(F("Transmit status: "));
    Serial.println(status, HEX);
    timeout.reset(1000);
  }
  blinkLed.blink(status == 0 ? 1000 : 250);
}
