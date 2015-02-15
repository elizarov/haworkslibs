/*
  A library that simplified embedding of blinking LED into the project.
*/

#ifndef BLINK_LED_H_
#define BLINK_LED_H_

#include <Arduino.h>

class BlinkLed {
public:
  BlinkLed(uint8_t pin);
  void blink(unsigned int interval);
  void off();

private:
  uint8_t       _pin;
  unsigned long _time;
  boolean       _state;
};

#endif /* BLINK_LED_H_ */
