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

#include "ink/geometry/internal/legacy_segment_intersection.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <utility>

#include "ink/geometry/internal/legacy_vector_utils.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace geometry_internal {

namespace {

int Exponent(float v) {
  int exp;
  std::frexp(v, &exp);
  return exp;
}

// Returns an exponent X such that we can compute the intersection of
// 2^X * segment1 and 2^X * segment2 without causing an overflow error.
int FindSafeExponentForSegmentIntersection(const Segment &segment1,
                                           const Segment &segment2) {
  // This approximation allows us to quickly determine that a segment pair will
  // not cause an overflow or underflow error. The derivation is as follows:
  //
  // The determinant of the segment vectors, where the overflow/underflow will
  // occur, is of the form (a - b)(c - d) - (e - f)(g - h). Overflow and
  // underflow errors occur in the following cases, where F_min and F_max are
  // the minimum and maximum positive float values:
  // - Any of |a - b|, |c - d|, |e - f|, or |g - h| is greater than F_max
  // - Either of |(a - b)(c - d)| or |(e - f)(g - h)| is greater than F_max
  // - Both of |(a - b)(c - d)| and |(e - f)(g - h)| are less than F_min (if
  //   only one is less than F_min, the underflow still occurs, but it does not
  //   cause an error because the other term is sufficiently large)
  // - The absolute value of the determinant, |(a - b)(c - d) - (e - f)(g - h)|,
  //   is greater than F_max
  // If we let m be the maximum absolute value of the segment endpoint's
  // components (i.e. m = max(|a|, |b|, |c|, |d|, |e|, |f|, |g|, |h|)), then we
  // can see that the following are true:
  // - 2m ≥ max(|a - b|, |c - d|, |e - f|, |g - h|)
  // - 4m² ≥ max(|(a - b)(c - d)|, |(e - f)(g - h)|)
  // - 8m² ≥ max(|(a - b)(c - d) - (e - f)(g - h)|)
  // We can then see that an overflow or underflow error can only occur if at
  // least on of the following are true:
  // - 2m ≥ F_max
  // - 4m² ≥ F_max
  // - 4m² ≤ F_min
  // - 8m² ≥ F_max
  // Therefore, if m lies in the interval [sqrt(F_min/4), sqrt(F_max/8)], then
  // no overflow or underflow error can occur.
  static const float kSafeUpperBound =
      std::sqrt(.125f * std::numeric_limits<float>::max());
  static const float kSafeLowerBound =
      std::sqrt(.25f * std::numeric_limits<float>::min());
  float max_abs_value = std::max(
      std::max(std::max(std::abs(segment1.start.x), std::abs(segment1.start.y)),
               std::max(std::abs(segment1.end.x), std::abs(segment1.end.y))),
      std::max(std::max(std::abs(segment2.start.x), std::abs(segment2.start.y)),
               std::max(std::abs(segment2.end.x), std::abs(segment2.end.y))));
  if (max_abs_value >= kSafeLowerBound && max_abs_value <= kSafeUpperBound)
    return 0;

  // The most egregious losses of precision occur either during multiplication
  // or division, however there are some edge cases where you may get an
  // overflow from adding or subtracting.
  // When adding two positive numbers (or subtracting two negative numbers), the
  // exponent of the result can only be as large as the largest exponent plus 1.
  // Fortunately, we don't have to consider underflows (going past the minimum
  // exponent) for addition and subtraction.
  int s1xExp =
      Exponent(std::max(std::abs(segment1.end.x), std::abs(segment1.start.x))) +
      1;
  int s1yExp =
      Exponent(std::max(std::abs(segment1.end.y), std::abs(segment1.start.y))) +
      1;
  int s2xExp =
      Exponent(std::max(std::abs(segment2.end.x), std::abs(segment2.start.x))) +
      1;
  int s2yExp =
      Exponent(std::max(std::abs(segment2.end.y), std::abs(segment2.start.y))) +
      1;

  int minPrecisionExponent = std::numeric_limits<float>::min_exponent;
  int maxPrecisionExponent = std::numeric_limits<float>::max_exponent;

  // The maximum exponent of a multiplication operation can only be as large as
  // the exponents of its operands added together.
  int expectedDetExponent = std::max(s2xExp + s1yExp, s1xExp + s2yExp) + 1;
  int expectedLenSquaredExponent =
      2 * std::max(std::max(s1xExp, s1yExp), std::max(s2xExp, s2yExp)) + 1;
  int expectedMaxExponent =
      std::max(expectedDetExponent, expectedLenSquaredExponent);

  int shiftExponent = 0;
  if (expectedMaxExponent > maxPrecisionExponent) {
    shiftExponent -= expectedMaxExponent - maxPrecisionExponent;
  } else if (expectedMaxExponent <= minPrecisionExponent) {
    shiftExponent -= expectedMaxExponent - minPrecisionExponent;
  }

  return shiftExponent;
}

bool SegmentIntersectionHelper(Segment segment1, Segment segment2,
                               std::array<float, 2> *seg1_interval,
                               std::array<float, 2> *seg2_interval) {
  if (segment1.start == segment2.start && segment1.end == segment2.end) {
    *seg1_interval = {{0, 1}};
    *seg2_interval = {{0, 1}};
    return true;
  } else if (segment1.start == segment2.end && segment1.end == segment2.start) {
    *seg1_interval = {{0, 1}};
    *seg2_interval = {{1, 0}};
    return true;
  }

  int exponent = FindSafeExponentForSegmentIntersection(segment1, segment2);
  if (exponent != 0) {
    segment1.start.x = std::ldexp(segment1.start.x, exponent);
    segment1.start.y = std::ldexp(segment1.start.y, exponent);
    segment1.end.x = std::ldexp(segment1.end.x, exponent);
    segment1.end.y = std::ldexp(segment1.end.y, exponent);
    segment2.start.x = std::ldexp(segment2.start.x, exponent);
    segment2.start.y = std::ldexp(segment2.start.y, exponent);
    segment2.end.x = std::ldexp(segment2.end.x, exponent);
    segment2.end.y = std::ldexp(segment2.end.y, exponent);
  }

  Vec u = segment1.end - segment1.start;
  Vec v = segment2.end - segment2.start;
  Vec w = segment2.start - segment1.start;
  float u_len_squared = Vec::DotProduct(u, u);
  float v_len_squared = Vec::DotProduct(v, v);

  if (u_len_squared == 0 && v_len_squared == 0) {
    // Both segments are degenerate -- they intersect only if they are also
    // coincident.
    if (Vec::DotProduct(w, w) == 0) {
      *seg1_interval = {{0, 1}};
      *seg2_interval = {{0, 1}};
      return true;
    } else {
      return false;
    }
  }

  bool segment1_start_is_collinear_with_segment2 =
      v_len_squared != 0 &&
      PositionRelativeToLine(segment2.start, segment2.end, segment1.start) ==
          RelativePos::kCollinear;
  bool segment1_end_is_collinear_with_segment2 =
      v_len_squared != 0 &&
      PositionRelativeToLine(segment2.start, segment2.end, segment1.end) ==
          RelativePos::kCollinear;
  bool segment2_start_is_collinear_with_segment1 =
      u_len_squared != 0 &&
      PositionRelativeToLine(segment1.start, segment1.end, segment2.start) ==
          RelativePos::kCollinear;
  bool segment2_end_is_collinear_with_segment1 =
      u_len_squared != 0 &&
      PositionRelativeToLine(segment1.start, segment1.end, segment2.end) ==
          RelativePos::kCollinear;

  Vec v_plus_w = segment2.end - segment1.start;   // v + w;
  Vec u_minus_w = segment1.end - segment2.start;  // u - w;

  // Check if the segments are parallel.
  if (u_len_squared != 0 && v_len_squared != 0 &&
      ((segment1_start_is_collinear_with_segment2 &&
        segment1_end_is_collinear_with_segment2) ||
       (segment2_start_is_collinear_with_segment1 &&
        segment2_end_is_collinear_with_segment1))) {
    (*seg1_interval)[0] = Vec::DotProduct(u, w) / u_len_squared;
    (*seg1_interval)[1] = Vec::DotProduct(u, v_plus_w) / u_len_squared;
    bool segments_travel_in_opposite_directions =
        (*seg1_interval)[1] < (*seg1_interval)[0];
    if (segments_travel_in_opposite_directions)
      std::swap((*seg1_interval)[0], (*seg1_interval)[1]);
    if ((*seg1_interval)[1] < 0 || (*seg1_interval)[0] > 1) {
      return false;
    } else if ((*seg1_interval)[0] < 0) {
      (*seg1_interval)[0] = 0;
      (*seg2_interval)[0] = Vec::DotProduct(v, -w) / v_len_squared;
    } else {
      (*seg2_interval)[0] = segments_travel_in_opposite_directions ? 1 : 0;
    }
    if ((*seg1_interval)[1] > 1) {
      (*seg1_interval)[1] = 1;
      (*seg2_interval)[1] = Vec::DotProduct(v, u_minus_w) / v_len_squared;
    } else {
      (*seg2_interval)[1] = segments_travel_in_opposite_directions ? 0 : 1;
    }
    return true;
  }

  auto populate_params = [](float segment1_param, float segment2_param,
                            std::array<float, 2> *seg1_interval,
                            std::array<float, 2> *seg2_interval) {
    (*seg1_interval)[0] = segment1_param;
    (*seg1_interval)[1] = segment1_param;
    (*seg2_interval)[0] = segment2_param;
    (*seg2_interval)[1] = segment2_param;
  };

  if (segment1_start_is_collinear_with_segment2) {
    // The start of segment1 is collinear with segment2.
    float p = Vec::DotProduct(v, -w) / v_len_squared;
    if (0 <= p && p <= 1) {
      populate_params(0, p, seg1_interval, seg2_interval);
      return true;
    }
  }
  if (segment1_end_is_collinear_with_segment2) {
    // The end of segment1 is collinear with segment2.
    float p = Vec::DotProduct(v, u_minus_w) / v_len_squared;
    if (0 <= p && p <= 1) {
      populate_params(1, p, seg1_interval, seg2_interval);
      return true;
    }
  }
  if (segment2_start_is_collinear_with_segment1) {
    // The start of segment2 is collinear with segment1.
    float p = Vec::DotProduct(u, w) / u_len_squared;
    if (0 <= p && p <= 1) {
      populate_params(p, 0, seg1_interval, seg2_interval);
      return true;
    }
  }
  if (segment2_end_is_collinear_with_segment1) {
    // The end of segment2 is collinear with segment1.
    float p = Vec::DotProduct(u, v_plus_w) / u_len_squared;
    if (0 <= p && p <= 1) {
      populate_params(p, 1, seg1_interval, seg2_interval);
      return true;
    }
  }

  float determinant = Vec::Determinant(u, v);
  if (determinant == 0) {
    // While we already checked for parallel segments, the determinant could
    // still be zero if one of the segments is degenerate.
    return false;
  }
  float segment1_param = Vec::Determinant(w, v) / determinant;
  float segment2_param = Vec::Determinant(w, u) / determinant;

  if (0 <= segment1_param && segment1_param <= 1 && 0 <= segment2_param &&
      segment2_param <= 1) {
    populate_params(segment1_param, segment2_param, seg1_interval,
                    seg2_interval);
    return true;
  } else {
    return false;
  }
}

}  // namespace

std::optional<LegacySegmentIntersection> LegacyIntersection(const Segment &a,
                                                            const Segment &b) {
  LegacySegmentIntersection intx;
  if (SegmentIntersectionHelper(a, b, &intx.segment1_interval,
                                &intx.segment2_interval)) {
    intx.intx.start = a.Lerp(intx.segment1_interval[0]);
    intx.intx.end = a.Lerp(intx.segment1_interval[1]);
    return intx;
  }
  return std::nullopt;
}

}  // namespace geometry_internal
}  // namespace ink
