#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using byte = uint8_t;

class String {
 public:
  String() = default;
  String(const char* s) : value_(s ? s : "") {}
  String(const std::string& s) : value_(s) {}
  String(const String&) = default;
  String(String&&) noexcept = default;
  String& operator=(const String&) = default;
  String& operator=(String&&) noexcept = default;
  String& operator=(const char* s) {
    value_ = s ? s : "";
    return *this;
  }
  String& operator=(const std::string& s) {
    value_ = s;
    return *this;
  }

  size_t write(uint8_t c) {
    value_.push_back(static_cast<char>(c));
    return 1;
  }
  size_t write(const uint8_t* buffer, size_t size) {
    if (!buffer || size == 0) {
      return 0;
    }
    value_.append(reinterpret_cast<const char*>(buffer), size);
    return size;
  }
  size_t write(const char* s, size_t size) {
    if (!s || size == 0) {
      return 0;
    }
    value_.append(s, size);
    return size;
  }
  size_t print(const char* s) {
    if (!s) {
      return 0;
    }
    value_.append(s);
    return std::strlen(s);
  }
  size_t print(const std::string& s) {
    value_.append(s);
    return s.size();
  }
  bool isEmpty() const { return value_.empty(); }
  const char* c_str() const { return value_.c_str(); }
  size_t length() const { return value_.length(); }
  explicit operator bool() const { return !value_.empty(); }

  bool operator==(const char* s) const { return value_ == (s ? s : ""); }
  bool operator!=(const char* s) const { return !(*this == s); }

  String& operator+=(const char* s) {
    if (s) {
      value_ += s;
    }
    return *this;
  }
  String& operator+=(char c) {
    value_.push_back(c);
    return *this;
  }

  std::string stdString() const { return value_; }

 private:
  std::string value_;
};

inline bool operator==(const char* lhs, const String& rhs) { return rhs == lhs; }
inline bool operator!=(const char* lhs, const String& rhs) { return !(lhs == rhs); }

using StringArray = std::vector<String>;

#ifndef ARDUINO
#define ARDUINO 1
#endif
