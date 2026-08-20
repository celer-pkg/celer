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

#ifndef INK_GEOMETRY_RECT_H_
#define INK_GEOMETRY_RECT_H_

#include <algorithm>
#include <array>
#include <string>

#include "absl/log/absl_check.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink {

// This class represents an axis-aligned rectangle, a.k.a. an axis-aligned
// bounding box (AABB). It is defined by four values, at the minimum and maximum
// x- and y-values.
class Rect {
 public:
  // Constructs the Rect from (0, 0) to (0, 0).
  Rect() = default;

  Rect(const Rect&) = default;
  Rect(Rect&&) = default;
  Rect& operator=(const Rect&) = default;
  Rect& operator=(Rect&&) = default;

  // Constructs a Rect centered on the given point, with the given width and
  // height. This CHECK-fails if either width or height is less than zero.
  static Rect FromCenterAndDimensions(Point center, float width, float height) {
    ABSL_CHECK(width >= 0 && height >= 0)
        << "Cannot construct a rectangle with negative width or height";
    return Rect(center.x - width / 2, center.y - height / 2,
                center.x + width / 2, center.y + height / 2);
  }

  // Constructs the smallest Rect containing the two given points.
  static Rect FromTwoPoints(Point a, Point b) {
    return Rect(std::min(a.x, b.x), std::min(a.y, b.y), std::max(a.x, b.x),
                std::max(a.y, b.y));
  }

  // Accessors for the bounding values.
  float XMin() const { return x_min_; }
  float YMin() const { return y_min_; }
  float XMax() const { return x_max_; }
  float YMax() const { return y_max_; }

  // Returns the point at the center of the Rect.
  Point Center() const;

  // Sets the center of the Rect, preserving the width and height.
  void SetCenter(Point center);

  // Returns the width of the Rect.
  float Width() const;

  // Sets the width of the Rect, preserving the center and height. This
  // CHECK-fails if the given width is less than zero.
  void SetWidth(float width);

  // Returns half the width of the Rect.  Mathematically, this is equivalent to
  // Width() / 2, however this method comes with the additional guarantee that
  // if the bounds of the Rect are finite, then SemiWidth() will be finite
  // (whereas Width() can potentially overflow to infinity).
  float SemiWidth() const;

  // Returns the height of the Rect.
  float Height() const;

  // Sets the height of the Rect, preserving the center and width. This
  // CHECK-fails if the given height is less than zero.
  void SetHeight(float height);

  // Returns half the height of the Rect.  Mathematically, this is equivalent to
  // Height() / 2, however this method comes with the additional guarantee that
  // if the bounds of the Rect are finite, then SemiHeight() will be finite
  // (whereas Height() can potentially overflow to infinity).
  float SemiHeight() const;

  // Returns the aspect ration of the Rect, i.e. the width divided by the
  // height. This CHECK-fails if the height is zero.
  float AspectRatio() const {
    ABSL_CHECK(Height() != 0)
        << "Cannot determine the Aspect Ratio when the height is 0";
    return Width() / Height();
  }

  // Returns the area of the Rect. This will always be non-negative.
  float Area() const { return Width() * Height(); }

  // Returns the corners of the Rect. The order of the corners is:
  // (x_min, y_min), (x_max, y_min), (x_max, y_max), (x_min, y_max)
  std::array<Point, 4> Corners() const {
    return std::array<Point, 4>{Point{x_min_, y_min_}, Point{x_max_, y_min_},
                                Point{x_max_, y_max_}, Point{x_min_, y_max_}};
  }

  // Returns the segment of the Rect between the corner at `index` and the point
  // at `index` + 1 modulo 4, as per the `Corners` method. This CHECK-fails if
  // `index` is not 0, 1, 2, or 3.
  Segment GetEdge(int index) const;

  // Returns whether the given point is contained within the Rect. Points that
  // lie exactly on the Rect's boundary are considered to be contained.
  bool Contains(Point point) const {
    return x_min_ <= point.x && x_max_ >= point.x && y_min_ <= point.y &&
           y_max_ >= point.y;
  }

  // Returns whether the given Rect is contained within this Rect. Edges of the
  // given Rect that overlap with this one's boundary are considered to be
  // contained.
  bool Contains(const Rect& rect) const {
    return x_min_ <= rect.XMin() && x_max_ >= rect.XMax() &&
           y_min_ <= rect.YMin() && y_max_ >= rect.YMax();
  }

  // Expands the Rect such that sides are a distance of `offset` from their
  // previous positions. This may also be used to inset the Rect, by specifying
  // a negative value. If the inset would reduce the width or height below zero,
  // it is set to zero instead.
  void Offset(float offset) { Offset(offset, offset); }

  // Expands the Rect, as per the single argument overload, but the offset
  // distance is specified independently for each dimension.
  void Offset(float horizontal_offset, float vertical_offset) {
    x_min_ -= horizontal_offset;
    x_max_ += horizontal_offset;
    y_min_ -= vertical_offset;
    y_max_ += vertical_offset;
    if (Width() < 0) SetWidth(0);
    if (Height() < 0) SetHeight(0);
  }

  // Scales the Rect's width and height by the given value, preserving its
  // center. This CHECK-fails if the scale factor is less than zero.
  void Scale(float scale) { Scale(scale, scale); }

  // Scales the Rect's width and height by the given pair of values, preserving
  // its center. This CHECK-fails if either scale factor is less than zero.
  void Scale(float x_scale, float y_scale) {
    ABSL_CHECK(x_scale >= 0 && y_scale >= 0)
        << "Cannot scale a rectangle by a value less than 0";
    Offset((-0.5f * (1 - x_scale) * Width()),
           (-0.5f * (1 - y_scale) * Height()));
  }

  // Translates the Rect, moving its center by the given offset, and preserving
  // its width and height.
  void Translate(Vec offset) {
    x_min_ += offset.x;
    x_max_ += offset.x;
    y_min_ += offset.y;
    y_max_ += offset.y;
  }

  // Expands the Rect such that it contains the given point.
  void Join(Point point);

  // Expands the Rect such that it contains the given Rect.
  void Join(const Rect& rect);

  // Returns a Rect that contains this one and has the specified aspect ratio.
  // The returned Rect will have the same center, and one of either the width or
  // the height will be the same as this Rect, and the other will be greater
  // than or equal. This CHECK-fails if the given aspect_ratio is less than or
  // equal to zero.
  Rect ContainingRectWithAspectRatio(float aspect_ratio) const;

  // Returns a Rect that is contained within this one and has the specified
  // aspect ratio. The returned Rect will have the same center, and one of
  // either the width or the height will be the same as this Rect, and the other
  // will be less than or equal. If the specified aspect ratio is 0, the width
  // of the Rect will be 0 and its height will be the same as this one's. This
  // CHECK-fails if the given aspect_ratio is less than zero.
  Rect InteriorRectWithAspectRatio(float aspect_ratio) const;

  // Resizes the Rect, setting the specified value to the given one.
  // If setting the value would cause the extrema to flip, then
  // the other extremum is also set to the given value.
  void ResizeSettingXMinTo(float x_min);
  void ResizeSettingYMinTo(float y_min);
  void ResizeSettingXMaxTo(float x_max);
  void ResizeSettingYMaxTo(float y_max);

  // Translates the Rect, setting the specified value to the given one,
  // and preserving width and height.
  // These functions are respectively equivalent to:
  //   rect.Translate({x_min - rect.XMin(), 0});
  //   rect.Translate({0, y_min - rect.YMin()});
  //   rect.Translate({x_max - rect.XMax(), 0});
  //   rect.Translate({0, y_max - rect.YMax()});
  void TranslateSettingXMinTo(float x_min);
  void TranslateSettingYMinTo(float y_min);
  void TranslateSettingXMaxTo(float x_max);
  void TranslateSettingYMaxTo(float y_max);

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Rect& rect) {
    sink.Append(rect.ToFormattedString());
  }

 private:
  constexpr Rect(float x1, float y1, float x2, float y2) {
    x_min_ = x1;
    y_min_ = y1;
    x_max_ = x2;
    y_max_ = y2;
  }

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  float x_min_ = 0;
  float y_min_ = 0;
  float x_max_ = 0;
  float y_max_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline void Rect::SetCenter(Point center) {
  float half_width = SemiWidth();
  float half_height = SemiHeight();
  x_min_ = center.x - half_width;
  x_max_ = center.x + half_width;
  y_min_ = center.y - half_height;
  y_max_ = center.y + half_height;
}

inline float Rect::Width() const { return x_max_ - x_min_; }

inline void Rect::SetWidth(float width) {
  ABSL_CHECK(width >= 0) << "Cannot set a width less than 0";
  float x_center = (x_min_ + x_max_) / 2;
  x_min_ = x_center - width / 2;
  x_max_ = x_center + width / 2;
}

inline float Rect::Height() const { return y_max_ - y_min_; }

inline void Rect::SetHeight(float height) {
  ABSL_CHECK(height >= 0) << "Cannot set a height less than 0";
  float y_center = (y_min_ + y_max_) / 2;
  y_min_ = y_center - height / 2;
  y_max_ = y_center + height / 2;
}

// Halve before subtracting in order to avoid potential overflow.
inline float Rect::SemiWidth() const { return 0.5f * x_max_ - 0.5f * x_min_; }
inline float Rect::SemiHeight() const { return 0.5f * y_max_ - 0.5f * y_min_; }

inline void Rect::Join(Point point) {
  x_min_ = std::min(x_min_, point.x);
  x_max_ = std::max(x_max_, point.x);
  y_min_ = std::min(y_min_, point.y);
  y_max_ = std::max(y_max_, point.y);
}

inline void Rect::Join(const Rect& rect) {
  x_min_ = std::min(x_min_, rect.XMin());
  x_max_ = std::max(x_max_, rect.XMax());
  y_min_ = std::min(y_min_, rect.YMin());
  y_max_ = std::max(y_max_, rect.YMax());
}

inline void Rect::ResizeSettingXMinTo(float x_min) {
  x_min_ = x_min;
  if (x_max_ < x_min_) {
    x_max_ = x_min_;
  }
}
inline void Rect::ResizeSettingYMinTo(float y_min) {
  y_min_ = y_min;
  if (y_max_ < y_min_) {
    y_max_ = y_min_;
  }
}
inline void Rect::ResizeSettingXMaxTo(float x_max) {
  x_max_ = x_max;
  if (x_min_ > x_max_) {
    x_min_ = x_max_;
  }
}
inline void Rect::ResizeSettingYMaxTo(float y_max) {
  y_max_ = y_max;
  if (y_min_ > y_max_) {
    y_min_ = y_max_;
  }
}

inline void Rect::TranslateSettingXMinTo(float x_min) {
  Translate(Vec{x_min - x_min_, 0});
}
inline void Rect::TranslateSettingYMinTo(float y_min) {
  Translate(Vec{0, y_min - y_min_});
}
inline void Rect::TranslateSettingXMaxTo(float x_max) {
  Translate(Vec{x_max - x_max_, 0});
}
inline void Rect::TranslateSettingYMaxTo(float y_max) {
  Translate(Vec{0, y_max - y_max_});
}

}  // namespace ink

#endif  // INK_GEOMETRY_RECT_H_
