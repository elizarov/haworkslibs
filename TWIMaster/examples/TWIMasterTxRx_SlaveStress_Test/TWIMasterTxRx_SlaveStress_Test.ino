/*
  Test sketch that transmits and receives in a single transaction
  and also acts as a slave on the same address. 
  The is random delay to truly stress-test multi-master operation.
*/

#include <TWIMaster.h>
#include <TWISlave.h>
#include <BlinkLed.h>
#include <Timeout.h>

const char ADDR = 'T';
const long ERROR_TIME = 5000; // blink error when there are no master exchanges in 5s
const uint8_t BLINK_LED_PIN = 13;

BlinkLed blinkLed(BLINK_LED_PIN);

Timeout timeout(0);
Timeout errorTimeout;

// master part
long tx;
uint8_t rx[4];

// slave part
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
  Serial.println(F("=== TWIMaster TxRx Slave Stress Test ==="));
  Serial.print(F("Working with TWI address ")); Serial.println(ADDR, HEX);
  TWISlave.begin(ADDR);
}

void loop() {
  if (timeout.check()) {
    tx = millis();
    uint8_t status = TWIMaster.transmitReceive(ADDR, tx, rx);
    Serial.print(F("TxRx status: "));
    Serial.println(status, HEX);
    if (status == 0) {
      for (uint8_t i = 0; i < sizeof(rx); i++) {
        Serial.print(' ');
        Serial.print(rx[i], HEX);
      }
      Serial.println();  
      errorTimeout.reset(ERROR_TIME);
    }  
    // random delay
    timeout.reset(random(1000));
  }
  errorTimeout.check();
  blinkLed.blink(errorTimeout.enabled() ? 1000 : 250);
}
