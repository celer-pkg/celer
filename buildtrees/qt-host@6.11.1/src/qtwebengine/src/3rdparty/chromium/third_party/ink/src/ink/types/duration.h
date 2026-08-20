// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INK_TYPES_DURATION_H_
#define INK_TYPES_DURATION_H_

#include <cmath>
#include <limits>
#include <string>

#include "absl/time/time.h"

namespace ink {

// `Duration32` is a 32-bit floating point precision type for representing
// signed "short" durations of time.
//
// This type should only be used for memory savings when storing many values and
// the maximum duration will be on the order of minutes to a few hours.
//
// Objects of this type can represent durations of up to around 28 minutes with
// precision to the tenth of a millsecond. That means adding a 0.1 ms
// `Duration32` to another representing up to +/-28 minutes will result in a
// distinct value. The precision for durations between 28 minutes and 46 hours
// degrades from ~0.1 ms to ~10 ms, which begins to bump up against the delta
// between consecutive rendered frames.
//
// The type is constructible from the higher precision 96-bit `absl::Duration`
// and provides a `ToAbseil()` conversion function. However, there are no
// overloads for performing boolean comparisons and arithmetic with a mix of
// `Duration32` and `absl::Duration`. Doing so requires explicitly converting
// one of the arguments. The conversion will generally be subject to a small
// amount of lossiness due to floating point representation of fractional
// values.
//
// This type stays consistent with the behavior of `absl::Duration` when it
// comes to NaN and infinity. `ToSeconds()` and `ToMillis()` will never return
// NaN. This means construction and arithmetic operations that would result in
// NaN under IEEE 754 are avoided:
//
//   * Creating a `Duration32` from a NaN using one of the factories results in
//     a positive infinite duration.
//   * Arithmetic operations that would result in NaN under IEEE 754, instead
//     give positive or negative infinite results as follows:
//
//       Duration32 inf = Duration32::Infinite();
//       Duration32 d = ... any duration ...
//       float finf = numeric_limits<float>::infinity();
//
//        inf + d         ==   inf
//       -inf + d         ==  -inf
//        inf - d         ==   inf
//       -inf - d         ==  -inf
//        inf /  inf      ==   finf
//       -inf /  inf      ==  -finf
//        inf / -inf      ==  -finf
//       -inf / -inf      ==   finf
//        inf /  finf     ==   inf
//       -inf /  finf     ==  -inf
//        inf / -finf     ==  -inf
//       -inf / -finf     ==   inf
//        Zero() / Zero() ==   finf
//        d * nanf()      ==   d >= 0 ? inf : -inf
//        nanf() * d      ==   d >= 0 ? inf : -inf
//        d / nanf()      ==   d >= 0 ? inf : -inf
//
class Duration32 {
 public:
  Duration32() = default;
  Duration32(const Duration32&) = default;
  Duration32& operator=(const Duration32&) = default;
  ~Duration32() = default;

  explicit Duration32(absl::Duration d);

  // Returns a duration representing zero length of time.
  static Duration32 Zero();
  // Returns a positive infinite duration.
  static Duration32 Infinite();

  // The following construct durations from floating point values of seconds and
  // milliseconds, respectively. A NaN results in a positive infinite duration.
  static Duration32 Seconds(float s);
  static Duration32 Millis(float ms);

  float ToSeconds() const;
  float ToMillis() const;
  absl::Duration ToAbseil() const;

  bool IsFinite() const;

  friend bool operator==(Duration32 a, Duration32 b);
  friend bool operator!=(Duration32 a, Duration32 b);
  friend bool operator<(Duration32 a, Duration32 b);
  friend bool operator>(Duration32 a, Duration32 b);
  friend bool operator<=(Duration32 a, Duration32 b);
  friend bool operator>=(Duration32 a, Duration32 b);

  friend Duration32 operator-(Duration32 d);

  friend Duration32 operator+(Duration32 a, Duration32 b);
  friend Duration32 operator-(Duration32 a, Duration32 b);
  friend Duration32 operator*(Duration32 d, float scale);
  friend Duration32 operator*(float scale, Duration32 d);
  friend Duration32 operator/(Duration32 d, float divisor);
  friend float operator/(Duration32 a, Duration32 b);

  friend Duration32& operator+=(Duration32& a, Duration32 b);
  friend Duration32& operator-=(Duration32& a, Duration32 b);
  friend Duration32& operator*=(Duration32& d, float scale);
  friend Duration32& operator/=(Duration32& d, float divisor);

  template <typename Sink>
  friend void AbslStringify(Sink& sink, Duration32 duration) {
    sink.Append(duration.ToFormattedString());
  }

  template <typename H>
  friend H AbslHashValue(H h, Duration32 duration) {
    return H::combine(std::move(h), duration.value_seconds_);
  }

 private:
  static_assert(std::numeric_limits<float>::is_iec559);  // IEEE 754 float

  explicit Duration32(float value_seconds) : value_seconds_(value_seconds) {}

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  float value_seconds_ = 0;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline Duration32::Duration32(absl::Duration d)
    : Duration32(absl::ToDoubleSeconds(d)) {}

inline Duration32 Duration32::Zero() { return Duration32(); }

inline Duration32 Duration32::Infinite() {
  return Duration32(std::numeric_limits<float>::infinity());
}

inline Duration32 Duration32::Seconds(float s) {
  if (std::isnan(s)) return Infinite();
  return Duration32(s);
}

inline Duration32 Duration32::Millis(float ms) {
  if (std::isnan(ms)) return Infinite();
  return Duration32(0.001f * ms);
}

inline float Duration32::ToSeconds() const { return value_seconds_; }

inline float Duration32::ToMillis() const { return 1000 * value_seconds_; }

inline absl::Duration Duration32::ToAbseil() const {
  return absl::Seconds(value_seconds_);
}

inline bool Duration32::IsFinite() const {
  return std::isfinite(value_seconds_);
}

inline bool operator==(Duration32 a, Duration32 b) {
  return a.value_seconds_ == b.value_seconds_;
}

inline bool operator!=(Duration32 a, Duration32 b) {
  return a.value_seconds_ != b.value_seconds_;
}

inline bool operator<(Duration32 a, Duration32 b) {
  return a.value_seconds_ < b.value_seconds_;
}

inline bool operator>(Duration32 a, Duration32 b) {
  return a.value_seconds_ > b.value_seconds_;
}

inline bool operator<=(Duration32 a, Duration32 b) {
  return a.value_seconds_ <= b.value_seconds_;
}

inline bool operator>=(Duration32 a, Duration32 b) {
  return a.value_seconds_ >= b.value_seconds_;
}

inline Duration32 operator-(Duration32 d) {
  return Duration32(-d.value_seconds_);
}

inline Duration32 operator+(Duration32 a, Duration32 b) {
  if (!a.IsFinite()) return a;
  return Duration32(a.value_seconds_ + b.value_seconds_);
}

inline Duration32 operator-(Duration32 a, Duration32 b) {
  if (!a.IsFinite()) return a;
  return Duration32(a.value_seconds_ - b.value_seconds_);
}

inline Duration32 operator*(float scale, Duration32 d) { return d * scale; }

inline Duration32& operator+=(Duration32& a, Duration32 b) { return a = a + b; }

inline Duration32& operator-=(Duration32& a, Duration32 b) { return a = a - b; }

inline Duration32& operator*=(Duration32& d, float scale) {
  return d = d * scale;
}

inline Duration32& operator/=(Duration32& d, float divisor) {
  return d = d / divisor;
}

}  // namespace ink
#endif  // INK_TYPES_DURATION_H_
