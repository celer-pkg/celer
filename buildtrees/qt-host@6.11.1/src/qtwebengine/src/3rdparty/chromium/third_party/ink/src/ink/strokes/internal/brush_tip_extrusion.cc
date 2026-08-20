#include "ink/strokes/internal/brush_tip_extrusion.h"

#include <algorithm>
#include <cstdlib>

#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/distance.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/rect.h"
#include "ink/strokes/internal/brush_tip_shape.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/rounded_polygon.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::Circle;

// Constructs a `RoundedPolygon` representing the shape that results from
// connecting `first` and `second` by the tangents of the circles indicated by
// `indices`. The returned shape starts at `indices.left.first`, proceeds to
// `indices.right.first`, then jumps to `indices.right.second` and proceeds to
// `indices.left.second`, always moving counter-clockwise per
// `GetNextPerimeterIndexCcw`. `indices` is expected to be the result of calling
// `GetTangentCircleIndices(first, second)`. `first` must not contain `second`,
// and vice versa.
//
// `offset` specifies how much the returned `RoundedPolygon` should be offset
// from the actual joined shape. This value must be >= 0.
//
// Note that not all of the component circles of `first` and `second` are
// guaranteed to be contained in the returned shape; e.g. you could have two
// rectangular `BrushTipShape`s that form a cross, which would leave two
// circles outside the `RoundedPolygon`.
static RoundedPolygon ConstructJoinedShape(
    const BrushTipShape& first, const BrushTipShape& second,
    const BrushTipShape::TangentCircleIndices& indices, float offset) {
  ABSL_DCHECK(!first.Contains(second));
  ABSL_DCHECK(!second.Contains(first));
  ABSL_CHECK_GE(offset, 0);

  // Each `BrushTipShape` has at most four circles, so we need at most eight
  // for the `RoundedPolygon`.
  absl::InlinedVector<Circle, 8> circles;

  auto add_circles = [&circles, offset](const BrushTipShape& shape,
                                        int first_index, int last_index) {
    auto add_one_circle_with_offset = [&circles, offset](const Circle& circle) {
      circles.push_back({circle.Center(), circle.Radius() + offset});
    };
    if (first_index == last_index) {
      add_one_circle_with_offset(shape.PerimeterCircles()[first_index]);
    } else {
      for (int index = first_index; index != last_index;
           index = shape.GetNextPerimeterIndexCcw(index)) {
        add_one_circle_with_offset(shape.PerimeterCircles()[index]);
      }
      add_one_circle_with_offset(shape.PerimeterCircles()[last_index]);
    }
  };

  add_circles(first, indices.left.first, indices.right.first);
  add_circles(second, indices.right.second, indices.left.second);

  return RoundedPolygon(circles);
}

BrushTipExtrusion::TangentQuality EvaluateTangentQualityInternal(
    const BrushTipShape& first, const BrushTipShape& second) {
  if (first.Contains(second)) {
    return BrushTipExtrusion::TangentQuality::kNoTangentsFirstContainsSecond;
  }

  if ((second.Contains(first))) {
    return BrushTipExtrusion::TangentQuality::kNoTangentsSecondContainsFirst;
  }

  // If we have two circles that don't contain each other, then we can always
  // construct good tangents.
  // NOMUTANTS -- this is just a short-circuit for performance.
  if (first.PerimeterCircles().size() == 1 &&
      second.PerimeterCircles().size() == 1) {
    return BrushTipExtrusion::TangentQuality::kGoodTangents;
  }

  // Fetch the indices of the circles that will be connect the two shapes.
  BrushTipShape::TangentCircleIndices indices =
      BrushTipShape::GetTangentCircleIndices(first, second);

  // If the first circle is immediately *clockwise* to the last circle for
  // each shape, then all circles contribute to the boundary of the joined
  // shape and there are no unused circles.
  // NOMUTANTS -- This is just a short-circuit for performance.
  if (first.GetNextPerimeterIndexCw(indices.left.first) ==
          indices.right.first &&
      second.GetNextPerimeterIndexCw(indices.right.second) ==
          indices.left.second) {
    return BrushTipExtrusion::TangentQuality::kGoodTangents;
  }

  // In order to avoid false-negatives from `RoundedPolygon::ContainsCircle` due
  // to floating-point precision issues, we enlarge the joined shape by a small
  // amount.
  Rect first_bounds = first.Bounds();
  Rect second_bounds = second.Bounds();
  float max_absolute_coordinate = std::max(
      {std::abs(first_bounds.XMin()), std::abs(first_bounds.XMax()),
       std::abs(first_bounds.YMin()), std::abs(first_bounds.YMax()),
       std::abs(second_bounds.XMin()), std::abs(second_bounds.XMax()),
       std::abs(second_bounds.YMin()), std::abs(second_bounds.YMax())});
  float offset = 1e-6 * max_absolute_coordinate;

  // Construct the joined shape, with the offset.
  RoundedPolygon joined_shape =
      ConstructJoinedShape(first, second, indices, offset);

  // Finally, check whether the unused circles are contained inside the joined
  // shape.
  for (int index = first.GetNextPerimeterIndexCcw(indices.right.first);
       index != indices.left.first;
       index = first.GetNextPerimeterIndexCcw(index)) {
    if (!joined_shape.ContainsCircle(first.PerimeterCircles()[index]))
      return BrushTipExtrusion::TangentQuality::
          kBadTangentsJoinedShapeDoesNotCoverInputShapes;
  }
  for (int index = second.GetNextPerimeterIndexCcw(indices.left.second);
       index != indices.right.second;
       index = second.GetNextPerimeterIndexCcw(index)) {
    if (!joined_shape.ContainsCircle(second.PerimeterCircles()[index]))
      return BrushTipExtrusion::TangentQuality::
          kBadTangentsJoinedShapeDoesNotCoverInputShapes;
  }

  return BrushTipExtrusion::TangentQuality::kGoodTangents;
}

bool StatesHaveEqualShapeProperties(const BrushTipState& first,
                                    const BrushTipState& second) {
  return first.width == second.width && first.height == second.height &&
         first.percent_radius == second.percent_radius &&
         first.rotation == second.rotation && first.slant == second.slant &&
         first.pinch == second.pinch;
}

}  // namespace

BrushTipExtrusion::TangentQuality BrushTipExtrusion::EvaluateTangentQuality(
    const BrushTipExtrusion& first, const BrushTipExtrusion& second,
    float travel_threshold) {
  if ((Distance(first.GetState().position, second.GetState().position)) >
          travel_threshold &&
      StatesHaveEqualShapeProperties(first.GetState(), second.GetState())) {
    return TangentQuality::kGoodTangents;
  }

  return EvaluateTangentQualityInternal(first.GetShape(), second.GetShape());
}

}  // namespace ink::strokes_internal
