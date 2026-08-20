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

#include "ink/geometry/internal/static_rtree.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include "absl/algorithm/container.h"
#include "absl/container/inlined_vector.h"
#include "absl/types/span.h"

namespace ink::geometry_internal {

absl::InlinedVector<uint32_t, kMaxExpectedRTreeBranchDepth>
ComputeNumberOfRTreeBranchNodesAtDepth(uint32_t n_leaf_nodes,
                                       uint32_t branching_factor) {
  // If there are few enough leaf nodes that they can fit in one branch, then we
  // only need the root.
  if (n_leaf_nodes <= branching_factor) {
    return {1u};
  }

  absl::InlinedVector<uint32_t, kMaxExpectedRTreeBranchDepth>
      n_branch_nodes_at_depth;
  uint32_t n_at_current_level = n_leaf_nodes;
  while (n_at_current_level > 1) {
    n_at_current_level =
        std::ceil(static_cast<double>(n_at_current_level) / branching_factor);
    n_branch_nodes_at_depth.push_back(n_at_current_level);
  }
  absl::c_reverse(n_branch_nodes_at_depth);
  return n_branch_nodes_at_depth;
}

absl::InlinedVector<uint32_t, kMaxExpectedRTreeBranchDepth>
ComputeRTreeBranchDepthOffsets(
    absl::Span<const uint32_t> n_branch_nodes_at_depth) {
  absl::InlinedVector<uint32_t, kMaxExpectedRTreeBranchDepth> offsets(
      n_branch_nodes_at_depth.size(), 0);
  for (size_t i = 1; i < offsets.size(); ++i) {
    offsets[i] = offsets[i - 1] + n_branch_nodes_at_depth[i - 1];
  }
  return offsets;
}

}  // namespace ink::geometry_internal
