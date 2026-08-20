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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_GEOMETRY_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_GEOMETRY_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "absl/base/attributes.h"
#include "absl/base/nullability.h"
#include "absl/container/flat_hash_map.h"
#include "absl/types/span.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/strokes/internal/brush_tip_extruder/derivative_calculator.h"
#include "ink/strokes/internal/brush_tip_extruder/directed_partial_outline.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/legacy_vertex.h"

namespace ink {
namespace brush_tip_extruder_internal {

enum class TextureCoordType { kTiling, kWinding };

// Metadata about `Geometry` at the last extrusion break, i.e. the point where
// the current stroke outline began.
struct GeometryLastExtrusionBreakMetadata {
  // Info about the break point for `Geometry::left_side_` and `::right_side_`.
  struct SideInfo {
    // The size of `Side::indices`.
    uint32_t index_count = 0;
    // The size of `Side::intersection_discontinuities`.
    uint32_t intersection_discontinuity_count = 0;
  };

  uint32_t break_count = 0;

  // The numbers of vertices and triangles in `Geometry::mesh_`.
  uint32_t vertex_count = 0;
  uint32_t triangle_count = 0;
  SideInfo left_side_info;
  SideInfo right_side_info;
};

// Saved state of the Geometry class below. Used when calling
// `Geometry::SetSavePoint()` and `Geometry::RevertToSavePoint()`.
struct GeometrySavePointState {
  struct SideState {
    // The sizes of `Side::indices` and `Side::intersection_discontinuities`.
    // respectively. In most cases, this is sufficient to restore the save
    // state, as they usually only grow without existing contents being
    // modified. However, `ClearSinceLastBreakPoint` deletes part of
    // the stroke, which needs to be restored; see `saved_indices` and
    // `saved_intersection_discontinuities` below.
    uint32_t n_indices = 0;
    uint32_t n_intersection_discontinuities = 0;

    // Saved values of `Side::indices` and `Side::intersection_discontinuities`
    // at indices greater than `n_indices` and `n_intersection_discontinuities`.
    // These will only be populated if geometry has been deleted, e.g. via
    // `ClearSinceLastExtrusionBreak`.
    std::vector<IndexType> saved_indices;
    std::vector<Side::IndexOffsetRange> saved_intersection_discontinuities;

    // The rest of the members are copies of those in `Side`.
    Side::MeshPartitionStart partition_start;
    uint32_t first_simplifiable_index_offset = 0;
    std::vector<ExtrudedVertex> vertex_buffer;
    uint32_t next_buffered_vertex_offset = 0;
    std::optional<Side::SelfIntersection> intersection;
    std::vector<Point> last_simplified_vertex_positions;
  };

  // Indicates whether the save point is currently active.
  bool is_active = false;

  // Indicates whether the save point contains the complete geometry after the
  // last extrusion break that has been cleared by
  // `ClearSinceLastExtrusionBreak`.
  bool contains_all_geometry_since_last_extrusion_break = false;

  uint32_t n_mesh_vertices = 0;
  uint32_t n_mesh_triangles = 0;

  // Saved values of `Geometry::vertex_side_ids_` and `Geometry::side_offsets_`.
  // Existing values of these `Geoemtry` members are only modified when geometry
  // between an extrusion break and save point is cleared, so we can save them
  // in contiguous chunks.
  std::vector<SideId> saved_vertex_side_ids;
  std::vector<uint32_t> saved_side_offsets;

  // A record of any vertices, triangle indices, and opposite side offsets that
  // existed prior to setting the save point and have been modified since.
  //
  // We expect a small number will need to be saved, but without expected order,
  // so we use maps for simpler bookkeeping during extrusion instead of trying
  // to store a sub-vectors for faster revert.
  absl::flat_hash_map<IndexType, ExtrudedVertex> saved_vertices;
  absl::flat_hash_map<uint32_t, std::array<IndexType, 3>>
      saved_triangle_indices;
  absl::flat_hash_map<IndexType, uint32_t> saved_opposite_side_offsets;

  GeometryLastExtrusionBreakMetadata saved_last_extrusion_break;

  SideState left_side_state;
  SideState right_side_state;
};

// Incrementally builds the triangle mesh data representing the line out of the
// passed-in vertices.
//
// The vertices and triangle indices are written into the currently set
// `MutableMeshView`.
class Geometry {
 public:
  enum class IntersectionHandling { kEnabled, kDisabled };

  // A size-type for outline indices.
  struct IndexCounts {
    size_t left = 0;
    size_t right = 0;
  };

  Geometry();
  explicit Geometry(const MutableMeshView& mesh);

  // Marks the current state so that subsequent extrusions via
  // `AppendLeftVertex()` / `AppendRightVertex()` and `ProcessNewVertices()` can
  // be undone.
  //
  // Does not affect line modifier, intersection handling behavior, mesh
  // transform or shader metadata.
  void SetSavePoint();

  // Reverts the geometry state to the last save and clears the save point.
  //
  // Does nothing if `SetSavePoint()` was either never called, or was last
  // called before the last call to `RevertToSavePoint()`.
  void RevertToSavePoint();

  void SetTextureCoordType(TextureCoordType type);

  // Sets whether or not to handle self-intersections. Enabled by default.
  void SetIntersectionHandling(IntersectionHandling intersection_handling);

  // The following return the offsets into `Side::indices` for the first new or
  // modified index in the last outline belonging to the left and right sides.
  uint32_t FirstMutatedLeftIndexOffsetInCurrentPartition() const;
  uint32_t FirstMutatedRightIndexOffsetInCurrentPartition() const;

  const MutableMeshView& GetMeshView() const;

  // Resets the values of member variables tracking mutations, including the
  // mutation tracking inside the mesh view. See also
  // `MutableMeshView::ResetMutationTracking()`.
  void ResetMutationTracking();

  // Returns the bounding region of the mesh that has visually changed since
  // either construction or the most recent call to either `Reset()` or
  // `ResetMutationTracking()`.
  //
  // A mesh triangle is considered visually changed and needs to be fully
  // redrawn if either of the following is true:
  //   * The triangle contains one or more vertices that are new or have been
  //     modified.
  //   * The indices at a particular triangle index are either new or have been
  //     modified.
  //
  // The returned envelope includes both the current positions of the mesh and
  // positions that have been removed.
  Envelope CalculateVisuallyUpdatedRegion() const;

  // Returns the number of triangles in the mesh that are guaranteed to not
  // change after future extrusions.
  //
  // This value is non-zero only when the line was started with
  // `IntersectionHandling::kDisabled`.
  uint32_t NStableTriangles() const;

  // Resets the geometry to begin a new stroke.
  void Reset(const MutableMeshView& mesh);

  // Appends a new vertex with the given properties to the appropriate side.
  // Left and right are as seen on the screen with respect to the direction of
  // extrusion.
  //
  // The vertices do not become part of the mesh until `ProcessNewVertices()` is
  // called.
  //
  // TODO: b/271837965 - Add parameters for winding texture coordinates.
  void AppendLeftVertex(Point position, float opacity_shift = 0,
                        const std::array<float, 3>& hsl_shift = {},
                        Point surface_uv = {0, 0}, float animation_offset = 0);
  void AppendRightVertex(Point position, float opacity_shift = 0,
                         const std::array<float, 3>& hsl_shift = {},
                         Point surface_uv = {0, 0}, float animation_offset = 0);

  // The following functions append the legacy vertex types to the appropriate
  // sides:
  void AppendLeftVertex(const strokes_internal::LegacyVertex& vertex);
  void AppendRightVertex(const strokes_internal::LegacyVertex& vertex);

  const Side& LeftSide() const;
  const Side& RightSide() const;

  // Appends and triangulates all buffered vertices not already in the mesh.
  //
  //   * `simplification_threshold` is used to first remove vertices that do not
  //      meaningfully contribute to the curvature of the line.
  //   * `last_tip_state` is the most recent `BrushTipState` used to create the
  //     vertices to be triangulated.
  //
  // This function only performs an action if there are left and right vertices.
  // In other words, if vertices have only been added to one side, calling this
  // function will not result in simplification of that side.
  void ProcessNewVertices(
      float simplification_threshold,
      const strokes_internal::BrushTipState& last_tip_state);

  // Starts a new logical partition of the stroke mesh that will be visibly
  // disconnected from existing geometry.
  //
  // For each `Side`, this also sets up new mesh partition tracking, clears any
  // buffered vertices, and resets any ongoing self-intersections.
  void AddExtrusionBreak();

  // Clears geometry added since the last added extrusion break. See
  // also `AddExtrusionBreak`.
  void ClearSinceLastExtrusionBreak();

  // Number of extrusion breaks.
  uint32_t ExtrusionBreakCount() const;

  // Counts of left and right indices at the last extrusion break.
  IndexCounts IndexCountsAtLastExtrusionBreak() const;

  // Updates the derivative attribute properties inside the current mesh.
  //
  // Derivative values will be updated for vertices based on the current value
  // of `FirstVisuallyMutatedTriangle()`. A vertex will be updated if it is part
  // of the visually affected triangulation or if it coincides with a different
  // vertex that is. For efficiency, this function should be called only once in
  // between resetting mutation tracking, because calling it will generally
  // decrease the value returned by `FirstVisuallyMutatedTriangle()`.
  void UpdateMeshDerivatives();

  // TODO: b/294561921 - Add an API to start or find a "connected" partition.
  // This would be used to build strokes that exceeds the 16-bit index limit
  // into multiple `MutableMesh` instead of relying on renderers to do the
  // partitioning.

  // For testing: creates a sub-mesh consisting of triangles appended after the
  // save point. This may not reflect the boundary of the save point with
  // complete accuracy if intersection handling is ongoing.
  void DebugMakeMeshAfterSavePoint(MutableMeshView mesh) const;

 private:
  enum class TriangleWinding {
    kCounterClockwise,
    kClockwise,
    kDegenerate,
  };

  // Returns the vertex associated with the last index in `side.indices`.
  ExtrudedVertex LastVertex(const Side& side) const;
  // Returns the position of the vertex associated with the last index in
  // `side.indices`.
  Point LastPosition(const Side& side) const;

  // Returns the winding of a triangle that would be made using the
  // positions of `left_side_.indices.back()`, `right_side_.indices.back()` and
  // the `proposed_position`, in this order.
  TriangleWinding ProposedTriangleWinding(Point proposed_position) const;

  // Returns the winding of a triangle that would be made using last proposed
  // intersection vertex of `intersecting_side`, the last vertex of the opposing
  // side, and the new proposed position.
  TriangleWinding ProposedIntersectionTriangleWinding(
      const Side& intersecting_side, Point proposed_position) const;

  // Returns the `Side` opposite to `side`. E.g. returns `left_side_` when
  // given `right_side_`.
  Side& OpposingSide(Side& side);
  const Side& OpposingSide(const Side& side) const;

  // Appends `vertex` to `side`, also calling `AppendVertexToMesh()` if this is
  // the first vertex of the current partition on this side.
  void AppendVertexToSide(Side& side, const ExtrudedVertex& vertex);

  // Appends `vertex` to `mesh_`, `vertex_side_ids_` and `side->indices`.
  void AppendVertexToMesh(Side& side, const ExtrudedVertex& vertex);

  // Attempts to append `vertex` and to add a new triangle using its index.
  //
  // Appends both the vertex and triangle if the triangle would have
  // counter-clockwise winding order. If the triangle would be degenerate,
  // appends the vertex only if its position is distinct from the last position
  // of `side`.
  void TryAppendVertexAndTriangleToMesh(Side& side,
                                        const ExtrudedVertex& vertex);

  // Removes vertices from `side->vertex_buffer` that do not contribute to
  // curvature.
  void SimplifyBufferedVertices(Side& side,
                                float initial_outline_reposition_budget,
                                float simplification_threshold,
                                float simplification_travel_limit);

  // Removes vertices from the left and right sides of the line that do not
  // contribute to the line's curvature.
  void SimplifyBufferedVertices(float initial_outline_reposition_budget,
                                float simplification_threshold,
                                float simplification_travel_limit);

  // Returns true if the first and second `indices` belong to the left and right
  // sides respectively.
  bool TriangleIndicesAreLeftRightConforming(
      absl::Span<const IndexType> indices) const;

  // Returns true if all three `indices` belong to the given `side`.
  bool TriangleIndicesAllBelongTo(absl::Span<const IndexType> indices,
                                  SideId side) const;

  // Searches backwards through through a "sufficiently convex" part of mesh to
  // find a triangle containing `segment.to`, and returns its index if one is
  // found.
  //
  // This function tests triangles in reverse from the end of the mesh until
  // `search_along_side.partition_start.first_triangle`. It will exit early if
  // it iterates past `max_early_exit_triangle` and finds that `segment` is not
  // contained in one of the tested triangles.
  std::optional<uint32_t> FindLastTriangleContainingSegmentEnd(
      const Side& search_along_side, const Segment& segment,
      uint32_t max_early_exit_triangle) const;

  // Repositions `outline` vertices to the first point of intersection between a
  // segment of the `outline` and `segment`. Here, "first" means as `outline` is
  // traversed from `outline_starting_side`.
  //
  // Returns true if an intersection is found, and false otherwise.
  //
  // If `outline_starting_side` is undergoing self-intersection, then its value
  // of `Side::outline_reposition_budget` will be used to end intersection
  // search. Otherwise, `default_outline_reposition_budget` will be used.
  //
  // A non-nullopt `containing_triangle` ensures all positions in the outline
  // that are moved are contained in the given triangle.
  ABSL_MUST_USE_RESULT bool MoveStartingVerticesToIntersection(
      Side& outline_starting_side, const DirectedPartialOutline& outline,
      const Segment& segment, float default_outline_reposition_budget,
      std::optional<Triangle> containing_triangle = std::nullopt);

  // Moves vertices in `outline` with indices in range [start, end) to the
  // `target` vertex.
  void MoveOutlineVerticesToTarget(const DirectedPartialOutline& outline,
                                   uint32_t start, uint32_t end,
                                   const ExtrudedVertex& target);

  // Finds the first vertex of `outline` that is not coincident with the vertex
  // at `outline[0]`.
  //
  // Returns an index into `outline` if such a vertex is found.
  std::optional<uint32_t> FirstVertexNotAtOutlineStart(
      const DirectedPartialOutline& outline) const;

  // Tries to extend the start of `outline` to meet `segment`.
  //
  // If the start of the outline consists of multiple degenerate vertices, they
  // will all be moved. Vertices will not be moved by more than
  // `max_extension_distance`. Returns true if extension succeeds.
  ABSL_MUST_USE_RESULT bool ExtendOutlineToSegment(
      Side& outline_starting_side, const DirectedPartialOutline& outline,
      const Segment& segment, float max_extension_distance);

  // Assigns all vertices in the range [begin, end) to the value of `target`.
  void AssignVerticesInRange(std::vector<IndexType>::iterator begin,
                             std::vector<IndexType>::iterator end,
                             const ExtrudedVertex& target);

  // Tries to perform the initial breaking up of triangles in intersection
  // handling that has exceeded the retriangulation threshold.
  //
  // Tries to break up triangles between the end of the mesh and
  // `intersection_vertex_triangle`. The action is not performed and the
  // function returns false if doing so would cause clockwise-winding
  // triangles because of `uncorrected_intersection_vertex` and a corrected
  // vertex cannot be computed.
  bool TryBeginIntersectionRetriangulation(
      Side& intersecting_side,
      const ExtrudedVertex& uncorrected_intersection_vertex,
      uint32_t intersection_vertex_triangle);

  // Continues to handle mesh triangle modification for an intersection.
  //
  // Will undo triangle modification if `intersection_vertex_triangle` is
  // increasing compared to the `oldest_retriangulation_triangle` in the
  // intersection. Otherwise, breaks up more triangles similarly to
  // `TryBeginIntersectionRetriangulation`.
  void ContinueIntersectionRetriangulation(
      Side& intersecting_side,
      const ExtrudedVertex& uncorrected_intersection_vertex,
      uint32_t intersection_vertex_triangle);

  // Restores some or all of the triangles that were broken up by
  // `BeginIntersectionRetriangulation` and
  // `ContinueIntersectionRetriangulation` by traversing the
  // `undo_triangulation_stack` on `intersecting_side->intersection`.
  //
  // If `stop_at_position` is not nullopt, the function stops when it reaches a
  // triangle that contains the position.
  void UndoIntersectionRetriangulation(
      Side& intersecting_side,
      std::optional<Point> stop_at_position = std::nullopt);

  // Returns the offset ranges of the left and right sides of the stroke that
  // would be part of the triangle fans with the given `intersection_vertex`.
  // See `FindLastClockwiseWindingMultiTriangleFanSegment`.
  Side::IndexOffsetRanges GetIntersectionTriangleFanOffsetRanges(
      const Side& intersecting_side, const ExtrudedVertex& intersection_vertex,
      uint32_t intersection_vertex_triangle) const;

  // If possible, returns a corrected `intersection_vertex` that will not cause
  // clockwise winding triangles when used as the new intersection pivot.
  //
  // The correction is meant to stretch and still use the
  // `intersection_vertex_triangle` as opposed to finding a new value of it.
  std::optional<ExtrudedVertex> MakeWindingCorrectedIntersectionVertex(
      const Side& intersecting_side, const ExtrudedVertex& intersection_vertex,
      uint32_t intersection_vertex_triangle) const;

  // Returns true if moving vertices of `outline` preceding the `intersection`
  // to `target_position` would result in one or more clockwise triangles.
  bool MovingStartingOutlineVerticesWouldCauseClockwiseTriangle(
      const Side& outline_starting_side, const DirectedPartialOutline& outline,
      const OutlineIntersectionResult::SegmentIntersection& intersection,
      Point target_position,
      bool stop_at_oldest_retriangulation_triangle = true) const;

  // Updates the pivot vertices for an ongoing intersection.
  void UpdateIntersectionPivotVertices(Side& intersecting_side,
                                       const ExtrudedVertex& new_pivot_vertex);

  // Updates the vertices on the opposite side of an intersection and returns
  // the envelope of any vertices modified. If the line modifier does not assign
  // winding texture coordinates, this function is a no-op.
  void UpdateIntersectionOuterVertices(Side& intersecting_side,
                                       IndexType pivot_start,
                                       IndexType pivot_end);

  // Tries to cleanly finish intersection handling assuming the vertex at
  // `new_index` is now in the exterior of the line. If this function succeeds,
  // it returns the envelope of any modified vertices. It may fail if an
  // intersection between the outline and the newest segment cannot be found. In
  // that case, it calls `GiveUpIntersectionHandling` with `intersecting_side`.
  void TryFinishIntersectionHandling(Side& intersecting_side,
                                     const ExtrudedVertex& new_vertex,
                                     const DirectedPartialOutline& outline);

  // Exits an unfinished self-intersection handling and starts a new mesh
  // partition if the intersection exits mid-retriangulation.
  //
  // In case a new partition is made, this function appends duplicate vertices
  // to the left and right sides to be part of the next triangle.
  void GiveUpIntersectionHandling(Side& intersecting_side);

  // Checks if the vertices on either side of the stroke no longer seamlessly
  // connect to the end of the previous partition, and reconnects them if
  // necessary.
  //
  // The disconnect is a result of how non-ccw exterior vertices must be
  // handled to assume a stroke that will continue to make a loop. However,
  // future vertices can indicate that the stroke actually has jittery
  // left-right behavior or may turn in the opposite direction.
  void UndoNonCcwPartitionSeparationIfNeeded(
      TriangleWinding proposed_winding, Side& new_vertex_side,
      const ExtrudedVertex& proposed_vertex);

  // Returns true if the opposite side has caused the vertex at
  // `side.partition.opposite_first_index_offset` to be repositioned.
  bool OppositeSideMovedPartitionInitialPosition(const Side& side) const;

  void DisconnectPartitionSides(Side& side);

  // Helper class for appending a single triangle to the mesh and handling the
  // different self-intersection cases.
  class TriangleBuilder {
   public:
    TriangleBuilder(Geometry* absl_nonnull geometry,
                    float initial_outline_reposition_budget,
                    float intersection_travel_limit,
                    float retriangulation_travel_threshold);

    // Tries to append the next buffered vertex from `new_vertex_side` and
    // create a new triangle using it and the last appended vertices on the left
    // and right side.
    //
    // The proposed vertex and or triangle may be rejected and old geometry may
    // be modified.
    void TryAppend(Side& new_vertex_side);

   private:
    // Collection of triangle data used when the slow-path is taken. Here,
    // "adjacent" refers to properties of the `new_index_side_`, and "opposite"
    // is used to refer to the other side.
    struct SlowPathTriangleInfo {
      Side* absl_nonnull adjacent_side;
      Side* absl_nonnull opposite_side;
      Point adjacent_position;
      Point opposite_position;
      ExtrudedVertex proposed_vertex;
      // The index of an existing triangle in `geometry_->mesh_` that contains
      // `proposed_vertex` if one is found.
      std::optional<uint32_t> proposed_vertex_triangle;
    };

    SlowPathTriangleInfo MakeSlowPathInfo(
        TriangleWinding proposed_winding, Side& new_vertex_side,
        const ExtrudedVertex& proposed_vertex);

    // Returns true if the adjacent and opposite sides of the triangle touch,
    // creating a special subcase of a degenerate triangle.
    bool SidesTouch(const SlowPathTriangleInfo& info);

    // Handles the slow-path cases including self-intersection handling.
    //
    // The problem is broken down into eight main cases given by the
    // combinations of the following three criteria:
    //   1. Does the proposed triangle have counterclockwise winding?
    //   2. Is `adjacent_side_` currently handling self-intersection?
    //   3. Is `opposite_side_` currently handling self-intersection?
    //
    // (1) "Yes" and (2)/(3) "No" represents the fast-path case. The other seven
    // possibilities are handled in the functions below.
    void TryAppendSlowPath(TriangleWinding proposed_winding,
                           const SlowPathTriangleInfo& info);
    void HandleCcwAdjacentIntersectingTriangle(
        const SlowPathTriangleInfo& info);
    void HandleCcwOppositeIntersectingTriangle(
        const SlowPathTriangleInfo& info);
    void HandleCcwBothSidesIntersectingTriangle(
        const SlowPathTriangleInfo& info);
    void HandleNonCcwNonIntersectingTriangle(const SlowPathTriangleInfo& info);
    void HandleNonCcwAdjacentIntersectingTriangle(
        const SlowPathTriangleInfo& info);
    void HandleNonCcwOppositeIntersectingTriangle(
        const SlowPathTriangleInfo& info);
    void HandleNonCcwBothSidesIntersectingTriangle(
        const SlowPathTriangleInfo& info);

    void HandleCcwAdjacentIntersectingTriangleHelper(
        const SlowPathTriangleInfo& info,
        bool allowed_to_begin_retriangulation);
    void HandleCcwOppositeIntersectingTriangleHelper(
        const SlowPathTriangleInfo& info);

    Side::SelfIntersection MakeAdjacentSelfIntersection(
        const SlowPathTriangleInfo& info) const;

    Geometry* absl_nonnull geometry_;
    float initial_outline_reposition_budget_;
    float intersection_travel_limit_;
    float retriangulation_travel_threshold_;
  };

  // Assigns the value of a vertex in `mesh_`.
  //
  // This function also:
  //   * Updates `first_mutated_left_index_offset_in_current_partition_` or
  //     `first_mutated_right_index_offset_in_current_partition_` depending on
  //     the side to which `index` belongs.
  //   * If `update_envelope_of_removed_geometry` is true, adds the position of
  //     vertex had prior to this call to `envelope_of_removed_geometry_`.
  //   * If `update_save_state` is true and a save point is set, saves the
  //     current value of the vertex if necessary.
  void SetVertex(IndexType index, const ExtrudedVertex& new_vertex,
                 bool update_save_state = true,
                 bool update_envelope_of_removed_geometry = true);

  // Assigns the value of triangle indices in `mesh_`. Updates
  // `oldest_mutated_triangle_` if necessary and invalidates cached mesh
  // triangulation.
  //
  // If `update_save_state` is true and a save point is set, the function will
  // save the current value of the triangle indices if necessary.
  void SetTriangleIndices(uint32_t triangle_index,
                          const std::array<IndexType, 3>& new_indices,
                          bool update_save_state = true);

  // Sets a new value for an existing index's `opposite_side_offset_`. If
  // `update_save_state` is true and a save point is set, the function will save
  // the current value as needed.
  void UpdateOppositeSideOffset(IndexType index, uint32_t new_offset,
                                bool update_save_state = true);

  // The following two functions handle starting and ending a triangle fan in
  // the case that the line modifier is assigning winding texture coordinates.
  // In this case, the pivot position of the triangle fan must consist of three
  // super-imposed vertices with different texture coordinates.
  void BeginSuperImposedPivotFan(Side& fan_pivot_side, Side& fan_outer_side);
  void EndSuperImposedPivotFan(Side& fan_pivot_side, Side& fan_outer_side);

  bool MeshEndsInSuperImposedPivot() const;

  // Empties `side->vertex_buffer` except for the last one or two vertices as
  // needed for the simplification algorithm during the next call to
  // `ProcessNewVertices`.
  void PrepBufferedVerticesForNextExtrusion(Side& side,
                                            bool must_keep_last_vertex);

  // Returns the first triangle of the `mesh_` that should be considered
  // visually updated.
  //
  // The returned value will be no greater than `mesh_.FirstMutatedTriangle()`,
  // but will be smaller if needed in order to include any triangles connected
  // to indices starting at `first_mutated_left_index_` and
  // `first_mutated_right_index_`.
  uint32_t FirstVisuallyMutatedTriangle() const;

  // Appends and triangulates buffered vertices from from the left and right
  // side.
  //
  // `initial_outline_reposition_budget` is the initial value given to
  // `Side::SelfIntersection::outline_reposition_budget` when an intersection is
  // begun. If no intersection is currently ongoing, it is also the default
  // value used by `MoveStartingVerticesToIntersection()`.
  //
  // `intersction_travel_limit` is the value to be given to
  // `Side::SelfIntersection::travel_limit_from_starting_position` if an
  // intersection begins on this call.
  //
  // `retriangulation_travel_threshold` is the minimum distance between the
  // start of a self-intersection and a newly proposed vertex that will trigger
  // retriangulation of the existing mesh.
  void TriangulateBufferedVertices(float initial_outline_reposition_budget,
                                   float intersection_travel_limit,
                                   float retriangulation_travel_threshold);

  bool handle_self_intersections_ = true;

  TextureCoordType texture_coord_type_ = TextureCoordType::kTiling;

  MutableMeshView mesh_;

  // Identifies which side each vertex in `mesh_` comes from.
  std::vector<SideId> vertex_side_ids_;
  // For each vertex in `mesh_`, stores its offset into `Side::indices` on the
  // side identified by the corresponding value in `vertex_side_ids_`.
  std::vector<uint32_t> side_offsets_;
  // For each vertex in `mesh_`, stores the first offset into the opposite
  // side's `indices` for a vertex that can be part of the same triangle.
  std::vector<uint32_t> opposite_side_offsets_;

  // The left and right sides of the line according to the direction of travel.
  Side left_side_;
  Side right_side_;

  GeometryLastExtrusionBreakMetadata last_extrusion_break_;

  // Intermediate storage used by `SimplifyBufferedVertices()` and kept as a
  // member variable to reuse allocations.
  std::vector<ExtrudedVertex> simplification_vertex_buffer_;

  // The save state for the geometry. This only contains a valid save state when
  // `save_point_state_.is_active` is true.
  GeometrySavePointState save_point_state_;

  // Envelope tracking mutations of geometry that would not be recovered by
  // inspecting the `mesh_`. This happens when a position is overwritten or a
  // vertex stops being part of the triangulation.
  Envelope envelope_of_removed_geometry_;

  // The following members keep track of when existing vertices of each side are
  // modified.
  std::optional<IndexType> first_mutated_left_index_;
  std::optional<IndexType> first_mutated_right_index_;

  // The following members keep track of modifications to `left_side_.indices`
  // and `right_side_.indices` following the latest extrusion break.
  uint32_t first_mutated_left_index_offset_in_current_partition_ = 0;
  uint32_t first_mutated_right_index_offset_in_current_partition_ = 0;

  DerivativeCalculator derivative_calculator_;
};

// --------------------------------------------------------------------------
//                      Implementation details follow

inline void Geometry::SetTextureCoordType(TextureCoordType type) {
  texture_coord_type_ = type;
}

inline const MutableMeshView& Geometry::GetMeshView() const { return mesh_; }

inline const Side& Geometry::LeftSide() const { return left_side_; }

inline const Side& Geometry::RightSide() const { return right_side_; }

inline uint32_t Geometry::FirstMutatedLeftIndexOffsetInCurrentPartition()
    const {
  return first_mutated_left_index_offset_in_current_partition_;
}

inline uint32_t Geometry::FirstMutatedRightIndexOffsetInCurrentPartition()
    const {
  return first_mutated_right_index_offset_in_current_partition_;
}

}  // namespace brush_tip_extruder_internal
}  // namespace ink

#endif  //  INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_GEOMETRY_H_
