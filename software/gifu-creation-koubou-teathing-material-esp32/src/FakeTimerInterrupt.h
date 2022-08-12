/**
 * @file .clang-format
 * @brief 
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2022 Kinoshita Lab. All rights reserved.
 *
 */

#pragma once
#ifndef FAKETIMERINTERRUPT_H
#define FAKETIMERINTERRUPT_H
#include <stdint.h>
#include <functional>

class FakeTimerInterrupt {
 public:
  FakeTimerInterrupt(const float sampling_rate) : sampling_rate_(sampling_rate) {
  }
  void setCallback(std::function<void()> f) {
    callback = f;
  }
  void tick() {
    if (max_count_ == 0) {
      return;
    }

    count_++;
    if (count_ < max_count_) {
      return;
    }
    //Serial.println("setcallback::timer exceeded");
    count_ = 0;

    if (!callback) {
      return;
    }

    callback();
  }

  void setIntervalMsec(const size_t msec) {
    max_count_ = (msec * sampling_rate_) / 1000.f;
  }

  void start() {
    count_ = 0;
  }

 protected:
  size_t count_ = 0;
  size_t max_count_ = 0;
  size_t sampling_rate_ = 0;
  std::function<void()> callback = nullptr;

 private:
  FakeTimerInterrupt(const FakeTimerInterrupt&) {}
};

#endif  // FAKETIMERINTERRUPT_H
