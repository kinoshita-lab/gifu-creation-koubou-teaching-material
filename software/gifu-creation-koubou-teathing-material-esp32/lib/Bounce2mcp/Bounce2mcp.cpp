// Please read Bounce2.h for information about the liscence and authors

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "Bounce2mcp.h"

#define DEBOUNCED_STATE 0
#define UNSTABLE_STATE 1
#define STATE_CHANGED 3

BounceMcp::BounceMcp()
    : previous_millis(0), interval_millis(10), state(0), pin(0) {}

void BounceMcp::attach(Adafruit_MCP23X17 mcpX, int pin, uint16_t interval_millis) {
  this->pin = pin;
  this->mcpX = mcpX;
  this->interval_millis = interval_millis;

  bool read = mcpX.digitalRead(pin);
  state = 0;
  if (mcpX.digitalRead(pin)) {
    state = _BV(DEBOUNCED_STATE) | _BV(UNSTABLE_STATE);
  }
#ifdef BOUNCE_LOCK_OUT
  previous_millis = 0;
#else
  previous_millis = millis();
#endif
}

void BounceMcp::interval(uint16_t interval_millis) {
  this->interval_millis = interval_millis;
}

bool BounceMcp::update() {
#ifdef BOUNCE_LOCK_OUT
  state &= ~_BV(STATE_CHANGED);
  // Ignore everything if we are locked out
  if (millis() - previous_millis >= interval_millis) {
    bool currentState = mcpX.digitalRead(pin);
    if ((bool)(state & _BV(DEBOUNCED_STATE)) != currentState) {
      previous_millis = millis();
      state ^= _BV(DEBOUNCED_STATE);
      state |= _BV(STATE_CHANGED);
    }
  }
  return state & _BV(STATE_CHANGED);
#else
  // Read the state of the switch in a temporary variable.
  bool currentState = mcpX.digitalRead(pin);
  state &= ~_BV(STATE_CHANGED);

  // If the reading is different from last reading, reset the debounce counter
  if (currentState != (bool)(state & _BV(UNSTABLE_STATE))) {
    previous_millis = millis();
    state ^= _BV(UNSTABLE_STATE);
  } else if (millis() - previous_millis >= interval_millis) {
    // We have passed the threshold time, so the input is now stable
    // If it is different from last state, set the STATE_CHANGED flag
    if ((bool)(state & _BV(DEBOUNCED_STATE)) != currentState) {
      previous_millis = millis();
      state ^= _BV(DEBOUNCED_STATE);
      state |= _BV(STATE_CHANGED);
    }
  }

  return state & _BV(STATE_CHANGED);
#endif
}

bool BounceMcp::read() {
  return state & _BV(DEBOUNCED_STATE);
}

bool BounceMcp::rose() {
  return (state & _BV(DEBOUNCED_STATE)) && (state & _BV(STATE_CHANGED));
}

bool BounceMcp::fell() {
  return !(state & _BV(DEBOUNCED_STATE)) && (state & _BV(STATE_CHANGED));
}
