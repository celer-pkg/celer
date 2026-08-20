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

#include "ink/strokes/internal/brush_tip_shape.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/circular_extrusion_helpers.h"
#include "ink/strokes/internal/extrusion_points.h"
#include "ink/types/small_array.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::Circle;
using ::ink::geometry_internal::SegmentIntersection;

// Calculates the radius for the shape control circles. This will return
// - 0 if applying the percent_radius to the shorter side would result in the
//   circles not being min_nonzero_radius_and_separation apart
// - The value obtained from applying percent_radius to the shorter side
//   otherwise.
// TODO(b/279163840): Enable non-uniform radii for the perimeter circles.
float CalculateCircleRadius(float percent_radius, float half_width,
                            float half_height,
                            float min_nonzero_radius_and_separation) {
  float min_half_dimension = std::min(half_width, half_height);
  float unmodified_radius = percent_radius * min_half_dimension;

  // This will result in the shape control circles to be points.
  if (unmodified_radius < min_nonzero_radius_and_separation) return 0;

  return unmodified_radius;
}

SmallArray<Circle, 4> MakeShapeControlCircles(
    const BrushTipState& tip_state, float min_nonzero_radius_and_separation) {
  ABSL_CHECK_GE(tip_state.width, 0);
  ABSL_CHECK_GE(tip_state.height, 0);
  ABSL_CHECK_GE(tip_state.percent_radius, 0);
  ABSL_CHECK_LE(tip_state.percent_radius, 1);
  ABSL_CHECK_GE(tip_state.pinch, 0);
  ABSL_CHECK_LE(tip_state.pinch, 1);

  float half_width = 0.5f * tip_state.width;
  float half_height = 0.5f * tip_state.height;
  float radius =
      CalculateCircleRadius(tip_state.percent_radius, half_width, half_height,
                            min_nonzero_radius_and_separation);

  // The x and y positions of the first control circle relative to the
  // `tip_state.position` prior to applying tip slant and rotation:
  float x = half_width - radius;
  float y = half_height - radius;

  // If the separation between circle centers would be less than the
  // threshold, we combine the centers.
  if (2 * x < min_nonzero_radius_and_separation) x = 0;
  if (2 * y < min_nonzero_radius_and_separation) y = 0;

  // If both `x` and `y` are zero, the shape is a single circle. There is no
  // need to apply slant and rotation due to symmetry.
  if (x == 0 && y == 0) {
    return {Circle(tip_state.position, radius)};
  }

  AffineTransform slant = AffineTransform::Rotate(tip_state.slant);
  AffineTransform rotate_and_translate =
      AffineTransform::Translate(tip_state.position.Offset()) *
      AffineTransform::Rotate(tip_state.rotation);
  auto make_circle = [&rotate_and_translate, &slant,
                      radius](Point circle_center) {
    Point center_with_slant =
        slant.Apply(Point{0, circle_center.y}) + Vec{circle_center.x, 0};
    return Circle(rotate_and_translate.Apply(center_with_slant), radius);
  };

  // When exactly one of `x` and `y` is zero, the shape is a stadium made with
  // only two circles:
  if (y == 0) {
    return {
        make_circle({x, 0}),
        make_circle({-x, 0}),
    };
  }
  if (x == 0) {
    return {
        make_circle({0, y}),
        make_circle({0, -y}),
    };
  }

  // The value of `x` after applying `tip_state.pinch`, which moves closer
  // together the two control circles that lie below the x-axis before rotation.
  float x_after_pinch = (1 - tip_state.pinch) * x;

  // If `x_after_pinch` falls below the minimum separatation, the shape should
  // be a rounded-triangle:
  if (2 * x_after_pinch < min_nonzero_radius_and_separation) {
    return {
        make_circle({x, y}),
        make_circle({-x, y}),
        make_circle({0, -y}),
    };
  }

  // The shape uses all four control circles for a rounded trapezoid.
  return {
      make_circle({x, y}),
      make_circle({-x, y}),
      make_circle({-x_after_pinch, -y}),
      make_circle({x_after_pinch, -y}),
  };
}

}  // namespace

BrushTipShape::BrushTipShape(const BrushTipState& tip_state,
                             float min_nonzero_radius_and_separation)
    : center_(tip_state.position),
      circles_(MakeShapeControlCircles(
          tip_state, std::max(min_nonzero_radius_and_separation,
                              std::numeric_limits<float>::min()))) {}

Point BrushTipShape::Center() const { return center_; }

absl::Span<const geometry_internal::Circle> BrushTipShape::PerimeterCircles()
    const {
  return circles_.Values();
}

namespace {

// Returns the left-right pair of indices into `tip_shape.PerimeterCircles()`
// for the points that are farthest from `tip_shape.Center()` opposite the given
// `forward` direction.
std::pair<int, int> GetRearIndices(const BrushTipShape& tip_shape,
                                   Vec forward) {
  absl::Span<const Circle> circles = tip_shape.PerimeterCircles();

  int left_index = 0;
  int right_index = 0;
  Vec offset = circles[0].Center() - tip_shape.Center();
  float rear_dot_product = Vec::DotProduct(forward, offset);
  float left_determinant = Vec::Determinant(forward, offset);
  float right_determinant = left_determinant;

  for (size_t i = 1; i < circles.size(); ++i) {
    offset = circles[i].Center() - tip_shape.Center();
    float dot_product = Vec::DotProduct(forward, offset);
    float determinant = Vec::Determinant(forward, offset);
    if (dot_product < rear_dot_product) {
      left_index = i;
      right_index = i;
      rear_dot_product = dot_product;
      left_determinant = determinant;
      right_determinant = determinant;
    } else if (dot_product == rear_dot_product) {
      if (determinant > left_determinant) {
        left_index = i;
        left_determinant = determinant;
      } else if (determinant < right_determinant) {
        right_index = i;
        right_determinant = determinant;
      }
    }
  }

  return {left_index, right_index};
}

// Returns properties of the tangent between two circles on the given `side`
// with respect to a travel `direction`. If no tangents exist (i.e. because one
// circle contains the other), this returns `std::nullopt`.
struct TangentProperties {
  // Signed angle (-π, π] of a turn traveling from an incoming point in the
  // given `direction` to the tangent point on the `start` circle and the
  // traveling toward the end of the tangent on the `end` circle.
  Angle turn_angle;
  float length;
};
enum class TangentSide { kLeft, kRight };
std::optional<TangentProperties> CalculateTangentProperties(const Circle& start,
                                                            const Circle& end,
                                                            Vec direction,
                                                            TangentSide side) {
  std::optional<Circle::TangentAngles> angles = start.GetTangentAngles(end);
  if (!angles.has_value()) return std::nullopt;

  Angle tangent_angle =
      side == TangentSide::kLeft ? angles->left : angles->right;
  Vec offset = end.GetPoint(tangent_angle) - start.GetPoint(tangent_angle);
  Angle angle = offset == Vec{0, 0}
                    ? Angle()
                    : Vec::SignedAngleBetween(direction, offset);
  return {{.turn_angle = angle, .length = offset.Magnitude()}};
}

// Returns the index into `tip_shape.PerimeterCircles()` for the perimeter
// circle with the leftmost turn angle from `starting_circle` in the given
// `direction`. A tie in turn angle returns the index for the closest point.
//
// TODO(b/279156264): Here and for the `GetRightmostTurningIndex` function
// below, a tie may need to return the farthest point instead of closeset to
// help find an intersection between two tangents in
// `AppendTurnExtrusionPoints`.
//
// The turn angle will be limited to [threshold_angle, π/2], so that we never
// return an index to a control point that is to the left and backwards with
// respect to `direction`.
std::optional<int> GetLeftmostTurningIndex(const Circle& starting_circle,
                                           Vec direction,
                                           const BrushTipShape& tip_shape,
                                           Angle threshold_angle) {
  std::optional<int> leftmost_index;
  TangentProperties leftmost_tangent;
  absl::Span<const Circle> circles = tip_shape.PerimeterCircles();
  for (size_t i = 0; i < circles.size(); ++i) {
    std::optional<TangentProperties> tangent = CalculateTangentProperties(
        starting_circle, circles[i], direction, TangentSide::kLeft);
    if (!tangent.has_value() || tangent->turn_angle > kQuarterTurn) continue;

    if (!leftmost_index.has_value() ||
        tangent->turn_angle > leftmost_tangent.turn_angle ||
        (tangent->turn_angle == leftmost_tangent.turn_angle &&
         tangent->length < leftmost_tangent.length)) {
      leftmost_index = i;
      leftmost_tangent = *tangent;
    }
  }

  if (leftmost_index.has_value() &&
      leftmost_tangent.turn_angle >= threshold_angle) {
    return *leftmost_index;
  }
  return std::nullopt;
}

// Returns the index into `tip_shape.PerimeterCircles()` for the perimeter
// circle with the rightmost turn angle from `starting_circle` in the given
// `direction`. A tie in turn angle returns the index for the closest point.
//
// The turn angle will be limited to [-π/2, limit_angle], so that we never
// return an index to a control point that is to the right and backwards with
// respect to `direction`.
std::optional<int> GetRightmostTurningIndex(const Circle& starting_circle,
                                            Vec direction,
                                            const BrushTipShape& tip_shape,
                                            Angle limit_angle) {
  std::optional<int> rightmost_index;
  TangentProperties rightmost_tangent;
  absl::Span<const Circle> circles = tip_shape.PerimeterCircles();
  for (size_t i = 0; i < circles.size(); ++i) {
    std::optional<TangentProperties> tangent = CalculateTangentProperties(
        starting_circle, circles[i], direction, TangentSide::kRight);
    if (!tangent.has_value() || tangent->turn_angle < -kQuarterTurn) continue;

    if (!rightmost_index.has_value() ||
        tangent->turn_angle < rightmost_tangent.turn_angle ||
        (tangent->turn_angle == rightmost_tangent.turn_angle &&
         tangent->length < rightmost_tangent.length)) {
      rightmost_index = i;
      rightmost_tangent = *tangent;
    }
  }

  if (rightmost_index.has_value() &&
      rightmost_tangent.turn_angle <= limit_angle) {
    return *rightmost_index;
  }
  return std::nullopt;
}

Vec GetForwardDirectionWithFallback(const BrushTipShape& first,
                                    const BrushTipShape& second) {
  return first.Center() == second.Center()
             ? Vec{1, 0}
             : (second.Center() - first.Center()).AsUnitVec();
}

}  // namespace

BrushTipShape::TangentCircleIndices BrushTipShape::GetTangentCircleIndices(
    const BrushTipShape& first, const BrushTipShape& second) {
  ABSL_DCHECK(!first.Contains(second));
  ABSL_DCHECK(!second.Contains(first));

  // If we look at the convex hull of all of the perimeter circles, we are
  // trying to find two segments where one end is a point on `first` and the
  // other is a point on `second`.

  if (first.circles_.Size() == 1 && second.circles_.Size() == 1) {
    return TangentCircleIndices{std::make_pair(0, 0), std::make_pair(0, 0)};
  }

  Vec forward = GetForwardDirectionWithFallback(first, second);

  if (first.circles_.Size() == 1) {
    std::optional<int> second_left =
        GetLeftmostTurningIndex(first.circles_[0], forward, second, -kHalfTurn);
    std::optional<int> second_right =
        GetRightmostTurningIndex(first.circles_[0], forward, second, kHalfTurn);
    // It should be impossible for the second indices to not have a value with
    // the relaxed threshold and limit angles. At least one perimeter circle in
    // `second` should not be traveling backwards relative to `forward`.
    ABSL_DCHECK(second_left.has_value());
    ABSL_DCHECK(second_right.has_value());

    return TangentCircleIndices{std::make_pair(0, second_left.value_or(0)),
                                std::make_pair(0, second_right.value_or(0))};
  }

  // We do something akin to a Jarvis march
  // (https://en.wikipedia.org/wiki/Gift_wrapping_algorithm).
  // For each side, we iterate to find the pair of indices for the `first` and
  // `second` control points. We start by trying the rearmost position on
  // `first` w.r.t. the direciton of travel. At each iteration, we try the
  // current index on `first` and find the corresponding index on `second` with
  // the most favorable turn angle, making sure the next perimeter circle on
  // `first` would be in the interior.
  //
  // Note that unlike a true Jarvis march, we do not calculate the turn angle
  // from the previous point on the convex hull and use the travel direction
  // instead. This allows for mitigation against the case when points on
  // `second` are actually behind the rearmost position on `first`.
  TangentCircleIndices indices;
  std::tie(indices.left.first, indices.right.first) =
      GetRearIndices(first, forward);

  // Find the solution for the left side:
  int candidate_index_on_first = indices.left.first;
  bool found_tangents = false;
  for (int i = 0; i < first.circles_.Size(); ++i) {
    int next_candidate_index_on_first =
        first.GetNextPerimeterIndexCw(candidate_index_on_first);
    std::optional<TangentProperties> tangent = CalculateTangentProperties(
        first.circles_[candidate_index_on_first],
        first.circles_[next_candidate_index_on_first], forward,
        TangentSide::kLeft);

    // Circles in the same `BrushTipShape` should never contain one another, so
    // we should always be able to find tangents.
    ABSL_CHECK(tangent.has_value());

    if (auto index_on_second =
            GetLeftmostTurningIndex(first.circles_[candidate_index_on_first],
                                    forward, second, tangent->turn_angle)) {
      found_tangents = true;
      indices.left = {candidate_index_on_first, *index_on_second};
      break;
    }
    candidate_index_on_first = next_candidate_index_on_first;
  }
  if (!found_tangents) {
    // If we failed to converge on a result, we relax the threshold angle to
    // find an index on `second`.
    std::optional<int> index_on_second = GetLeftmostTurningIndex(
        first.circles_[indices.left.first], forward, second, -kHalfTurn);
    // It should be impossible for the second index to not have a value. At
    // least one control point in second should not be traveling backwards from
    // the rearmost control point in `first`.
    ABSL_DCHECK(index_on_second.has_value());
    indices.left.second = index_on_second.value_or(0);
  }

  // Find the solution for the right side:
  candidate_index_on_first = indices.right.first;
  found_tangents = false;
  for (int i = 0; i < first.circles_.Size(); ++i) {
    int next_candidate_index_on_first =
        first.GetNextPerimeterIndexCcw(candidate_index_on_first);
    std::optional<TangentProperties> tangent = CalculateTangentProperties(
        first.circles_[candidate_index_on_first],
        first.circles_[next_candidate_index_on_first], forward,
        TangentSide::kRight);

    // Circles in the same `BrushTipShape` should never contain one another, so
    // we should always be able to find tangents.
    ABSL_CHECK(tangent.has_value());

    if (auto index_on_second =
            GetRightmostTurningIndex(first.circles_[candidate_index_on_first],
                                     forward, second, tangent->turn_angle)) {
      found_tangents = true;
      indices.right = {candidate_index_on_first, *index_on_second};
      break;
    }
    candidate_index_on_first = next_candidate_index_on_first;
  }
  if (!found_tangents) {
    // If we failed to converge on a result, we relax the limit angle to find an
    // index on `second`.
    std::optional<int> index_on_second = GetRightmostTurningIndex(
        first.circles_[indices.right.first], forward, second, kHalfTurn);
    // It should be impossible for the second index to not have a value. At
    // least one control point in second should not be traveling backwards from
    // the rearmost control point in `first`.
    ABSL_DCHECK(index_on_second.has_value());
    indices.right.second = index_on_second.value_or(0);
  }

  return indices;
}

// Returns the minimum bounding rectangle of the `BrushTipShape`.
Rect BrushTipShape::Bounds() const {
  absl::Span<const Circle> circles = PerimeterCircles();
  ABSL_DCHECK(
      !circles.empty());  // There is no way to construct an "empty" shape.

  float d = 2 * circles.front().Radius();
  Rect bounds = Rect::FromCenterAndDimensions(circles.front().Center(), d, d);
  circles.remove_prefix(1);
  for (const Circle& c : circles) {
    d = 2 * c.Radius();
    bounds.Join(Rect::FromCenterAndDimensions(c.Center(), d, d));
  }
  return bounds;
}

namespace {

// Returns the tangent angles between `first` and `second`. `tangent_indices`
// is expected to be the result of `GetTangentCircleIndices(first, second)`.
//
// Valid tangents must exist between the circles indicated by `tangent_indices`;
// this is guaranteed by higher-level logic.
Circle::TangentAngles GetTangentAngles(
    const BrushTipShape& first, const BrushTipShape& second,
    const BrushTipShape::TangentCircleIndices& tangent_indices) {
  std::optional<Circle::TangentAngles> left_tangents =
      first.PerimeterCircles()[tangent_indices.left.first].GetTangentAngles(
          second.PerimeterCircles()[tangent_indices.left.second]);
  std::optional<Circle::TangentAngles> right_tangents =
      first.PerimeterCircles()[tangent_indices.right.first].GetTangentAngles(
          second.PerimeterCircles()[tangent_indices.right.second]);

  ABSL_CHECK(left_tangents.has_value());
  ABSL_CHECK(right_tangents.has_value());

  return {.left = left_tangents->left, .right = right_tangents->right};
}

void AppendLeftCircularTurnPoints(
    absl::Span<const Circle> circles, float max_chord_height,
    AddCircularTangentIntersections add_intersections,
    std::vector<Point>& left_points) {
  for (size_t i = 1; i + 1 < circles.size(); ++i) {
    if (circles[i].Radius() == 0) {
      left_points.push_back(circles[i].Center());
    } else {
      AppendCircularTurnExtrusionPoints(
          circles[i - 1], circles[i], circles[i + 1], max_chord_height,
          add_intersections, &left_points, nullptr);
    }
  }
}

void AppendRightCircularTurnPoints(
    absl::Span<const Circle> circles, float max_chord_height,
    AddCircularTangentIntersections add_intersections,
    std::vector<Point>& right_points) {
  for (size_t i = 1; i + 1 < circles.size(); ++i) {
    if (circles[i].Radius() == 0) {
      right_points.push_back(circles[i].Center());
    } else {
      AppendCircularTurnExtrusionPoints(
          circles[i - 1], circles[i], circles[i + 1], max_chord_height,
          add_intersections, nullptr, &right_points);
    }
  }
}

}  // namespace

void BrushTipShape::AppendTurnExtrusionPoints(
    const BrushTipShape& start, const BrushTipShape& middle,
    const BrushTipShape& end, float max_chord_height,
    ExtrusionPoints& extrusion_points) {
  TangentCircleIndices incoming_tangent_indices =
      GetTangentCircleIndices(start, middle);
  TangentCircleIndices outgoing_tangent_indices =
      GetTangentCircleIndices(middle, end);

  Circle::TangentAngles incoming_angles =
      GetTangentAngles(start, middle, incoming_tangent_indices);
  Circle::TangentAngles outgoing_angles =
      GetTangentAngles(middle, end, outgoing_tangent_indices);

  // We follow similar logic to `AppendCircularTurnExtrusionPoints` to order the
  // angles starting with `incoming_angles.left`. See that function and
  // circular_turn_extrusion_points.svg for the breakdown of cases using a
  // circle.

  if (outgoing_angles.left < incoming_angles.left)
    outgoing_angles.left += kFullTurn;
  if (incoming_angles.right < incoming_angles.left)
    incoming_angles.right += kFullTurn;
  if (outgoing_angles.right < incoming_angles.left)
    outgoing_angles.right += kFullTurn;

  // Left side:
  if (incoming_tangent_indices.left.second ==
          outgoing_tangent_indices.left.first ||
      outgoing_angles.left >=
          std::min(incoming_angles.right, outgoing_angles.right)) {
    // Add arcs connecting perimeter circles.
    absl::InlinedVector<Circle, 5> circles;
    circles.push_back(start.circles_[incoming_tangent_indices.left.first]);
    int middle_index = incoming_tangent_indices.left.second;
    circles.push_back(middle.circles_[middle_index]);
    while (middle_index != outgoing_tangent_indices.left.first) {
      middle_index = middle.GetNextPerimeterIndexCw(middle_index);
      circles.push_back(middle.circles_[middle_index]);
    }
    circles.push_back(end.circles_[outgoing_tangent_indices.left.second]);
    AppendLeftCircularTurnPoints(circles, max_chord_height,
                                 AddCircularTangentIntersections::kYes,
                                 extrusion_points.left);
  } else {
    // Try to add the intersection of incoming and outgoing tangents.
    Segment incoming_tangent{
        start.circles_[incoming_tangent_indices.left.first].GetPoint(
            incoming_angles.left),
        middle.circles_[incoming_tangent_indices.left.second].GetPoint(
            incoming_angles.left)};
    Segment outgoing_tangent{
        middle.circles_[outgoing_tangent_indices.left.first].GetPoint(
            outgoing_angles.left),
        end.circles_[outgoing_tangent_indices.left.second].GetPoint(
            outgoing_angles.left)};
    if (std::optional<Point> intersection =
            SegmentIntersection(incoming_tangent, outgoing_tangent);
        intersection.has_value()) {
      extrusion_points.left.push_back(*intersection);
    } else {
      extrusion_points.left.push_back(incoming_tangent.end);
      extrusion_points.left.push_back(outgoing_tangent.start);
    }
  }

  // Right side:
  if (incoming_tangent_indices.right.second ==
          outgoing_tangent_indices.right.first ||
      outgoing_angles.right > incoming_angles.right ||
      (outgoing_angles.left >= outgoing_angles.right &&
       incoming_angles.right >= outgoing_angles.left)) {
    // Add arcs connecting perimeter circles.
    absl::InlinedVector<Circle, 5> circles;
    circles.push_back(start.circles_[incoming_tangent_indices.right.first]);
    int middle_index = incoming_tangent_indices.right.second;
    circles.push_back(middle.circles_[middle_index]);
    while (middle_index != outgoing_tangent_indices.right.first) {
      middle_index = middle.GetNextPerimeterIndexCcw(middle_index);
      circles.push_back(middle.circles_[middle_index]);
    }
    circles.push_back(end.circles_[outgoing_tangent_indices.right.second]);
    AppendRightCircularTurnPoints(circles, max_chord_height,
                                  AddCircularTangentIntersections::kYes,
                                  extrusion_points.right);
  } else {
    // Try to add the intersection of incoming and outgoing tangents.
    Segment incoming_tangent{
        start.circles_[incoming_tangent_indices.right.first].GetPoint(
            incoming_angles.right),
        middle.circles_[incoming_tangent_indices.right.second].GetPoint(
            incoming_angles.right)};
    Segment outgoing_tangent{
        middle.circles_[outgoing_tangent_indices.right.first].GetPoint(
            outgoing_angles.right),
        end.circles_[outgoing_tangent_indices.right.second].GetPoint(
            outgoing_angles.right)};
    if (std::optional<Point> intersection =
            SegmentIntersection(incoming_tangent, outgoing_tangent);
        intersection.has_value()) {
      extrusion_points.right.push_back(*intersection);
    } else {
      extrusion_points.right.push_back(incoming_tangent.end);
      extrusion_points.right.push_back(outgoing_tangent.start);
    }
  }
}

namespace {

// Returns true if the `extrusion_point` lies to the "right" of the line that
// goes through `shape_center` along `forward`, when viewed from the positive
// z-axis in the direction of `forward`.
bool IsRightPoint(Vec forward, Point shape_center, Point extrusion_point) {
  return Vec::Determinant(forward, extrusion_point - shape_center) < 0;
}

// Moves points that were added to `extrusion_points.left`, but should be in
// `extrusion_points.right`.
//
// This is a helper used by `AppendStartcapExtrusionPoints()` and
// `AppendWholeShapeExtrusionPoints()` below where points are added in
// "clockwise" order to `extrusion_points.left` and must then be sorted into
// left and right.
void MoveNewRightPointsOutOfLeft(Vec forward, Point shape_center,
                                 int64_t new_point_count,
                                 ExtrusionPoints& extrusion_points) {
  auto new_points = absl::MakeSpan(
      &extrusion_points.left[extrusion_points.left.size() - new_point_count],
      new_point_count);
  int64_t new_right_point_count = 0;
  std::optional<int64_t> starting_right_index;

  // When looking at `new_points`, the points will be organized into two or
  // three contiguous sections with the left points being correctly ordered
  // back to front, and the right points being backwards. Assuming equal numbers
  // of left and right points, these could look like one of the following:
  //
  // 1) LLRRRRLL
  //         ^
  // 2) LLLLRRRR
  //           ^
  // 3) RRLLLLRR
  //     ^
  // 4) RRRRLLLL
  //       ^
  //
  // The ^ indicates the "starting" right point, which would be furthest
  // backward with respect to the `forward` vector.

  for (int64_t i = 0; i < new_point_count; ++i) {
    if (IsRightPoint(forward, shape_center, new_points[i])) {
      ++new_right_point_count;
    } else if (!starting_right_index.has_value() && new_right_point_count > 0) {
      starting_right_index = i - 1;
    }
  }
  if (!starting_right_index.has_value() && new_right_point_count > 0) {
    starting_right_index = new_points.size() - 1;
  }

  if (!starting_right_index.has_value()) return;

  // Move the points from `.left` to `.right` putting the right points into
  // the correct order. We also move the left points so that they can be moved
  // back to the start of `.left` without dealing with cases where the left
  // points are split up and not overwriting the wrong data would be subtle.
  for (int64_t i = 0; i < new_point_count; ++i) {
    int64_t j = *starting_right_index - i;
    if (j < 0) j += new_point_count;
    extrusion_points.right.push_back(new_points[j]);
  }
  extrusion_points.left.resize(extrusion_points.left.size() - new_point_count);
  for (int64_t i = 0; i < new_point_count - new_right_point_count; ++i) {
    extrusion_points.left.push_back(extrusion_points.right.back());
    extrusion_points.right.pop_back();
  }
}

}  // namespace

void BrushTipShape::AppendStartcapExtrusionPoints(
    const BrushTipShape& first, const BrushTipShape& second,
    float max_chord_height, ExtrusionPoints& extrusion_points) {
  TangentCircleIndices tangent_indices = GetTangentCircleIndices(first, second);

  absl::InlinedVector<Circle, 5> circles;
  circles.push_back(second.circles_[tangent_indices.right.second]);
  int first_index = tangent_indices.right.first;
  circles.push_back(first.circles_[first_index]);
  while (first_index != tangent_indices.left.first) {
    first_index = first.GetNextPerimeterIndexCw(first_index);
    circles.push_back(first.circles_[first_index]);
  }
  circles.push_back(second.circles_[tangent_indices.left.second]);

  // First append all of the points into `extrusion_points.left`, and then split
  // according to the travel direction:

  int64_t size_before_append = extrusion_points.left.size();
  AppendLeftCircularTurnPoints(circles, max_chord_height,
                               AddCircularTangentIntersections::kNo,
                               extrusion_points.left);

  Vec forward = GetForwardDirectionWithFallback(first, second);
  MoveNewRightPointsOutOfLeft(forward, first.Center(),
                              extrusion_points.left.size() - size_before_append,
                              extrusion_points);
}

void BrushTipShape::AppendEndcapExtrusionPoints(
    const BrushTipShape& second_to_last, const BrushTipShape& last,
    float max_chord_height, ExtrusionPoints& extrusion_points) {
  TangentCircleIndices tangent_indices =
      GetTangentCircleIndices(second_to_last, last);

  absl::InlinedVector<Circle, 5> circles;
  circles.push_back(second_to_last.circles_[tangent_indices.left.first]);
  int last_index = tangent_indices.left.second;
  circles.push_back(last.circles_[last_index]);
  while (last_index != tangent_indices.right.second) {
    last_index = last.GetNextPerimeterIndexCw(last_index);
    circles.push_back(last.circles_[last_index]);
  }
  circles.push_back(second_to_last.circles_[tangent_indices.right.first]);

  // First append all of the points into `extrusion_points.left`, and then split
  // according to the travel direction:

  int64_t size_before_append = extrusion_points.left.size();
  AppendLeftCircularTurnPoints(circles, max_chord_height,
                               AddCircularTangentIntersections::kNo,
                               extrusion_points.left);

  Vec forward = GetForwardDirectionWithFallback(second_to_last, last);

  int64_t boundary_index =
      static_cast<int64_t>(extrusion_points.left.size()) - 1;
  while (boundary_index >= size_before_append &&
         IsRightPoint(forward, last.Center(),
                      extrusion_points.left[boundary_index])) {
    extrusion_points.right.push_back(extrusion_points.left[boundary_index]);
    --boundary_index;
  }
  extrusion_points.left.resize(boundary_index + 1);
}

void BrushTipShape::AppendWholeShapeExtrusionPoints(
    const BrushTipShape& shape, float max_chord_height, Vec forward_direction,
    ExtrusionPoints& extrusion_points) {
  if (shape.circles_.Size() == 1) {
    // When the tip shape is circular, we make two arcs where the first and last
    // pairs of the left and right points would be separated by a chord with
    // `max_chord_height`, but limited to not exceed half of the circle.

    const Circle& circle = shape.circles_[0];
    Angle max_chord_angle =
        std::min(circle.GetArcAngleForChordHeight(max_chord_height), kHalfTurn);
    Angle rear_angle = (-forward_direction).Direction();
    Angle left_arc_start = rear_angle - max_chord_angle / 2;
    Angle right_arc_start = rear_angle + max_chord_angle / 2;
    circle.AppendArcToPolyline(left_arc_start, max_chord_angle - kHalfTurn,
                               max_chord_height, extrusion_points.left);
    circle.AppendArcToPolyline(right_arc_start, kHalfTurn - max_chord_angle,
                               max_chord_height, extrusion_points.right);
    return;
  }

  // When the shape is not a single circle, we gather all of the perimeter
  // circles and traverse them to add turn points around the entire tip. Start
  // at the "last" perimeter circle in clockwise order and go around clockwise
  // until we end with the "first" circle again. We don't try to order the
  // circles themselves according to `forward_direction`, because the boundary
  // between left and right points may lie within the turn points of one of the
  // circles. So as with `AppendStartcapExtrusionPoints()`, we put all of the
  // points into `extrusion_points.left` and then split them after the fact.
  absl::InlinedVector<Circle, 6> circles;
  int i = shape.GetNextPerimeterIndexCcw(0);
  for (int j = 0; j < shape.circles_.Size() + 2; ++j) {
    circles.push_back(shape.circles_[i]);
    i = shape.GetNextPerimeterIndexCw(i);
  }

  int64_t size_before_append = extrusion_points.left.size();
  AppendLeftCircularTurnPoints(circles, max_chord_height,
                               AddCircularTangentIntersections::kNo,
                               extrusion_points.left);
  MoveNewRightPointsOutOfLeft(forward_direction, shape.Center(),
                              extrusion_points.left.size() - size_before_append,
                              extrusion_points);
}

namespace {

// Returns true if any perimeter circle of `shape` is to the "right" of the
// corner described by the given circle and tangent angles.
//
// `ccw_incoming_angle` and `ccw_outgoing_angle` are expected to both be
// normalized to the same range of angles.
bool ShapeIsOutsideOfCorner(const BrushTipShape& shape,
                            const Circle& corner_circle,
                            Angle ccw_incoming_angle,
                            Angle ccw_outgoing_angle) {
  // The following diagram describes the geometry of the corner:
  //
  //        CCW Outgoing Edge                           Corner
  //           <--------                              Exterior
  //     ---------------------~ ~ - ,
  //                , '       ^        ' ,
  //              ,           |           ,
  //             ,            B            ,
  //            ,                           ,
  //            ,           Corner       A->,
  //            ,           Circle          |
  //         Corner                        ,|   ^
  //         Interior                     , |   |  CCW Incoming Edge
  //                ,                  , '  |   |
  //                  ' - , _ _ _ ,  '      |
  //
  // In the diagram above:
  //   * A is `corner_circle.GetPoint(ccw_incoming_angle)`.
  //   * B is `corner_circle.GetPoint(ccw_outgoing_angle)`.
  //   * A and B split the `corner_circle` into two arcs:
  //       * The CCW arc from A to B is part of the exterior corner boundary.
  //       * The CCW arc from B to A is part of the interior of the corner.
  //
  // The "right" tangent from the `corner_circle` to one of the perimeter
  // circles on `shape` can meet the `corner_circle` along one of these two
  // arcs. See also the diagram in ink/geometry/internal/get_tangent_angles.svg.
  //
  // If the tangent meets along the exterior arc, then at least some of `shape`
  // is guaranteed to lie outside of the corner. Otherwise, if all found
  // tangents touch the `corner_circle` along the interior arc, then the shape
  // is not necessarily in the exterior. Another call to this function with a
  // different `corner_circle` will be able to confirm whether or not the shape
  // is fully in the interior.

  // To check the order of tangent angles about the circle, we will renormalize
  // the angles so that the incoming angle is the smallest.
  if (ccw_outgoing_angle < ccw_incoming_angle) ccw_outgoing_angle += kFullTurn;

  for (const Circle& test_circle : shape.PerimeterCircles()) {
    // If either circle contains the other, there is no well defined tangent
    // angle between them.
    if (corner_circle.Contains(test_circle)) continue;
    if (test_circle.Contains(corner_circle)) {
      // Since we already checked that `corner_circle` does not contain
      // `test_circle`, we know that `test_circle` is stricly larger.
      return true;
    }

    Angle angle = corner_circle.GuaranteedRightTangentAngle(test_circle);
    if (angle < ccw_incoming_angle) angle += kFullTurn;
    if (angle < ccw_outgoing_angle) return true;
  }

  return false;
}

}  // namespace

bool BrushTipShape::Contains(const BrushTipShape& other_shape) const {
  // First do a rough bounds check to see if we can exit early without calling
  // `Vec::Magnitude()` or any trig functions.
  if (!this->Bounds().Contains(other_shape.Bounds())) return false;

  if (circles_.Size() == 1) {
    const Circle& circle = circles_[0];
    for (const Circle& c : other_shape.circles_.Values()) {
      if (!circle.Contains(c)) return false;
    }
    return true;
  }

  // With more than one perimeter circle, we iterate over each rounded corner in
  // CCW order and check if the `other_shape` is exterior to it. This is only
  // guaranteed to work because the `BrushTipShape` is always convex.
  const Circle& last_ccw_circle = circles_[GetNextPerimeterIndexCw(0)];
  Angle incoming_ccw_angle =
      last_ccw_circle.GuaranteedRightTangentAngle(circles_[0]);
  for (int i = 0; i < circles_.Size(); ++i) {
    const Circle& corner_circle = circles_[i];
    Angle outgoing_ccw_angle = corner_circle.GuaranteedRightTangentAngle(
        circles_[GetNextPerimeterIndexCcw(i)]);
    if (ShapeIsOutsideOfCorner(other_shape, circles_[i], incoming_ccw_angle,
                               outgoing_ccw_angle)) {
      return false;
    }

    incoming_ccw_angle = outgoing_ccw_angle;
  }
  return true;
}

int BrushTipShape::GetNextPerimeterIndexCw(int index) const {
  ABSL_DCHECK_LT(index, circles_.Size());
  return index == 0 ? circles_.Size() - 1 : index - 1;
}

int BrushTipShape::GetNextPerimeterIndexCcw(int index) const {
  ABSL_DCHECK_LT(index, circles_.Size());
  return index + 1 >= circles_.Size() ? 0 : index + 1;
}

}  // namespace ink::strokes_internal
