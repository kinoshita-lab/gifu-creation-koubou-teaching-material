#include "SerialUtility.h"
void p_(const __FlashStringHelper* fmt, ...) {
  char buf[128];  // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char*)fmt, args);  // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char*)fmt, args);  // for the rest of the world
#endif
  va_end(args);
  Serial.print(buf);
}