/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cmath>
#include <limits>
#include "Yoga-internal.h"

struct YGFloatOptional {
private:
  float value_ = std::numeric_limits<float>::quiet_NaN();

public:
  explicit constexpr YGFloatOptional(float value) : value_(value) {}
  constexpr YGFloatOptional() = default;

  // returns the wrapped value, or a value x with YGIsUndefined(x) == true
  constexpr float unwrap() const { return value_; }

  bool isUndefined() const { return std::isnan(value_); }
};

// operators take YGFloatOptional by value, as it is a 32bit value

inline bool operator==(YGFloatOptional lhs, YGFloatOptional rhs) {
  return lhs.unwrap() == rhs.unwrap() ||
      (lhs.isUndefined() && rhs.isUndefined());
}
inline bool operator!=(YGFloatOptional lhs, YGFloatOptional rhs) {
  return !(lhs == rhs);
}

inline bool operator==(YGFloatOptional lhs, float rhs) {
  return lhs == YGFloatOptional{rhs};
}
inline bool operator!=(YGFloatOptional lhs, float rhs) {
  return !(lhs == rhs);
}

inline bool operator==(float lhs, YGFloatOptional rhs) {
  return rhs == lhs;
}
inline bool operator!=(float lhs, YGFloatOptional rhs) {
  return !(lhs == rhs);
}

inline YGFloatOptional operator+(YGFloatOptional lhs, YGFloatOptional rhs) {
  return YGFloatOptional{lhs.unwrap() + rhs.unwrap()};
}

inline bool operator>(YGFloatOptional lhs, YGFloatOptional rhs) {
  return lhs.unwrap() > rhs.unwrap();
}

inline bool operator<(YGFloatOptional lhs, YGFloatOptional rhs) {
  return lhs.unwrap() < rhs.unwrap();
}

inline bool operator>=(YGFloatOptional lhs, YGFloatOptional rhs) {
  return lhs > rhs || lhs == rhs;
}

inline bool operator<=(YGFloatOptional lhs, YGFloatOptional rhs) {
  return lhs < rhs || lhs == rhs;
}
