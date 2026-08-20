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

#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"
#include "ink/strokes/internal/legacy_vertex.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::LegacyVertex;
using ::ink::strokes_internal::StrokeVertex;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

TEST(MutableMeshViewTest, HasMeshData) {
  MutableMeshView mesh_view;
  EXPECT_FALSE(mesh_view.HasMeshData());

  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  mesh_view = MutableMeshView(vertices, indices);
  EXPECT_TRUE(mesh_view.HasMeshData());

  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh_view = MutableMeshView(mesh);
  EXPECT_TRUE(mesh_view.HasMeshData());
}

TEST(MutableMeshViewTest, VertexCountOnAppendWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  EXPECT_EQ(mesh_view.VertexCount(), 0);
  EXPECT_EQ(vertices.size(), mesh_view.VertexCount());

  vertices.push_back({});
  EXPECT_EQ(mesh_view.VertexCount(), 1);
  EXPECT_EQ(vertices.size(), mesh_view.VertexCount());

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  EXPECT_EQ(mesh_view.VertexCount(), 4);
  EXPECT_EQ(vertices.size(), mesh_view.VertexCount());

  vertices.push_back({});
  vertices.push_back({});
  EXPECT_EQ(mesh_view.VertexCount(), 6);
  EXPECT_EQ(vertices.size(), mesh_view.VertexCount());
}

TEST(MutableMeshViewTest, VertexCountOnAppendWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  EXPECT_EQ(mesh_view.VertexCount(), 0);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());

  mesh.AppendVertex({});
  EXPECT_EQ(mesh_view.VertexCount(), 1);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  EXPECT_EQ(mesh_view.VertexCount(), 4);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  EXPECT_EQ(mesh_view.VertexCount(), 6);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());
}

TEST(MutableMeshViewTest, TriangleCountOnAppendWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  EXPECT_EQ(mesh_view.TriangleCount(), 0);
  EXPECT_EQ(indices.size(), 3 * mesh_view.TriangleCount());

  indices.insert(indices.end(), {0, 1, 2});
  EXPECT_EQ(mesh_view.TriangleCount(), 1);
  EXPECT_EQ(indices.size(), 3 * mesh_view.TriangleCount());

  mesh_view.AppendTriangleIndices({0, 1, 2});
  mesh_view.AppendTriangleIndices({0, 1, 2});
  mesh_view.AppendTriangleIndices({0, 1, 2});
  EXPECT_EQ(mesh_view.TriangleCount(), 4);
  EXPECT_EQ(indices.size(), 3 * mesh_view.TriangleCount());

  mesh_view.AppendTriangleIndices({0, 1, 2});
  mesh_view.AppendTriangleIndices({0, 1, 2});
  EXPECT_EQ(mesh_view.TriangleCount(), 6);
  EXPECT_EQ(indices.size(), 3 * mesh_view.TriangleCount());
}

TEST(MutableMeshViewTest, TriangleCountOnAppendWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  EXPECT_EQ(mesh_view.TriangleCount(), 0);
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());

  mesh.AppendTriangleIndices({0, 1, 2});
  EXPECT_EQ(mesh_view.TriangleCount(), 1);
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());

  mesh_view.AppendTriangleIndices({0, 1, 2});
  mesh_view.AppendTriangleIndices({0, 1, 2});
  mesh_view.AppendTriangleIndices({0, 1, 2});
  EXPECT_EQ(mesh_view.TriangleCount(), 4);
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());

  mesh_view.AppendTriangleIndices({0, 1, 2});
  mesh_view.AppendTriangleIndices({0, 1, 2});
  EXPECT_EQ(mesh_view.TriangleCount(), 6);
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());
}

TEST(MutableMeshViewTest, GetAndSetPositionAndVertexWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  ExtrudedVertex appended_vertex = {.position = {1, 2},
                                    .color = {3, 4, 5, 6},
                                    .texture_coords = {7, 8},
                                    .secondary_texture_coords = {9, 10}};
  mesh_view.AppendVertex(appended_vertex);
  mesh_view.AppendVertex({});

  EXPECT_THAT(mesh_view.GetPosition(2), PointEq(appended_vertex.position));

  ExtrudedVertex stored_vertex = mesh_view.GetVertex(2);
  EXPECT_EQ(stored_vertex, appended_vertex);
  EXPECT_EQ(stored_vertex.ToLegacy(), vertices.at(2));

  ExtrudedVertex set_vertex = {.position = {-2, -3},
                               .color = {-4, -5, -6, -7},
                               .texture_coords = {-8, -9},
                               .secondary_texture_coords = {-10, -11}};
  mesh_view.SetVertex(2, set_vertex);
  stored_vertex = mesh_view.GetVertex(2);
  EXPECT_EQ(stored_vertex, set_vertex);
  EXPECT_EQ(stored_vertex.ToLegacy(), vertices.at(2));
}

TEST(MutableMeshViewTest, GetAndSetPositionAndVertexWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  ExtrudedVertex appended_vertex = {
      .position = {1, 2},
      .new_non_position_attributes = {
          .opacity_shift = 0.5,
          .hsl_shift = {0.1, -0.3, 0.8},
          .side_derivative = {3, 4},
          .side_label = StrokeVertex::kExteriorLeftLabel,
          .forward_derivative = {6, 7},
          .forward_label = StrokeVertex::kExteriorBackLabel}};
  mesh_view.AppendVertex(appended_vertex);
  mesh_view.AppendVertex({});

  EXPECT_THAT(mesh_view.GetPosition(2), PointEq(appended_vertex.position));

  ExtrudedVertex stored_vertex = mesh_view.GetVertex(2);
  EXPECT_EQ(stored_vertex, appended_vertex);
  EXPECT_THAT(stored_vertex.position, PointEq(mesh.VertexPosition(2)));

  ExtrudedVertex set_vertex = {
      .position = {-2, -3},
      .new_non_position_attributes = {.opacity_shift = -0.3,
                                      .hsl_shift = {0.5, 0.4, 1.0},
                                      .side_derivative = {-4, -5},
                                      .side_label =
                                          StrokeVertex::kExteriorRightLabel,
                                      .forward_derivative = {-7, -8},
                                      .forward_label =
                                          StrokeVertex::kExteriorFrontLabel},
  };
  mesh_view.SetVertex(2, set_vertex);
  stored_vertex = mesh_view.GetVertex(2);
  EXPECT_EQ(stored_vertex, set_vertex);
  EXPECT_THAT(stored_vertex.position, PointEq(mesh.VertexPosition(2)));
}

TEST(MutableMeshViewTest, GetDerivativesAndLabelsWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  // Setting the values of "new non position attributes" on legacy vertices will
  // be ignored.
  mesh_view.AppendVertex({.new_non_position_attributes{
      .side_derivative = {1, 2},
      .side_label = StrokeVertex::kExteriorLeftLabel,
      .forward_derivative = {3, 4},
      .forward_label = StrokeVertex::kExteriorFrontLabel}});
  // Getting the derivative values should always return the zero-vector, and
  // getting the label values should always return `kInteriorLabel`.
  EXPECT_THAT(mesh_view.GetSideDerivative(0), VecEq({0, 0}));
  EXPECT_THAT(mesh_view.GetForwardDerivative(0), VecEq({0, 0}));
  EXPECT_EQ(mesh_view.GetSideLabel(0), StrokeVertex::kInteriorLabel);
  EXPECT_EQ(mesh_view.GetForwardLabel(0), StrokeVertex::kInteriorLabel);
}

TEST(MutableMeshViewTest, GetDerivativesAndLabelsWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  ExtrudedVertex appended_vertex = {
      .position = {1, 2},
      .new_non_position_attributes = {
          .opacity_shift = -0.8,
          .hsl_shift = {0.2, 0.3, 0.4},
          .side_derivative = {3, 4},
          .side_label = StrokeVertex::kExteriorLeftLabel,
          .forward_derivative = {6, 7},
          .forward_label = StrokeVertex::kExteriorBackLabel}};
  mesh_view.AppendVertex(appended_vertex);
  mesh_view.AppendVertex({});

  EXPECT_THAT(mesh_view.GetSideDerivative(2), VecEq({3, 4}));
  EXPECT_THAT(mesh_view.GetForwardDerivative(2), VecEq({6, 7}));
  EXPECT_EQ(mesh_view.GetSideLabel(2),
            appended_vertex.new_non_position_attributes.side_label);
  EXPECT_EQ(mesh_view.GetForwardLabel(2),
            appended_vertex.new_non_position_attributes.forward_label);
}

TEST(MutableMeshViewTest, SetDerivativesAndLabelsWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  mesh_view.AppendVertex({.position = {1, 2}});

  // Setting the value of derivative on legacy vertices should be ignored.
  mesh_view.SetSideDerivative(0, {1, 7});
  mesh_view.SetForwardDerivative(0, {5, 3});
  mesh_view.SetSideLabel(0, StrokeVertex::kExteriorRightLabel);
  mesh_view.SetForwardLabel(0, StrokeVertex::kExteriorFrontLabel);
  EXPECT_THAT(vertices, ElementsAreArray({LegacyVertex{.position = {1, 2}}}));
}

TEST(MutableMeshViewTest, SetDerivativesAndLabelsWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});

  ASSERT_THAT(
      mesh.FloatVertexAttribute(
              2, StrokeVertex::kFullFormatAttributeIndices.side_derivative)
          .Values(),
      ElementsAre(0, 0));
  ASSERT_THAT(
      mesh.FloatVertexAttribute(
              1, StrokeVertex::kFullFormatAttributeIndices.forward_derivative)
          .Values(),
      ElementsAre(0, 0));
  ASSERT_THAT(mesh.FloatVertexAttribute(
                      1, StrokeVertex::kFullFormatAttributeIndices.side_label)
                  .Values(),
              ElementsAre(StrokeVertex::kInteriorLabel.encoded_value));
  ASSERT_THAT(
      mesh.FloatVertexAttribute(
              2, StrokeVertex::kFullFormatAttributeIndices.forward_label)
          .Values(),
      ElementsAre(StrokeVertex::kInteriorLabel.encoded_value));

  mesh_view.SetSideDerivative(2, {3, 5});
  mesh_view.SetForwardDerivative(1, {2, 7});
  mesh_view.SetSideLabel(1, StrokeVertex::kExteriorRightLabel);
  mesh_view.SetForwardLabel(2, StrokeVertex::kExteriorFrontLabel);

  EXPECT_THAT(
      mesh.FloatVertexAttribute(
              2, StrokeVertex::kFullFormatAttributeIndices.side_derivative)
          .Values(),
      ElementsAre(3, 5));
  EXPECT_THAT(
      mesh.FloatVertexAttribute(
              1, StrokeVertex::kFullFormatAttributeIndices.forward_derivative)
          .Values(),
      ElementsAre(2, 7));
  EXPECT_THAT(mesh.FloatVertexAttribute(
                      1, StrokeVertex::kFullFormatAttributeIndices.side_label)
                  .Values(),
              ElementsAre(StrokeVertex::kExteriorRightLabel.encoded_value));
  EXPECT_THAT(
      mesh.FloatVertexAttribute(
              2, StrokeVertex::kFullFormatAttributeIndices.forward_label)
          .Values(),
      ElementsAre(StrokeVertex::kExteriorFrontLabel.encoded_value));
}

TEST(MutableMeshViewTest, GetTriangleAndIndicesWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({.position = {1, 2}});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({.position = {2, 3}});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({.position = {4, 5}});
  mesh_view.AppendVertex({});

  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({1, 3, 5});
  mesh_view.AppendTriangleIndices({});

  EXPECT_THAT(mesh_view.GetTriangleIndices(1), ElementsAreArray({1, 3, 5}));
  EXPECT_EQ(mesh_view.GetVertexIndex(1, 0), 1);
  EXPECT_EQ(mesh_view.GetVertexIndex(1, 1), 3);
  EXPECT_EQ(mesh_view.GetVertexIndex(1, 2), 5);

  EXPECT_THAT(mesh_view.GetTriangle(1),
              TriangleEq({.p0 = {1, 2}, .p1 = {2, 3}, .p2 = {4, 5}}));
}

TEST(MutableMeshViewTest, GetTriangleAndIndicesWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({.position = {1, 2}});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({.position = {2, 3}});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({.position = {4, 5}});
  mesh_view.AppendVertex({});

  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({1, 3, 5});
  mesh_view.AppendTriangleIndices({});

  EXPECT_THAT(mesh.TriangleIndices(1), ElementsAreArray({1, 3, 5}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(1), ElementsAreArray({1, 3, 5}));
  EXPECT_EQ(mesh_view.GetVertexIndex(1, 0), 1);
  EXPECT_EQ(mesh_view.GetVertexIndex(1, 1), 3);
  EXPECT_EQ(mesh_view.GetVertexIndex(1, 2), 5);

  EXPECT_THAT(mesh.GetTriangle(1),
              TriangleEq({.p0 = {1, 2}, .p1 = {2, 3}, .p2 = {4, 5}}));
  EXPECT_THAT(mesh_view.GetTriangle(1),
              TriangleEq({.p0 = {1, 2}, .p1 = {2, 3}, .p2 = {4, 5}}));
}

TEST(MutableMeshViewTest, SetTriangleIndicesWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({1, 3, 5});
  mesh_view.AppendTriangleIndices({});

  mesh_view.SetTriangleIndices(3, {3, 6, 7});
  EXPECT_THAT(indices,
              ElementsAreArray({0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 6, 7, 0, 0, 0}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(3), ElementsAreArray({3, 6, 7}));
}

TEST(MutableMeshViewTest, SetTriangleIndicesWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({});
  mesh_view.AppendTriangleIndices({1, 3, 5});
  mesh_view.AppendTriangleIndices({});

  mesh_view.SetTriangleIndices(3, {3, 6, 7});
  EXPECT_THAT(mesh.TriangleIndices(3), ElementsAreArray({3, 6, 7}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(3), ElementsAreArray({3, 6, 7}));
}

TEST(MutableMeshViewTest, InsertTriangleIndicesWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  mesh_view.AppendTriangleIndices({1, 2, 3});
  mesh_view.AppendTriangleIndices({4, 5, 6});

  mesh_view.InsertTriangleIndices(1, {7, 8, 9});
  EXPECT_THAT(indices, ElementsAreArray({1, 2, 3, 7, 8, 9, 4, 5, 6}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(0), ElementsAreArray({1, 2, 3}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(1), ElementsAreArray({7, 8, 9}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(2), ElementsAreArray({4, 5, 6}));
}

TEST(MutableMeshViewTest, InsertTriangleIndicesWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  mesh_view.AppendTriangleIndices({1, 2, 3});
  mesh_view.AppendTriangleIndices({4, 5, 6});

  mesh_view.InsertTriangleIndices(1, {7, 8, 9});
  EXPECT_THAT(mesh.TriangleIndices(0), ElementsAreArray({1, 2, 3}));
  EXPECT_THAT(mesh.TriangleIndices(1), ElementsAreArray({7, 8, 9}));
  EXPECT_THAT(mesh.TriangleIndices(2), ElementsAreArray({4, 5, 6}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(0), ElementsAreArray({1, 2, 3}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(1), ElementsAreArray({7, 8, 9}));
  EXPECT_THAT(mesh_view.GetTriangleIndices(2), ElementsAreArray({4, 5, 6}));
}

TEST(MutableMeshViewTest, ResizeWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  vertices.resize(20);
  indices.resize(3 * 12);

  EXPECT_EQ(mesh_view.VertexCount(), 20);
  EXPECT_EQ(mesh_view.TriangleCount(), 12);

  mesh_view.Resize(28, 16);
  EXPECT_EQ(mesh_view.VertexCount(), 28);
  EXPECT_EQ(mesh_view.TriangleCount(), 16);

  mesh_view.Resize(10, 7);
  EXPECT_EQ(mesh_view.VertexCount(), 10);
  EXPECT_EQ(mesh_view.TriangleCount(), 7);

  mesh_view.Resize(0, 0);
  EXPECT_EQ(mesh_view.VertexCount(), 0);
  EXPECT_EQ(mesh_view.TriangleCount(), 0);

  EXPECT_TRUE(vertices.empty());
  EXPECT_TRUE(indices.empty());
}

TEST(MutableMeshViewTest, ResizeWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);
  mesh.Resize(/* new_vertex_count = */ 20, /* new_triangle_count = */ 12);

  EXPECT_EQ(mesh_view.VertexCount(), 20);
  EXPECT_EQ(mesh_view.TriangleCount(), 12);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());

  mesh_view.Resize(28, 16);
  EXPECT_EQ(mesh_view.VertexCount(), 28);
  EXPECT_EQ(mesh_view.TriangleCount(), 16);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());

  mesh_view.Resize(10, 7);
  EXPECT_EQ(mesh_view.VertexCount(), 10);
  EXPECT_EQ(mesh_view.TriangleCount(), 7);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());

  mesh_view.Resize(0, 0);
  EXPECT_EQ(mesh_view.VertexCount(), 0);
  EXPECT_EQ(mesh_view.TriangleCount(), 0);
  EXPECT_EQ(mesh.VertexCount(), mesh_view.VertexCount());
  EXPECT_EQ(mesh.TriangleCount(), mesh_view.TriangleCount());
}

TEST(MutableMeshViewTest, ResizeWithLegacyVectorsInitializesToZero) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);
  mesh_view.AppendVertex({.position = {0, 0}});
  mesh_view.AppendVertex({.position = {1, 0}});
  mesh_view.AppendVertex({.position = {1, 1}});
  mesh_view.AppendTriangleIndices({0, 1, 2});

  mesh_view.Resize(5, 3);

  EXPECT_EQ(mesh_view.GetVertex(0), (ExtrudedVertex{.position = {0, 0}}));
  EXPECT_EQ(mesh_view.GetVertex(1), (ExtrudedVertex{.position = {1, 0}}));
  EXPECT_EQ(mesh_view.GetVertex(2), (ExtrudedVertex{.position = {1, 1}}));
  EXPECT_EQ(mesh_view.GetVertex(3), ExtrudedVertex{});
  EXPECT_EQ(mesh_view.GetVertex(4), ExtrudedVertex{});

  EXPECT_THAT(mesh_view.GetTriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(mesh_view.GetTriangleIndices(1), ElementsAre(0, 0, 0));
  EXPECT_THAT(mesh_view.GetTriangleIndices(2), ElementsAre(0, 0, 0));
}

TEST(MutableMeshViewTest, ResizeWithNewMeshInitializesToZero) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);
  mesh_view.AppendVertex({.position = {0, 0}});
  mesh_view.AppendVertex({.position = {1, 0}});
  mesh_view.AppendVertex({.position = {1, 1}});
  mesh_view.AppendTriangleIndices({0, 1, 2});

  mesh_view.Resize(5, 3);

  EXPECT_EQ(mesh_view.GetVertex(0), (ExtrudedVertex{.position = {0, 0}}));
  EXPECT_EQ(mesh_view.GetVertex(1), (ExtrudedVertex{.position = {1, 0}}));
  EXPECT_EQ(mesh_view.GetVertex(2), (ExtrudedVertex{.position = {1, 1}}));
  EXPECT_EQ(mesh_view.GetVertex(3), ExtrudedVertex{});
  EXPECT_EQ(mesh_view.GetVertex(4), ExtrudedVertex{});

  EXPECT_THAT(mesh_view.GetTriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(mesh_view.GetTriangleIndices(1), ElementsAre(0, 0, 0));
  EXPECT_THAT(mesh_view.GetTriangleIndices(2), ElementsAre(0, 0, 0));
}

TEST(MutableMeshViewTest, MutationTrackingWithLegacyVectors) {
  std::vector<LegacyVertex> vertices;
  std::vector<uint32_t> indices;
  MutableMeshView mesh_view(vertices, indices);

  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 0);
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 0);

  vertices.resize(5);
  indices.resize(3 * 4);

  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 0);
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 0);

  mesh_view = MutableMeshView(vertices, indices);
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount());
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());

  mesh_view.AppendVertex({});
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount() - 1);
  mesh_view.AppendTriangleIndices({});
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount() - 1);

  mesh_view.SetVertex(3, {});
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 3);
  mesh_view.SetTriangleIndices(2, {});
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 2);

  mesh_view.InsertTriangleIndices(1, {});
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 1);

  mesh_view.ResetMutationTracking();
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount());
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());

  mesh_view.Resize(3, 2);
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount());
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());

  mesh_view.ResetMutationTracking();
  mesh_view.Resize(5, 3);
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 3);
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 2);

  // Setting the side and forward derivatives on legacy data should be ignored.
  mesh_view.ResetMutationTracking();
  mesh_view.SetSideDerivative(2, {1, 0});
  mesh_view.SetForwardDerivative(1, {3, 5});
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount());
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());
}

TEST(MutableMeshViewTest, MutationTrackingWithNewMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 0);
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 0);

  mesh.Resize(/* new_vertex_count = */ 5, /* new_triangle_count = */ 4);

  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 0);
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 0);

  mesh_view = MutableMeshView(mesh);
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount());
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());

  mesh_view.AppendVertex({});
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount() - 1);
  mesh_view.AppendTriangleIndices({});
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount() - 1);

  mesh_view.SetVertex(3, {});
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 3);
  mesh_view.SetTriangleIndices(2, {});
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 2);

  mesh_view.InsertTriangleIndices(1, {});
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 1);

  mesh_view.ResetMutationTracking();
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount());
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());

  mesh_view.Resize(3, 2);
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), mesh_view.VertexCount());
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());

  mesh_view.ResetMutationTracking();
  mesh_view.Resize(5, 3);
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 3);
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), 2);

  mesh_view.ResetMutationTracking();
  mesh_view.SetSideDerivative(2, {1, 0});
  mesh_view.SetForwardDerivative(1, {3, 5});
  EXPECT_EQ(mesh_view.FirstMutatedVertex(), 1);
  EXPECT_EQ(mesh_view.FirstMutatedTriangle(), mesh_view.TriangleCount());
}

TEST(MutableMeshViewDeathTest, ConstructedWithIncompatibleMutableMeshFormat) {
  MutableMesh mesh;
  EXPECT_DEATH_IF_SUPPORTED(MutableMeshView mesh_view(mesh), "");
}

TEST(MutableMeshViewDeathTest, DefaultConstructedVertexCount) {
  MutableMeshView mesh_view;
  ASSERT_FALSE(mesh_view.HasMeshData());
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.VertexCount(), "");
}

TEST(MutableMeshViewDeathTest, DefaultConstructedTriangleCount) {
  MutableMeshView mesh_view;
  ASSERT_FALSE(mesh_view.HasMeshData());
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.TriangleCount(), "");
}

TEST(MutableMeshViewDeathTest, DefaultConstructedAppendVertex) {
  MutableMeshView mesh_view;
  ASSERT_FALSE(mesh_view.HasMeshData());
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.AppendVertex({}), "");
}

TEST(MutableMeshViewDeathTest, DefaultConstructedAppendTriangleIndices) {
  MutableMeshView mesh_view;
  ASSERT_FALSE(mesh_view.HasMeshData());
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.AppendTriangleIndices({}), "");
}

TEST(MutableMeshViewDeathTest, GetPositionOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.VertexCount(), 5);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetPosition(5), "");
}

TEST(MutableMeshViewDeathTest, GetSideLabelOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.VertexCount(), 5);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetSideLabel(5), "");
}

TEST(MutableMeshViewDeathTest, GetForwardLabelOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.VertexCount(), 5);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetForwardLabel(5), "");
}

TEST(MutableMeshViewDeathTest, GetVertexOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.VertexCount(), 5);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetVertex(5), "");
}

TEST(MutableMeshViewDeathTest, GetTriangleOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.TriangleCount(), 4);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetTriangle(4), "");
}

TEST(MutableMeshViewDeathTest, GetTriangleIndicesOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.TriangleCount(), 4);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetTriangleIndices(4), "");
}

TEST(MutableMeshViewDeathTest, GetVertexIndexFirstParamOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.TriangleCount(), 4);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetVertexIndex(4, 0), "");
}

TEST(MutableMeshViewDeathTest, GetVertexIndexSecondParamOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_GT(mesh_view.TriangleCount(), 0);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.GetVertexIndex(0, 3), "");
}

TEST(MutableMeshViewDeathTest, SetSideDerivativeOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.VertexCount(), 5);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.SetSideDerivative(5, {}), "");
}

TEST(MutableMeshViewDeathTest, SetForwardDerivativeOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.VertexCount(), 5);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.SetForwardDerivative(5, {}), "");
}

TEST(MutableMeshViewDeathTest, SetVertexOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.VertexCount(), 5);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.SetVertex(5, {}), "");
}

TEST(MutableMeshViewDeathTest, SetTriangleIndicesOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.TriangleCount(), 4);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.SetTriangleIndices(4, {}), "");
}

TEST(MutableMeshViewDeathTest, InsertTriangleIndicesOutOfBounds) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 4);
  MutableMeshView mesh_view(mesh);
  ASSERT_TRUE(mesh_view.HasMeshData());
  ASSERT_EQ(mesh_view.TriangleCount(), 4);
  EXPECT_DEATH_IF_SUPPORTED(mesh_view.InsertTriangleIndices(5, {}), "");
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
