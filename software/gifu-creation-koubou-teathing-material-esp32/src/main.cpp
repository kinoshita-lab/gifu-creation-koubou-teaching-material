#include <Arduino.h>
#include <BLEMidi.h>
#include <MozziGuts.h>
#include <mozzi_midi.h>
#include <Oscil.h>  // oscillator template
#include <tables/square_analogue512_int8.h>
#include <tables/saw_analogue512_int8.h>
#include <tables/sin2048_int8.h>
#include <functional>

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#include <ADSR.h>

#include "IO.h"
#include "FakeTimerInterrupt.h"
#include "SerialUtility.h"

constexpr int SAMPLING_RATE = AUDIO_RATE;

using namespace gifu_creation_koubou_2022_synth;

#define RX (18)
#define TX (19)

DFRobotDFPlayerMini dfPlayer;
gifu_creation_koubou_2022_synth::Io io;

const uint8_t touch_pins[2] = {
    13,
    12,
};

constexpr int kNumInputPins = 4;
const uint8_t in_pins[kNumInputPins] = {
    5,
    17,
    16,
    15,
};

constexpr int kNumAnalogPins = 9;
const uint8_t analog_in_pins[kNumAnalogPins] = {
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

int beat = 0;

struct Transport {
  int bpm = 120;
  bool playing = false;
};
Transport transport;

// unusable analogread pins: 2
// unusable gpio

// modes
enum EditMode {
  kModeSeq,
  kModeEG,
  kModeLFO,
  kNumMode,
};
int mode = kModeSeq;

float getNormalizedFrequency(const int value, const int value_max, const int freq_max) {
  float normalized_freq = io.analogRead(Io::kV1) / (float)value_max;
  normalized_freq *= normalized_freq;
  normalized_freq *= (float)freq_max;

  return normalized_freq;
}

struct Synth {
  int attack = 10;
  int decay = 10;
  int sustain = 100;
  int release = 0;
};
Synth synth;

int raw_knob_values[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
int seq_freqs[8]{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
int seq_step = 0;

class WhiteNoise {
 public:
  WhiteNoise() {}
  int8_t next() {
    return (rand() % 255) - 128;
    ;
  }
};

enum {
  kOscSquare,
  kOscSaw,
  kOscNoise,
};
int osc_type = kOscSquare;

bool touch_amp_enabled = false;
bool touch_lfo_speed_enabled = false;
bool touch_lfo_depth_enabled = false;

ADSR<CONTROL_RATE, AUDIO_RATE> envelope;
Oscil<SQUARE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> squareWave(SQUARE_ANALOGUE512_DATA);
Oscil<SAW_ANALOGUE512_NUM_CELLS, AUDIO_RATE> sawWave(SAW_ANALOGUE512_DATA);
WhiteNoise whiteNoise;

// LFO
Oscil<2048, AUDIO_RATE> lfo1(SIN2048_DATA);
Oscil<2048, AUDIO_RATE> lfo2(SIN2048_DATA);
float lfo1_depth = 0.0;
float lfo2_depth = 0.0;

bool tick_flag = false;
void bpmTick() {
  if (transport.playing) {
    squareWave.setFreq(seq_freqs[seq_step]);
    sawWave.setFreq(seq_freqs[seq_step]);

    envelope.noteOff();
    envelope.noteOn(true);
    seq_step++;
    if (seq_step >= 8) {
      seq_step = 0;
    }
  }

  beat = beat % 4;
  if (beat == 0) {
    tick_flag = true;
    //beat = 0;
  }
  beat++;
}

bool out_value = false;

FakeTimerInterrupt bpmtick(SAMPLING_RATE);

SoftwareSerial swSer(RX, TX);

void setMode(const int new_mode) {
  mode = new_mode;
  io.digitalWrite(Io::kModeLFOLed, LOW);
  io.digitalWrite(Io::kModeEGLed, LOW);
  io.digitalWrite(Io::kModeSeqLed, LOW);
  switch (mode) {
    case kModeSeq:
      Serial.println("seq");
      io.digitalWrite(Io::kModeSeqLed, HIGH);

      break;
    case kModeEG:
      Serial.println("eg");
      io.digitalWrite(Io::kModeEGLed, HIGH);
      break;
    case kModeLFO:
      Serial.println("lfo");
      io.digitalWrite(Io::kModeLFOLed, HIGH);
      break;
  }
}

// switch callbacks
void onSwitchPlay(const int low_hi) {
  p("onSwitchPlay\n");
  transport.playing = !transport.playing;
  if (!transport.playing) {
    p("noteoff\n");
    envelope.noteOff();
  }

  if (transport.playing) {
    beat = 0;
    seq_step = 0;
    bpmTick();
  }

  bpmtick.start();
  io.digitalWrite(Io::kPlayLed, transport.playing);
}

int last_freq = 1000;
void onSwitchTrigger(const int low_hi) {
  p("onSwitchTrigger, low_hi = %d\n", low_hi);

  if (!low_hi) {
    squareWave.setFreq(seq_freqs[0]);
    sawWave.setFreq(seq_freqs[0]);
    envelope.noteOn();

  } else {
    envelope.noteOff();
  }
  io.digitalWrite(Io::kTriggerLed, !low_hi);
}

void onSwitchMode(const int low_hi) {
  p("onSwitchMode\n");
  auto newMode = mode + 1;
  if (newMode >= kNumMode) {
    newMode = 0;
  }
  setMode(newMode);
}

bool audioPlayerPlaying = false;
void onSwitchAudioPlayer(const int low_hi) {
  p("onSwitchAudioPlayer\n");
  audioPlayerPlaying = !audioPlayerPlaying;
  if (audioPlayerPlaying) {
    dfPlayer.loop(1);
    io.digitalWrite(Io::kAudioPlayerLed, HIGH);
  } else {
    dfPlayer.stop();
    io.digitalWrite(io.kAudioPlayerLed, LOW);
  }
}  // sw4

void selectOsc() {
  if (!io.digitalReadMcp(Io::kMcpPinPatchSaw)) {
    osc_type = kOscSaw;
  } else if (!io.digitalReadMcp(Io::kMcpPinPatchNoise)) {
    osc_type = kOscNoise;
  } else {  // dafaulting to square
    osc_type = kOscSquare;
  }
  p("osc_type = %d\n", osc_type);
}
void onPatchSaw(const int low_hi) {
  p("onPatchSaw\n");
  selectOsc();
}

void onPatchSquare(const int low_hi) {
  p("onPatchSquare\n");
  selectOsc();
}

void onPatchNoise(const int low_hi) {
  p("onPatchNoise\n");
  selectOsc();
}

void onPatchTouchAmp(const int low_hi) {
  touch_amp_enabled = !low_hi;
  p("touch_amp_enabled = %d\n", touch_amp_enabled);
}
void onPatchTouchLFOSpeed(const int low_hi) {
  touch_lfo_speed_enabled = !low_hi;
  p("touch_lfo_speed_enabled = %d\n", touch_lfo_speed_enabled);
}
void onPatchTouchLFODepth(const int low_hi) {
  touch_lfo_depth_enabled = !low_hi;
  p("touch_lfo_depth_enabled = %d\n", touch_lfo_depth_enabled);
}

void setup() {
  Serial.begin(115200);
  swSer.begin(9600);

  Serial.println("Hello!");

  io.inputChangeCallbacks[Io::kSwitchPlay] = onSwitchPlay;
  io.inputChangeCallbacks[Io::kSwitchTrigger] = onSwitchTrigger;
  io.inputChangeCallbacks[Io::kSwitchMode] = onSwitchMode;
  io.inputChangeCallbacks[Io::kSwitchAudioPlayer] = onSwitchAudioPlayer;

  // patching
  io.inputChangeCallbacks[Io::kPatchSaw] = onPatchSaw;
  io.inputChangeCallbacks[Io::kPatchSquare] = onPatchSquare;
  io.inputChangeCallbacks[Io::kPatchNoise] = onPatchNoise;

  io.inputChangeCallbacks[Io::kPatchTouchAmp] = onPatchTouchAmp;
  io.inputChangeCallbacks[Io::kPatchTouchLFOSpeed] = onPatchTouchLFOSpeed;
  io.inputChangeCallbacks[Io::kPatchTouchLFODepth] = onPatchTouchLFODepth;
  io.setup();

  setMode(kModeSeq);

  for (auto i = 0; i < 5; i++) {
    if (dfPlayer.begin(swSer)) {
      break;
      delay(500);
    }
  }

  Serial.println(F("DFPlayer Mini online."));
  dfPlayer.reset();
  dfPlayer.volume(30);

  delay(1000);

  startMozzi(CONTROL_RATE);
  squareWave.setFreq(1000);
  sawWave.setFreq(1000);

  envelope.setAttackTime(0);
  envelope.setAttackLevel(255);
  envelope.setDecayLevel(255);
  envelope.setReleaseLevel(1);
  envelope.setDecayTime(0);
  envelope.setSustainLevel(255);
  envelope.setSustainTime(UINT32_MAX);
  envelope.setReleaseTime(0);
  bpmtick.setCallback(bpmTick);

  bpmtick.setCallback(bpmTick);
  bpmtick.setIntervalMsec(250);
}

auto last_analog_value = 0;
uint8_t last_touch_value[2] = {0, 0};
bool onOff = false;
void setSeqFreq(const int index, const int freq) {
  if (index >= 8) {
    return;
  }

  seq_freqs[index] = freq;
}
int analog_read_index = 0;
void updateControl() {
  envelope.update();
  io.scanInput();
  last_touch_value[0] = io.getTouch(Io::TouchPinId::kTouch0);
  last_touch_value[1] = io.getTouch(Io::TouchPinId::kTouch1);
  if (touch_lfo_speed_enabled) {
    const auto lfo_freq = last_touch_value[0] >> 2;
    lfo1.setFreq(lfo_freq);
  }
  if (touch_lfo_depth_enabled) {
    lfo1_depth = last_touch_value[1] / 127.0;
  }
  io.digitalWrite(Io::kBpmLed, tick_flag);
  if (tick_flag) {
    tick_flag = 0;
  }

  auto value = io.analogRead(analog_read_index) >> 5;
  if (analog_read_index == 0) {
    value = map(value, 19, 127, 0, 127);
  }
  if (value != raw_knob_values[analog_read_index]) {
    raw_knob_values[analog_read_index] = value;
    //p("raw_knob_values[%d] = %d\n", i, value);
    switch (mode) {
      case kModeSeq: {
        const auto freq = mtof(value);
        setSeqFreq(analog_read_index, freq);
        if (!transport.playing) {
          const auto freq = mtof(raw_knob_values[0]);
          squareWave.setFreq(freq);
          sawWave.setFreq(freq);
          last_freq = freq;
        }
      } break;
      case kModeEG: {
        if (analog_read_index == 0) {
          const auto attack = value << 5;
          p("attack = %d\n", attack);
          envelope.setAttackTime(attack);
        }
        if (analog_read_index == 1) {
          const auto decay = value << 5;
          p("decay = %d\n", decay);
          envelope.setDecayTime(value << 5);
        }
        if (analog_read_index == 2) {
          const auto sustain = value << 1;
          p("sustain = %d\n", sustain);
          envelope.setDecayLevel(sustain);
          envelope.setSustainLevel(sustain);
        }
        if (analog_read_index == 3) {
          const auto release = value << 5;
          envelope.setReleaseTime(release);
        }

      } break;
      case kModeLFO:
        if (analog_read_index == 0) {
          if (!touch_lfo_speed_enabled) {
            const auto lfo_freq = value >> 2;
            p("lfo_freq = %d\n", lfo_freq);
            lfo1.setFreq(lfo_freq);
          }
        }
        if (analog_read_index == 1) {
          if (!touch_lfo_depth_enabled) {
            lfo1_depth = value / 127.0;
            p("lfo_depth = %f\n", lfo1_depth);
          }
        }
        if (analog_read_index == 2) {
          const auto lfo2_freq = value >> 2;
          p("lfo2_freq = %d\n", lfo2_freq);
          lfo2.setFreq(lfo2_freq);
        }
        if (analog_read_index == 3) {
          lfo2_depth = value / 127.0;
          p("lfo2_depth = %f\n", lfo2_depth);
        }
        break;
      default:
        break;

        // update triggered pitch
    }

    // update bpm
    if (analog_read_index == 8) {
      auto bpm = io.analogRead(Io::kV9) >> 5;
      bpm = map(bpm, 0, 4096 >> 5, 30, 240);
      const auto absDelta = abs(bpm - transport.bpm);
      if (absDelta >= 2) {
        transport.bpm = bpm;
        bpmtick.setIntervalMsec(60000 / transport.bpm / 4);
        //p("BPM: %d\n", transport.bpm);
      }
    }
  }
  analog_read_index++;
  analog_read_index = analog_read_index % 9;
}

int8_t AM_modulate(int8_t carrier, int8_t modulation, int8_t depth) {
  if (depth < 1) {
    return carrier;
  }
  int16_t sample = (int16_t)carrier;                    // carrier is signed
  uint16_t am = (uint16_t)((int16_t)modulation + 128);  // unsigned modulation 0..255
  uint16_t dp = (uint16_t)((int16_t)depth + 128);       // unsigned depth 0..255

  return (int8_t)((sample * (((am * dp) >> 8) + (255 - dp))) >> 8);
}
int updateAudio() {
  auto get_output = [](Q15n16 pitch_mod) -> int8_t {
    switch (osc_type) {
      case kOscSaw:
        return sawWave.phMod(pitch_mod);
      case kOscNoise:
        return whiteNoise.next();
      case kOscSquare:  // FALLTHRU
      default:
        return squareWave.phMod(pitch_mod);
    }
  };
  bpmtick.tick();
  Q15n16 pitch_lfo_out = (Q15n16)(lfo2.next() * ((int)(lfo2_depth * 255)));

  auto out = get_output(pitch_lfo_out);
  if (touch_amp_enabled) {
    out = (out * last_touch_value[0]) >> 7;
  }

  auto lfo1_out = lfo1.next();
  auto am_modulated = AM_modulate(out, lfo1_out, (int)(lfo1_depth * 127));
  return (int)(envelope.next() * am_modulated) >> 8;
}

void loop() {
  audioHook();
}