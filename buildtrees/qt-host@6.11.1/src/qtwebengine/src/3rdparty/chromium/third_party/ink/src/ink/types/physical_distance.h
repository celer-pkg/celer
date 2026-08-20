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

#ifndef INK_TYPES_PHYSICAL_DISTANCE_H_
#define INK_TYPES_PHYSICAL_DISTANCE_H_

#include <cmath>
#include <string>

namespace ink {

// `PhysicalDistance` is a 32-bit floating point precision type for
// representing signed "short" distances in Euclidian physical space.  This type
// should generally only be used when the maximum distance will be on the order
// of centimeters to a few meters (i.e. on the order of the size of a display
// screen).
//
// Objects of this type can represent distance of up to around 20 meters with
// precision to 1 micrometer (which is a fraction of a pixel on even the
// highest-density displays, at least as of 2024). That means adding a 1μm
// `PhysicalDistance` to another representing up to ±20m will result in a
// distinct value.
class PhysicalDistance {
 public:
  PhysicalDistance() = default;
  PhysicalDistance(const PhysicalDistance&) = default;
  PhysicalDistance& operator=(const PhysicalDistance&) = default;
  ~PhysicalDistance() = default;

  // Returns a distance representing a zero length.
  static constexpr PhysicalDistance Zero();

  static PhysicalDistance Centimeters(float cm);
  static PhysicalDistance Inches(float in);
  static PhysicalDistance Meters(float m);

  float ToCentimeters() const;
  float ToInches() const;
  float ToMeters() const;

  bool IsFinite() const;

  friend bool operator==(PhysicalDistance a, PhysicalDistance b);
  friend bool operator!=(PhysicalDistance a, PhysicalDistance b);
  friend bool operator<(PhysicalDistance a, PhysicalDistance b);
  friend bool operator>(PhysicalDistance a, PhysicalDistance b);
  friend bool operator<=(PhysicalDistance a, PhysicalDistance b);
  friend bool operator>=(PhysicalDistance a, PhysicalDistance b);

  friend PhysicalDistance operator-(PhysicalDistance d);

  friend PhysicalDistance operator+(PhysicalDistance a, PhysicalDistance b);
  friend PhysicalDistance operator-(PhysicalDistance a, PhysicalDistance b);
  friend PhysicalDistance operator*(PhysicalDistance d, float scale);
  friend PhysicalDistance operator*(float scale, PhysicalDistance d);
  friend PhysicalDistance operator/(PhysicalDistance d, float divisor);
  friend float operator/(PhysicalDistance a, PhysicalDistance b);

  friend PhysicalDistance& operator+=(PhysicalDistance& a, PhysicalDistance b);
  friend PhysicalDistance& operator-=(PhysicalDistance& a, PhysicalDistance b);
  friend PhysicalDistance& operator*=(PhysicalDistance& d, float scale);
  friend PhysicalDistance& operator/=(PhysicalDistance& d, float divisor);

  template <typename Sink>
  friend void AbslStringify(Sink& sink, PhysicalDistance duration) {
    sink.Append(duration.ToFormattedString());
  }

  template <typename H>
  friend H AbslHashValue(H h, PhysicalDistance duration) {
    return H::combine(std::move(h), duration.value_centimeters_);
  }

 private:
  static constexpr float kCentimetersPerMeter = 100;
  // See https://www.nist.gov/pml/owm/si-units-length
  static constexpr float kCentimetersPerInch = 2.54;

  explicit PhysicalDistance(float value_centimeters)
      : value_centimeters_(value_centimeters) {}

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  float value_centimeters_ = 0;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline constexpr PhysicalDistance PhysicalDistance::Zero() {
  return PhysicalDistance();
}

inline PhysicalDistance PhysicalDistance::Centimeters(float cm) {
  return PhysicalDistance(cm);
}

inline PhysicalDistance PhysicalDistance::Inches(float in) {
  return PhysicalDistance::Centimeters(in * kCentimetersPerInch);
}

inline PhysicalDistance PhysicalDistance::Meters(float m) {
  return PhysicalDistance::Centimeters(m * kCentimetersPerMeter);
}

inline float PhysicalDistance::ToCentimeters() const {
  return value_centimeters_;
}

inline float PhysicalDistance::ToInches() const {
  return ToCentimeters() / kCentimetersPerInch;
}

inline float PhysicalDistance::ToMeters() const {
  return ToCentimeters() / kCentimetersPerMeter;
}

inline bool PhysicalDistance::IsFinite() const {
  return std::isfinite(value_centimeters_);
}

inline bool operator==(PhysicalDistance a, PhysicalDistance b) {
  return a.value_centimeters_ == b.value_centimeters_;
}

inline bool operator!=(PhysicalDistance a, PhysicalDistance b) {
  return a.value_centimeters_ != b.value_centimeters_;
}

inline bool operator<(PhysicalDistance a, PhysicalDistance b) {
  return a.value_centimeters_ < b.value_centimeters_;
}

inline bool operator>(PhysicalDistance a, PhysicalDistance b) {
  return a.value_centimeters_ > b.value_centimeters_;
}

inline bool operator<=(PhysicalDistance a, PhysicalDistance b) {
  return a.value_centimeters_ <= b.value_centimeters_;
}

inline bool operator>=(PhysicalDistance a, PhysicalDistance b) {
  return a.value_centimeters_ >= b.value_centimeters_;
}

inline PhysicalDistance operator-(PhysicalDistance d) {
  return PhysicalDistance(-d.value_centimeters_);
}

inline PhysicalDistance operator+(PhysicalDistance a, PhysicalDistance b) {
  return PhysicalDistance(a.value_centimeters_ + b.value_centimeters_);
}

inline PhysicalDistance operator-(PhysicalDistance a, PhysicalDistance b) {
  return PhysicalDistance(a.value_centimeters_ - b.value_centimeters_);
}

inline PhysicalDistance operator*(PhysicalDistance d, float scale) {
  return PhysicalDistance(d.value_centimeters_ * scale);
}

inline PhysicalDistance operator*(float scale, PhysicalDistance d) {
  return d * scale;
}

inline PhysicalDistance operator/(PhysicalDistance d, float divisor) {
  return PhysicalDistance(d.value_centimeters_ / divisor);
}

inline float operator/(PhysicalDistance a, PhysicalDistance b) {
  return a.value_centimeters_ / b.value_centimeters_;
}

inline PhysicalDistance& operator+=(PhysicalDistance& a, PhysicalDistance b) {
  return a = a + b;
}

inline PhysicalDistance& operator-=(PhysicalDistance& a, PhysicalDistance b) {
  return a = a - b;
}

inline PhysicalDistance& operator*=(PhysicalDistance& d, float scale) {
  return d = d * scale;
}

inline PhysicalDistance& operator/=(PhysicalDistance& d, float divisor) {
  return d = d / divisor;
}

}  // namespace ink

#endif  // INK_TYPES_PHYSICAL_DISTANCE_H_
