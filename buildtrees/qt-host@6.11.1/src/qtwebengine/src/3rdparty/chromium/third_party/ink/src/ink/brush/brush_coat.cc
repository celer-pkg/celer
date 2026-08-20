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

#include "ink/brush/brush_coat.h"

#include <string>
#include <variant>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/geometry/mesh_format.h"

namespace ink {
namespace brush_internal {
namespace {

bool BrushTipUsesColorShift(const BrushTip& tip) {
  for (const BrushBehavior& behavior : tip.behaviors) {
    for (const BrushBehavior::Node& node : behavior.nodes) {
      if (const auto* output = std::get_if<BrushBehavior::TargetNode>(&node)) {
        switch (output->target) {
          case BrushBehavior::Target::kHueOffsetInRadians:
          case BrushBehavior::Target::kSaturationMultiplier:
          case BrushBehavior::Target::kLuminosity:
            return true;
          default:
            break;
        }
      }
    }
  }
  return false;
}

}  // namespace

absl::Status ValidateBrushCoat(const BrushCoat& coat) {
  if (absl::Status status = ValidateBrushTip(coat.tip); !status.ok()) {
    return status;
  }
  if (absl::Status status = ValidateBrushPaint(coat.paint); !status.ok()) {
    return status;
  }
  return absl::OkStatus();
}

std::vector<MeshFormat::AttributeId> GetRequiredAttributeIds(
    const BrushCoat& coat) {
  std::vector<MeshFormat::AttributeId> ids = {
      // All meshes must have a kPosition attribute.
      MeshFormat::AttributeId::kPosition,
      // The side/forward attributes are always required, in order to support
      // shader-based anti-aliasing.
      MeshFormat::AttributeId::kSideDerivative,
      MeshFormat::AttributeId::kSideLabel,
      MeshFormat::AttributeId::kForwardDerivative,
      MeshFormat::AttributeId::kForwardLabel,
      // Opacity shift is always required (even when there's no `kOpacityShift`
      // brush behavior), in order to support overlap behavior for translucent
      // colors.
      MeshFormat::AttributeId::kOpacityShift,
  };

  if (BrushTipUsesColorShift(coat.tip)) {
    ids.push_back(MeshFormat::AttributeId::kColorShiftHsl);
  }

  for (const BrushPaint::TextureLayer& layer : coat.paint.texture_layers) {
    if (layer.mapping == BrushPaint::TextureMapping::kWinding) {
      ids.push_back(MeshFormat::AttributeId::kSurfaceUv);
      break;
    }
  }

  return ids;
}

std::string ToFormattedString(const BrushCoat& coat) {
  return absl::StrFormat("BrushCoat{tip=%v, paint=%v}", coat.tip, coat.paint);
}

}  // namespace brush_internal
}  // namespace ink
