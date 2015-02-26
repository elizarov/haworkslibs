/*
  Test code for TWISlave that transmits 4 bytes of millis time and receives 4 bytes and prints them.
  This sketch prints measurements to serial and blinks led on each one.

  Author: Roman Elizarov
*/

#include <TWISlave.h>
#include <BlinkLed.h>

const char ADDR = 'T';

const uint8_t BLINK_LED_PIN = 13;

BlinkLed blinkLed(BLINK_LED_PIN);

union {
  uint8_t bytes[4];
  long time;
} buf;

void printBuf(uint8_t doneSize) {
  for (uint8_t i = 0; i < doneSize; i++) {
    Serial.print(' ');
    Serial.print(buf.bytes[i], HEX);
  }
  Serial.println();  
}

void twiSlaveTransmit(uint8_t doneSize, bool more) {
  Serial.print(F("Transmit("));
  Serial.print(doneSize, DEC);
  Serial.print(',');
  Serial.print(more ? F("MORE"): F("DONE"));
  Serial.println(")");
  if (doneSize == 0) { // start
    buf.time = millis();
    TWISlave.use(&buf, sizeof(buf));
  } 
  if (!more) // done
    printBuf(doneSize);
}

void twiSlaveReceive(uint8_t doneSize, bool more) {
  Serial.print(F("Receive("));
  Serial.print(doneSize, DEC);
  Serial.print(',');
  Serial.print(more ? F("MORE"): F("DONE"));
  Serial.println(")");
  if (doneSize == 0) { // start
    buf.time = millis();
    TWISlave.use(&buf, sizeof(buf));
  } 
  if (!more) // done
    printBuf(doneSize);
}

void setup() {
  Serial.begin(57600);
  Serial.println(F("=== TWISlave Test ==="));
  Serial.print(F("Listening on TWI address ")); Serial.println(ADDR, HEX);
  TWISlave.begin(ADDR);
}

void loop() {
  blinkLed.blink(1000);
}
