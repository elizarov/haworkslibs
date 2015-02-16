#include "Button.h"

const int DEBOUNCE_INTERVAL = 50; // 50 ms

Button::Button(uint8_t pin) :
  _pin(pin),
  _state(UP) 
{
  pinMode(pin, INPUT); // just in case
  digitalWrite(pin, 1); // pullup
}

bool Button::check() {
  if (digitalRead(_pin) == 0) {
    // button is down
    switch (_state) {
    case UP:
      _time = millis(); 
      _state = DEBOUNCE;
      break;
    case DEBOUNCE:
      if (long(millis() - _time) >= DEBOUNCE_INTERVAL) {
        _state = DOWN;
        return true; // pressed state changed
      }
    case DOWN: 
      break; // nothing to do
    }
  } else {
    // button is up
    switch (_state) {
    case UP:
      break; // nothing to do
    case DEBOUNCE:
      _time = 0; // was down for too short, forget
      _state = UP;   
      break;
    case DOWN:
      _state = UP; 
      _time = millis() - _time; // remember how long it was pressed
      return true; // pressed state changed
    }
  }
  return false; // no state change
}

long Button::pressed() {
  return _state == DOWN ? long(millis() - _time) : 0;
}

long Button::released() {
  if (_state != UP)
    return 0; // not released
  long duration = _time;
  _time = 0; // report release time just once
  return duration;
}
