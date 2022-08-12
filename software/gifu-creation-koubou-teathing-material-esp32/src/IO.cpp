/**
 * @file IO.cpp
 * @brief
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2022 Kinoshita Lab. All rights reserved.
 *
 */
#include "IO.h"
#include <Arduino.h>
#include <MozziGuts.h>
#include "SerialUtility.h"
#include "Bounce2mcp.h"
#include "Bounce2.h"

namespace gifu_creation_koubou_2022_synth {

BounceMcp mcp_debouncers[Io::kNumMcpInputPinId] = {
    BounceMcp(),
    BounceMcp(),
    BounceMcp(),
    BounceMcp(),

    BounceMcp(),
    BounceMcp(),
    BounceMcp(),
    BounceMcp(),

};

Bounce2::Button bouncers[Io::kNumEspInputPinId] = {
    Bounce2::Button(),
    Bounce2::Button(),
    Bounce2::Button(),
    Bounce2::Button(),
};

const uint8_t Io::touch_pins[(int)TouchPinId::kNumTouch] = {T5, T4};
const uint8_t Io::esp_input_pins[(int)Io::EspInputPinId::kNumEspInputPinId]{
    5,
    17,
    16,
    15,
};

const uint8_t Io::analog_pins[Io::kNumAnalogPins]{
    0,
    2,
    4,
    14,
    27,
    32,
    33,
    34,
    35,
};

Io::Io() {}

void Io::setup() {
  p("Io::setup()");
  // initialize mcp
  if (!mcp.begin_I2C()) {
    Serial.println("MCP23X17 not found");
    while (true) {
      delay(1000);
      Serial.println("MCP23X17 not found");
    }
  } else {
    Serial.println("MCP23X17 found");
  }

  // initialize ESP32 pins
  for (int i = 0; i < kNumEspInputPinId; i++) {
    pinMode(esp_input_pins[i], INPUT_PULLUP);
    bouncers[i].attach(esp_input_pins[i], INPUT_PULLUP);
    bouncers[i].interval(5);
    bouncers[i].setPressedState(LOW);
  }

  // initialize mcp
  for (auto i = 0; i < kNumMCPOutput; ++i) {
    mcp.pinMode(i + 8, OUTPUT);
    mcp.digitalWrite(i, LOW);
    output_buffers = 0;
    mcp.writeGPIOB(output_buffers);
  }
  for (auto i = 0; i < kNumMCPInput; ++i) {
    mcp.pinMode(i, INPUT_PULLUP);
  }
  // attach bouncers for mcp
  for (auto i = 0; i < kNumMcpInputPinId; ++i) {
    mcp_debouncers[i].attach(mcp, i, 5);
  }

  // read and emit input status
  for (auto i = 0; i < kNumMcpInputPinId; ++i) {
    const auto pin_value = mcp.digitalRead(i);
    const auto id = mcpPin2Id(i);
    if (inputChangeCallbacks[id]) {
      if (pin_value == HIGH) {
        if (shouldMcpInputCheckRose(i)) {
          inputChangeCallbacks[id](HIGH);
        }
      } else {
        inputChangeCallbacks[id](LOW);
      }
    }
  }

  // set average value to touch pins
  touchSetCycles(0x1000 >> 5, 0x1000 >> 1);
  auto averageInput = [&](const uint8_t touch_pin) -> int {
    auto sum = 0;
    auto loop_times = 100;
    for (auto i = 0; i < loop_times; ++i) {
      sum += touchRead((int)touch_pin);
    }
    return sum / loop_times;
  };
  touch_average[(int)TouchPinId::kTouch0] = averageInput(touch_pins[(int)TouchPinId::kTouch0]);
  touch_average[(int)TouchPinId::kTouch1] = averageInput(touch_pins[(int)TouchPinId::kTouch1]);

  p("touch_average[%d] = %d\n", (int)TouchPinId::kTouch0, touch_average[(int)TouchPinId::kTouch0]);
  p("touch_average[%d] = %d\n", (int)TouchPinId::kTouch1, touch_average[(int)TouchPinId::kTouch1]);
}

int Io::getTouch(const TouchPinId id) {
  const uint8_t read = touchRead((int)touch_pins[(int)id]);
  const auto limited = std::min(touch_average[(int)id], read);
  const auto mapped = map(limited, 0, touch_average[(int)id], 127, 0);

  return mapped;
}

void Io::scanInput() {
  // scan mcp
  for (auto i = 0; i < kNumMcpInputPinId; ++i) {
    mcp_debouncers[i].update();
    const auto id = mcpPin2Id(i);
    if (mcp_debouncers[i].fell()) {
      if (id != -1) {
        if (inputChangeCallbacks[id])
          inputChangeCallbacks[id](LOW);
      }
    }
    if (shouldMcpInputCheckRose(i)) {
      if (id != 1) {
        if (mcp_debouncers[i].rose()) {
          if (inputChangeCallbacks[id])
            inputChangeCallbacks[id](HIGH);
        }
      }
    }
  }

  for (auto i = 0; i < kNumEspInputPinId; ++i) {
    bouncers[i].update();
    const auto id = espPin2Id(i);
    if (bouncers[i].fell()) {
      if (id != -1) {
        if (inputChangeCallbacks[id])
          inputChangeCallbacks[id](LOW);
      }
    }
    if (i == kEspPinSwitchTrigger) {
      if (bouncers[i].rose()) {
        if (inputChangeCallbacks[id])
          inputChangeCallbacks[id](HIGH);
      }
    }
  }
}

int Io::digitalReadMcp(const int pin) {
  return mcp.digitalRead(pin);
}
void Io::digitalWrite(const int pin, int value) {
  if (value) {
    output_buffers |= (1 << pin);
  } else {
    output_buffers &= ~(1 << pin);
  }
  mcp.writeGPIOB(output_buffers);
}
int Io::analogRead(const int index) {
  auto sum = 0.f;
  for (auto i = 0; i < 5; ++i) {
    sum += mozziAnalogRead(analog_pins[index]);
  }
  return sum / 5;
}

}  // namespace gifu_creation_koubou_2022_synth
