#include <TWISlave.h>
#include <BlinkLed.h>

const char ADDR = 'T';

const uint8_t BLINK_LED_PIN = 13;

BlinkLed blinkLed(BLINK_LED_PIN);

union {
  uint8_t bytes[4];
  long time;
} buf;

void twiSlaveCall(uint8_t addr, bool slaveTransmit) {
  if (slaveTransmit) {
    Serial.println(F("Transmit call"));
    buf.time = millis();
  } else 
    Serial.println(F("Receive call"));
  TWISlave.use(&buf, sizeof(buf));
}

void twiSlaveDone(uint8_t size, bool slaveTransmit) {
  if (slaveTransmit)
    Serial.print(F("Transmit done: "));
  else  
    Serial.print(F("Receive done: "));
  for (uint8_t i = 0; i < size; i++)
    Serial.print(buf.bytes[i], HEX);
  Serial.println();  
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
