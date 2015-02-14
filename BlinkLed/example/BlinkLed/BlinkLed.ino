#include <BlinkLed.h>

const uint8_t BLINK_LED_PIN = 13;

BlinkLed blinkLed(BLINK_LED_PIN);

void setup() {
}

void loop() {
  blinkLed.blink(1000);
}
