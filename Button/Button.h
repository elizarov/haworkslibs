/*
  Simple debounced button driver.
*/
#ifndef BUTTON_H_
#define BUTTON_H_

#include <Arduino.h>
#include <Timeout.h>

class Button {
private:
  uint8_t _pin;
  bool _state;
  Timeout _timeout;

public:
  Button(uint8_t pin);
  bool check();    // returns true on any change, must be called periodically
  operator bool() { return _state; } // return true when pressed down
};

#endif