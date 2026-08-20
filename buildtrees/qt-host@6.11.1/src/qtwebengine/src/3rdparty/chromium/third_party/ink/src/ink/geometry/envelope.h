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

#ifndef INK_GEOMETRY_ENVELOPE_H_
#define INK_GEOMETRY_ENVELOPE_H_

#include <array>
#include <optional>
#include <string>

#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {

// This class represents the minimum bounding rectangle of zero or more objects.
// It is effectively a wrapper around a std::optional<Rect>, with convenience
// functions for expanding to include other objects.
class Envelope {
 public:
  // Constructs an empty envelope.
  Envelope() = default;

  // Constructs an envelope from the bounds of the given object.
  explicit Envelope(Point p) { Add(p); }
  explicit Envelope(const Segment& s) { Add(s); }
  explicit Envelope(const Triangle& t) { Add(t); }
  explicit Envelope(const Rect& r) { Add(r); }
  explicit Envelope(const Quad& q) { Add(q); }

  // Constructs an envelope from the bounds of the objects in the
  // iterator range [begin, end).
  // `Iterator` must be an input iterator over one of the types for
  // which `Add` is defined.
  template <typename Iterator>
  explicit Envelope(Iterator begin, Iterator end) {
    Add(begin, end);
  }

  // Constructs an envelope from the bounds of the objects in the
  // container.
  // `Container` must a container that holds objects of one of the types
  // for which `Add` is defined.
  // This is equivalent to:
  //   Envelope(container.begin(), container.end());
  template <typename Container>
  explicit Envelope(const Container& container) {
    Add(container);
  }

  Envelope(const Envelope&) = default;
  Envelope(Envelope&&) = default;
  Envelope& operator=(const Envelope&) = default;
  Envelope& operator=(Envelope&&) = default;

  // Returns the accumulated bounding rectangle of the objects that have been
  // added to the envelope since it was constructed or reset, or std::nullopt
  // if no objects have been added.
  const std::optional<Rect>& AsRect() const { return rect_; }

  // Returns true if the envelope is empty. This is equivalent to:
  //   !envelope.Bounds().has_value();
  bool IsEmpty() const { return !rect_.has_value(); }

  // Clears the accumulated bounding rectangle.
  void Reset() { rect_ = std::nullopt; }

  // Expands the accumulated bounding rectangle (if necessary) such that it also
  // contains the given object.
  void Add(Point p);
  void Add(const Rect& r);
  void Add(const Segment& s);
  void Add(const Triangle& t);
  void Add(const Quad& q);
  void Add(const Envelope& e);

  // Adds the objects in the iterator range [begin, end) to the envelope.
  // `Iterator` must be an input iterator over one of the types for which `Add`
  // is defined.
  template <typename Iterator>
  void Add(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) Add(*it);
  }

  // Adds the objects in the container to the envelope.
  // `Container` must a container that holds objects of one of the types for
  // which `Add` is defined. This is equivalent to:
  //   envelope.Add(container.begin(), container.end());
  template <typename Container>
  void Add(const Container& container) {
    Add(container.begin(), container.end());
  }

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Envelope& envelope) {
    sink.Append(envelope.ToFormattedString());
  }

 private:
  std::string ToFormattedString() const;

  std::optional<Rect> rect_ = std::nullopt;
};

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline void Envelope::Add(Point p) {
  if (IsEmpty()) {
    rect_ = Rect::FromCenterAndDimensions(p, 0, 0);
  } else {
    rect_->Join(p);
  }
}

inline void Envelope::Add(const Segment& s) {
  if (IsEmpty()) {
    rect_ = Rect::FromTwoPoints(s.start, s.end);
  } else {
    rect_->Join(s.start);
    rect_->Join(s.end);
  }
}

inline void Envelope::Add(const Triangle& t) {
  if (IsEmpty()) {
    rect_ = Rect::FromTwoPoints(t.p0, t.p1);
    rect_->Join(t.p2);
  } else {
    rect_->Join(t.p0);
    rect_->Join(t.p1);
    rect_->Join(t.p2);
  }
}

inline void Envelope::Add(const Rect& r) {
  if (IsEmpty()) {
    rect_ = r;
  } else {
    rect_->Join(r);
  }
}

inline void Envelope::Add(const Quad& q) {
  std::array<Point, 4> corners = q.Corners();
  if (IsEmpty()) {
    rect_ = Rect::FromTwoPoints(corners[0], corners[1]);
    rect_->Join(corners[2]);
    rect_->Join(corners[3]);
  } else {
    for (Point p : corners) {
      rect_->Join(p);
    }
  }
}

inline void Envelope::Add(const Envelope& e) {
  if (e.IsEmpty()) {
    return;
  }
  if (IsEmpty()) {
    rect_ = e.AsRect();
  } else {
    rect_->Join(e.AsRect().value());
  }
}

}  // namespace ink

#endif  // INK_GEOMETRY_ENVELOPE_H_
