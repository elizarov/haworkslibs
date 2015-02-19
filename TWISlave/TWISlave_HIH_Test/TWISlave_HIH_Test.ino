/*
  Test code for TWISlave combined with Honeywell HumidIcon HIHxxx-020 I2C temperature and humidity sensor on the same bus.
  This sketch prints measurements to serial and blinks led on each one.
*/

#include <Timeout.h>
#include <HIH.h>
#include <TWIMaster.h>
#include <TWISlave.h>
#include <FixNum.h>

const char ADDR = 'T';

const uint8_t LED_PIN = 13;

HIH hih;
Timeout stateTimeout(1000);
Timeout ledTimeout;

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
    Serial.print(F("Transmit done:"));
  else  
    Serial.print(F("Receive done:"));
  for (uint8_t i = 0; i < size; i++) {
    Serial.print(' ');
    Serial.print(buf.bytes[i], HEX);
  }  
  Serial.println();  
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(57600);
  Serial.println(F("=== TWISlave + HIH Test ==="));
  Serial.print(F("Listening on TWI address ")); Serial.println(ADDR, HEX);
  TWISlave.begin(ADDR);
}

void loop() {
  if (hih.check()) {
    Serial.println(F("--- TWISlave + HIH Test ---"));
    HIH::temp_t temp = hih.getTemp();
    HIH::rh_t rh = hih.getRH();
    Serial.print(F("T = ")); Serial.print(temp.format()); Serial.println(F(" Celcius"));
    Serial.print(F("H = ")); Serial.print(rh.format());   Serial.println(F(" %"));
    digitalWrite(LED_PIN, 1);
    ledTimeout.reset(hih.getTemp() ? 1000 : 250);
  }
  // Turn off led after timeout
  if (ledTimeout.check())
    digitalWrite(LED_PIN, 0);
  // Print state periodically if there an error
  if (stateTimeout.check()) {
    if (!hih.getTemp()) {
      Serial.print(F("HIH State = ")); Serial.println(hih.getState(), HEX);
    }
    stateTimeout.reset(1000);
  }
}
