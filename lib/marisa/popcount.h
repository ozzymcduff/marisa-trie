#ifndef MARISA_POPCOUNT_H_
#define MARISA_POPCOUNT_H_

#include "./base.h"

namespace marisa {

class PopCount {
 public:
  PopCount(UInt32 x) : value_(x) {
    value_ = (value_ & 0x55555555U) + ((value_ >> 1) & 0x55555555U);
    value_ = (value_ & 0x33333333U) + ((value_ >> 2) & 0x33333333U);
    value_ = (value_ + (value_ >> 4)) & 0x0F0F0F0FU;
    value_ += value_ << 8;
    value_ += value_ << 16;
  }

  operator UInt32() const {
    return lo32();
  }

  UInt32 lo8() const {
    return value_ & 0xFFU;
  }
  UInt32 lo16() const {
    return (value_ >> 8) & 0xFFU;
  }
  UInt32 lo24() const {
    return (value_ >> 16) & 0xFFU;
  }
  UInt32 lo32() const {
    return value_ >> 24;
  }

 private:
  UInt32 value_;
};

}  // namespace marisa

#endif  // MARISA_POPCOUNT_H_
