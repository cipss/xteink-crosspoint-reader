#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "Arduino.h"

class Print {
 public:
  virtual ~Print() = default;
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t* buffer, size_t size) {
    size_t written = 0;
    for (size_t i = 0; i < size; ++i) {
      written += write(buffer[i]);
    }
    return written;
  }

  size_t print(const char* text) {
    if (!text) {
      return 0;
    }
    return write(reinterpret_cast<const uint8_t*>(text), std::strlen(text));
  }
  size_t print(const String& text) {
    if (text.isEmpty()) {
      return 0;
    }
    return write(reinterpret_cast<const uint8_t*>(text.c_str()), text.length());
  }
  size_t print(const std::string& text) {
    return write(reinterpret_cast<const uint8_t*>(text.c_str()), text.length());
  }
  size_t println(const char* text = "") {
    size_t written = print(text);
    written += print("\n");
    return written;
  }
  size_t println(const String& text) {
    size_t written = print(text);
    written += print("\n");
    return written;
  }
  virtual void flush() {}
};
