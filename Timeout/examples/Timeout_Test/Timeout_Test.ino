#include <Timeout.h>

/*
  Simple LED-blinking sketch using Timeout library from haworkslibs
*/

const uint8_t LED_PIN = 13;

bool ledState;
Timeout ledTimeout(Timeout::SECOND);
Timeout serialTimeout(0);

void setup() {
  Serial.begin(57600);
  Serial.println("*** Timeout Test ***");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (ledTimeout.check()) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    // on for 3 seconds, off for 2 seconds
    ledTimeout.reset(ledState ? 3 * Timeout::SECOND : 2 * Timeout::SECOND);
  }
  // print remaining led time to serial every 500 ms
  if (serialTimeout.check()) {
    Serial.print("LED="); Serial.print(ledState, DEC);
    Serial.print(" remaining="); Serial.println(ledTimeout.remaining(), DEC);
    serialTimeout.reset(500);
  }  
}
