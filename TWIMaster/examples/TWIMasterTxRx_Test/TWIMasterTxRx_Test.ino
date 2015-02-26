/*
  Test sketch that transmits and receives in a single transaction.
*/

#include <TWIMaster.h>
#include <BlinkLed.h>
#include <Timeout.h>

const char ADDR = 'T';

const uint8_t BLINK_LED_PIN = 13;

BlinkLed blinkLed(BLINK_LED_PIN);
Timeout timeout(0);

long tx;
uint8_t rx[4];
uint8_t status = 0;

void setup() {
  Serial.begin(57600);
  Serial.println(F("=== TWIMaster TxRx Test ==="));
  Serial.print(F("Working with TWI address ")); Serial.println(ADDR, HEX);
}

void loop() {
  if (timeout.check()) {
    tx = millis();
    status = TWIMaster.transmitReceive(ADDR, tx, rx);
    Serial.print(F("TxRx status: "));
    Serial.println(status, HEX);
    if (status == 0) {
      for (uint8_t i = 0; i < sizeof(rx); i++) {
        Serial.print(' ');
        Serial.print(rx[i], HEX);
      }
      Serial.println();  
    }  
    timeout.reset(1000);
  }
  blinkLed.blink(status == 0 ? 1000 : 250);
}
