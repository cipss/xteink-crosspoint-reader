#pragma once

#include <Print.h>

class HalFile : public Print {
 public:
  HalFile() = default;
  ~HalFile() override = default;
  size_t write(uint8_t b) override { return static_cast<size_t>(b != 0); }
  size_t write(const uint8_t* buffer, size_t size) override { (void)buffer; return size; }
  void flush() override {}
};
