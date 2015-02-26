/*
  Test sketch that is supposed to receive 4 bytes via I2C acting as a master
  from TWISlave_Test sketch that acts as a slave.
*/

#include <TWIMaster.h>
#include <BlinkLed.h>
#include <Timeout.h>

const char ADDR = 'T';

const uint8_t BLINK_LED_PIN = 13;

BlinkLed blinkLed(BLINK_LED_PIN);
Timeout timeout(0);

uint8_t buf[4];
uint8_t status = 0;

void setup() {
  Serial.begin(57600);
  Serial.println(F("=== TWIMaster Receive Test ==="));
  Serial.print(F("Requesting from TWI address ")); Serial.println(ADDR, HEX);
}

void loop() {
  if (timeout.check()) {
    status = TWIMaster.receive(ADDR, buf);
    Serial.print(F("Receive status: "));
    Serial.println(status);
    if (status == 0) {
      for (uint8_t i = 0; i < sizeof(buf); i++) {
        Serial.print(' ');
        Serial.print(buf[i], HEX);
      }
      Serial.println();  
    }  
    timeout.reset(1000);
  }
  blinkLed.blink(status == 0 ? 1000 : 250);
}
