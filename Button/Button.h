/*
  Simple debounced button driver.
  Button is connected between a specified pin and a ground. 
  Internull pullup on the pin is used.

  Basic usage:

    Button button(pin);

  Must periodically call for debouncing logic:

    button.check();

  Can use result of the check call to determine when state changes.

  For a simple code that depends on up / down state of a button, do:

    if (button) {
      // a code that is invoked when button is pressed down
    }

  To detect long-press of a button while the button is still pressed,
  use the following code:

    if (button.pressed() >= THRESHOLD) {
      // do something when button was pressed longer that THRESHOLD
    }

  To detect button release and make decision based on its press time,
  use the following code (can call release only once, thus use a variable):

    long time = button.released();
    if (time >= THRESHOLD) {
      // that was long press
    } else if (time >= 0) {
      // was released in time below THRESHOLD
    }

  Author: Roman Elizarov
*/

#ifndef BUTTON_H_
#define BUTTON_H_

#include <Arduino.h>

class Button {
public:
  Button(uint8_t pin);
  bool check();        // returns true on any change, must be called periodically
  operator bool();     // returns true when in pressed state
  long pressed();      // returns number >= 0 when pressed down indicating how long it is being pressed
  long released();     // returns number >= 0 only once on pressed / released transition indicating how long it was pressed

private:
  enum State { UP, DEBOUNCE, DOWN };

  uint8_t _pin;
  State _state;
  long _time;
};

// ------------ short method implementations are inline here ------------

inline Button::operator bool() {
  return _state == DOWN;
}

#endif
