/**
 * @file IO.h
 * @brief 
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2022 Kinoshita Lab. All rights reserved.
 *
 */
#pragma once
#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>
#include <functional>
#include <array>

namespace gifu_creation_koubou_2022_synth {

class Io {
 public:
  Io();
  virtual ~Io() = default;
  void setup();

  enum class TouchPinId {
    kTouch0,  // pin 12,
    kTouch1,  // pin 13,
    kNumTouch,
  };
  static const uint8_t touch_pins[(int)TouchPinId::kNumTouch];

  int getTouch(const TouchPinId id);

  enum InputPin {
    kSwitchPlay,         // SW1, pin 5
    kSwitchTrigger,      // SW2, pin 17
    kSwitchMode,         // SW5, MCP GPA6
    kSw3,                // SW3, pin 16
    kSwitchAudioPlayer,  // SW4, pin 15

    kPatchSaw,     // MCP GPA0
    kPatchSquare,  // MCP GPA1
    kPatchNoise,   // MCP GPA2

    kPatchTouchAmp,       // MCP GPA3
    kPatchTouchLFOSpeed,  // MCP GPA4
    kPatchTouchLFODepth,  // MCP GPA5

    kSw6,  // SW6, MCP GPA7,

    kNumInputPins,

  };
  enum EspInputPinId {
    kEspPinSwitchPlay,     // SW1, pin 5
    kEspPinSwitchTrigger,  // SW2, pin 17
    kEspPinSw3,            // SW3, pin 16
    kEspPinSw4,            // SW4, pin 15
    kNumEspInputPinId,
  };
  static const uint8_t esp_input_pins[(int)EspInputPinId::kNumEspInputPinId];
  enum McpInputPinId {
    kMcpPinPatchSaw,            // MCP GPA0
    kMcpPinPatchSquare,         // MCP GPA1
    kMcpPinPatchNoise,          // MCP GPA2
    kMcpPinPatchTouchAmp,       // MCP GPA3
    kMcpPinPatchTouchLFOSpeed,  // MCP GPA4
    kMcpPinPathTouchLFODepth,   // MCP GPA5
    kMcpPinSwMode,              // MCP GPA6
    kMcpPinSw6,                 // SW6, MCP GPA7,
    kNumMcpInputPinId,
  };
  std::array<std::function<void(const int low_hi)>, kNumInputPins> inputChangeCallbacks;
  // call it from updateControl
  void scanInput();

  int digitalReadMcp(const int pin);

  enum OutputPinId {
    kAudioPlayerLed,
    kReservedLed,
    kModeLFOLed,
    kModeEGLed,
    kModeSeqLed,
    kTriggerLed,
    kPlayLed,
    kBpmLed,
    kNumOutputPins,
  };
  void digitalWrite(const int pin, int value);

  enum AnalogPinId {
    kV1,  // pin 0
    kV2,  // pin 2
    kV3,  // pin 4
    kV4,  // pin 14
    kV5,  // pin 27
    kV6,  // pin 32
    kV7,  // pin 33
    kV8,  // pin 34
    kV9,  // pin 35
    kNumAnalogPins,
  };
  static const uint8_t analog_pins[kNumAnalogPins];
  int analogRead(const int id);

  static const int kNumMCPInput = 8;
  static const int kNumMCPOutput = 8;

 protected:
  Adafruit_MCP23X17 mcp;
  uint8_t input_buffers[kNumMCPInput];
  uint8_t output_buffers;
  uint8_t touch_average[(int)TouchPinId::kNumTouch];

  bool shouldMcpInputCheckRose(const int id) {
    switch (id) {
      case kMcpPinPatchSaw:
      case kMcpPinPatchSquare:
      case kMcpPinPatchNoise:
      case kMcpPinPatchTouchAmp:
      case kMcpPinPatchTouchLFOSpeed:
      case kMcpPinPathTouchLFODepth:
        return true;
      default:
        return false;
    }
  };

  int mcpPin2Id(const int pin) {
    switch (pin) {
      case kMcpPinPatchSaw:  // MCP GPA0
        return kPatchSaw;
      case kMcpPinPatchSquare:  // MCP GPA1
        return kPatchSquare;
      case kMcpPinPatchNoise:  // MCP GPA2
        return kPatchNoise;
      case kMcpPinPatchTouchAmp:  // MCP GPA3
        return kPatchTouchAmp;
      case kMcpPinPatchTouchLFOSpeed:  // MCP GPA4
        return kPatchTouchLFOSpeed;
      case kMcpPinPathTouchLFODepth:  // MCP GPA5
        return kPatchTouchLFODepth;
      case kMcpPinSwMode:  // MCP GPA6
        return kSwitchMode;
      case kMcpPinSw6:  // SW6, MCP GPA7,
        return kSw6;
      default:
        return -1;
    }
  };
  int espPin2Id(const int pin) {
    switch (pin) {
      case kEspPinSwitchPlay:  //
        return kSwitchPlay;
      case kEspPinSwitchTrigger:
        return kSwitchTrigger;
      case kEspPinSw3:
        return kSw3;
      case kEspPinSw4:
        return kSwitchAudioPlayer;
      default:
        return -1;
    }
  };
};
}  // namespace gifu_creation_koubou_2022_synth

#endif  // IO_H
