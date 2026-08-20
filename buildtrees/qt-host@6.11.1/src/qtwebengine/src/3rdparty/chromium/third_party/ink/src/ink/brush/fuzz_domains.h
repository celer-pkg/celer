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

#ifndef INK_STROKES_BRUSH_FUZZ_DOMAINS_H_
#define INK_STROKES_BRUSH_FUZZ_DOMAINS_H_

#include "fuzztest/fuzztest.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"

namespace ink {

// The domain of all valid brushes.
fuzztest::Domain<Brush> ValidBrush();
// The domain of all valid brushes that can be serialized to proto.
fuzztest::Domain<Brush> SerializableBrush();

// The domain of all valid brush behaviors.
fuzztest::Domain<BrushBehavior> ValidBrushBehavior();
// The domain of all valid brush behaviors that can be serialized to proto.
fuzztest::Domain<BrushBehavior> SerializableBrushBehavior();

// The domain of all valid brush behavior nodes.
fuzztest::Domain<BrushBehavior::Node> ValidBrushBehaviorNode();
// The domain of all valid brush behavior nodes that can be serialized to proto.
fuzztest::Domain<BrushBehavior::Node> SerializableBrushBehaviorNode();

// The domain of all valid brush coats.
fuzztest::Domain<BrushCoat> ValidBrushCoat();
// The domain of all valid brush coats that can be serialized to proto.
fuzztest::Domain<BrushCoat> SerializableBrushCoat();

// The domain of all valid brush families.
fuzztest::Domain<BrushFamily> ValidBrushFamily();
// The domain of all valid brush families that can be serialized to proto.
fuzztest::Domain<BrushFamily> SerializableBrushFamily();

// The domain of all valid brush family input models.
fuzztest::Domain<BrushFamily::InputModel> ValidBrushFamilyInputModel();

// The domain of all valid brush paints.
fuzztest::Domain<BrushPaint> ValidBrushPaint();
// The domain of all valid brush paints that can be serialized to proto.
fuzztest::Domain<BrushPaint> SerializableBrushPaint();

// The domain of all valid brush tips.
fuzztest::Domain<BrushTip> ValidBrushTip();
// The domain of all valid brush tips that can be serialized to proto.
fuzztest::Domain<BrushTip> SerializableBrushTip();

// The domain of all valid easing functions.
fuzztest::Domain<EasingFunction> ValidEasingFunction();

}  // namespace ink

#endif  // INK_STROKES_BRUSH_FUZZ_DOMAINS_H_
