#include "BlinkLed.h"

BlinkLed::BlinkLed(uint8_t pin) : 
  _pin(pin)
{
  pinMode(pin, OUTPUT);
}

void BlinkLed::blink(unsigned int interval) {
  unsigned long now = millis();
  if (now - _time > interval) {  
    _state = !_state;
    _time = now;
    digitalWrite(_pin, _state);
  }
}

void BlinkLed::off() {
  _time = millis();
  _state = false; // will turn on after next blink call
  digitalWrite(_pin, 0);
}
