/**
 * @file SerialUtility.h
 * @brief 
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2022 Kinoshita Lab. All rights reserved.
 *
 */

#pragma once
#ifndef SERIALUTILITY_H
#define SERIALUTILITY_H
#include <Arduino.h>

void p_(const __FlashStringHelper* fmt, ...);
#define USE_USB_SERIAL
#ifdef USE_USB_SERIAL
#define p(fmt, ...) p_(F(fmt), ##__VA_ARGS__)
#define dbg(...)                                                                   \
  do {                                                                             \
    fprintf(config::dbgOut, "[dbg] %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__); \
    fprintf(config::dbgOut, __VA_ARGS__);                                          \
    putc('\n', config::dbgOut);                                                    \
  } while (0)
#else
#define #define p(fmt, ...)
dbg(...)
#endif

#endif  // SERIALUTILITY_H
