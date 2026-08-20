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

#include "ink/strokes/internal/circular_extrusion_helpers.h"

#include <algorithm>
#include <optional>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::Circle;
using ::ink::geometry_internal::SegmentIntersection;

// Returns the number of the case from circular_turn_extrusion_points.svg
// corresponding to the order of the three other tangent angles relative to
// `in.left`.
//
// Assumes that the angles have been normalized such that `in.left` is the
// smallest of the four values.
int GetTangentAngleOrderCase(const Circle::TangentAngles& in,
                             const Circle::TangentAngles& out) {
  Angle second_angle = std::min({in.right, out.left, out.right});

  if (second_angle == out.left) {
    if (out.right <= in.right) return 3;  // L-in, L-out, R-out, R-in
    return 2;                             // L-in, L-out, R-in, R-out
  }  //
  if (second_angle == out.right) {       //
    if (out.left <= in.right) return 5;  // L-in, R-out, L-out, R-in
    return 1;                            // L-in, R-out, R-in, L-out
  }  //
  if (out.left <= out.right) return 6;  // L-in, R-in, L-out, R-out
  return 4;                             // L-in, R-in, R-out, L-out
}

}  // namespace

void AppendCircularTurnExtrusionPoints(
    const Circle& start, const Circle& middle, const Circle& end,
    float max_chord_height, AddCircularTangentIntersections add_intersections,
    std::vector<Point>* left_points, std::vector<Point>* right_points) {
  // There are six cases, which can be uniquely identified by the relative order
  // of the angles between the center point and the points at which the tangents
  // meet the center circle. See circular_turn_extrusion_points.svg for the
  // cases.
  std::optional<Circle::TangentAngles> incoming_angles =
      start.GetTangentAngles(middle);
  std::optional<Circle::TangentAngles> outgoing_angles =
      middle.GetTangentAngles(end);

  // This is guaranteed by the fact that the consecutive circles do not contain
  // one another.
  ABSL_CHECK(incoming_angles.has_value());
  ABSL_CHECK(outgoing_angles.has_value());

  // Snap nearly-collinear angles together, to prevent adding an unnecessary
  // loop. The chosen tolerance is just a bit over four times machine precision
  // for ±π radians, which is ~5.73e-5 degrees.
  constexpr Angle kCollinearTol = Angle::Radians(1e-5);
  if (Angle left_delta = Abs(incoming_angles->left - outgoing_angles->left);
      left_delta < kCollinearTol || left_delta + kCollinearTol > kFullTurn) {
    incoming_angles->left = outgoing_angles->left;
  }
  if (Angle right_delta = Abs(incoming_angles->right - outgoing_angles->right);
      right_delta < kCollinearTol || right_delta + kCollinearTol > kFullTurn) {
    incoming_angles->right = outgoing_angles->right;
  }

  // W.l.o.g., we designate incoming_angles->left as the start of the order (we
  // can do this because the order is cyclic). So, we normalize the rest of
  // the angles to lie within the range
  // [incoming_angles->left, incoming_angles->left + 2π radians).
  if (outgoing_angles->left < incoming_angles->left)
    outgoing_angles->left += kFullTurn;
  if (incoming_angles->right < incoming_angles->left)
    incoming_angles->right += kFullTurn;
  if (outgoing_angles->right < incoming_angles->left)
    outgoing_angles->right += kFullTurn;

  auto add_intersection = [&start, &middle, &end](Angle incoming_angle,
                                                  Angle outgoing_angle,
                                                  std::vector<Point>& result) {
    Segment incoming_tangent{start.GetPoint(incoming_angle),
                             middle.GetPoint(incoming_angle)};
    Segment outgoing_tangent{middle.GetPoint(outgoing_angle),
                             end.GetPoint(outgoing_angle)};
    if (std::optional<Point> intersection =
            SegmentIntersection(incoming_tangent, outgoing_tangent);
        intersection.has_value()) {
      result.push_back(*intersection);
    } else {
      // If we don't find an intersection (which can happen if the radii are
      // large), we fall back to just creating a segment connecting the
      // tangents.
      // TODO(b/184291546): This can result in a "zig-zag" in the outline.
      result.push_back(incoming_tangent.end);
      result.push_back(outgoing_tangent.start);
    }
  };

  auto add_left_arc = [&]() {
    if (left_points == nullptr) return;

    // "Normalize" the left arc angle to be in [-2π, 0) radians so that the arc
    // is traversed on the "left" side of the circle given the travel
    // directions.  Note that we have ordered the angles above such that
    // `incoming_angles->left` is the smallest value.
    Angle arc_angle = outgoing_angles->left - incoming_angles->left - kFullTurn;
    middle.AppendArcToPolyline(incoming_angles->left, arc_angle,
                               max_chord_height, *left_points);
  };
  auto add_right_arc = [&]() {
    if (right_points == nullptr) return;

    // Normalize the right arc angle to be in [0, 2π) radians so that the arc is
    // traverse on the "right" side of the circle given the travel directions.
    Angle arc_angle =
        (outgoing_angles->right - incoming_angles->right).Normalized();
    middle.AppendArcToPolyline(incoming_angles->right, arc_angle,
                               max_chord_height, *right_points);
  };

  if (add_intersections == AddCircularTangentIntersections::kNo) {
    add_left_arc();
    add_right_arc();
    return;
  }

  auto add_left_intersection = [&]() {
    if (left_points == nullptr) return;
    add_intersection(incoming_angles->left, outgoing_angles->left,
                     *left_points);
  };
  auto add_right_intersection = [&]() {
    if (right_points == nullptr) return;
    add_intersection(incoming_angles->right, outgoing_angles->right,
                     *right_points);
  };

  switch (GetTangentAngleOrderCase(*incoming_angles, *outgoing_angles)) {
    case 1:
      add_left_arc();
      add_right_intersection();
      break;
    case 2:
      add_left_intersection();
      add_right_arc();
      break;
    case 3:
      add_left_intersection();
      add_right_intersection();
      break;
    case 4:
      add_left_arc();
      add_right_arc();
      break;
    default: {
      // Case 5 or 6.
      //
      // Edge cases can come up if the circles mostly overlap and their radii
      // are rapidly increasing (in case 6) or decreasing (in case 5). The case
      // 5/6 angle order can occur even though the three centers travel in
      // roughly the same direction. We check if the magnitude of the turn angle
      // is greater than or equal to the chosen value of 120 degrees before
      // adding arcs on both sides. Otherwise, the behavior is like case 1 or 2
      // depending on the sign of the turn angle.
      Angle turn_angle = Vec::SignedAngleBetween(
          middle.Center() - start.Center(), end.Center() - middle.Center());
      if (Abs(turn_angle) >= kFullTurn / 3) {
        add_left_arc();
        add_right_arc();
      } else if (turn_angle < Angle()) {
        add_left_arc();
        add_right_intersection();
      } else {
        add_left_intersection();
        add_right_arc();
      }
      break;
    }
  }
}

}  // namespace ink::strokes_internal
