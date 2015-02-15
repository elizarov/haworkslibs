#include "Button.h"

const int DEBOUNCE = 50; // 50 ms

Button::Button(uint8_t pin) :
  _pin(pin) 
{
  pinMode(pin, INPUT); // just in case
  digitalWrite(pin, 1); // pullup
}

bool Button::check() {
  bool state = digitalRead(_pin) == 0;
  if (state == _state) {
    _timeout.disable();
    return false;
  }
  if (!_timeout.enabled()) {
    _timeout.reset(DEBOUNCE);
    return false;
  }
  if (!_timeout.check())
    return false;
  _state = state;
  return true;
}

