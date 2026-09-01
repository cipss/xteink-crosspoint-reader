#pragma once

#include "Print.h"

class HWCDC : public Print {
 public:
  void begin(unsigned long = 9600) {}
  void end() {}
  operator bool() const { return true; }
  size_t write(uint8_t value) override { return static_cast<size_t>(value == 0 ? 0 : 1); }
  size_t write(const uint8_t* buffer, size_t size) override {
    (void)buffer;
    return size;
  }
};

inline HWCDC Serial;

class HardwareSerial : public HWCDC {};
