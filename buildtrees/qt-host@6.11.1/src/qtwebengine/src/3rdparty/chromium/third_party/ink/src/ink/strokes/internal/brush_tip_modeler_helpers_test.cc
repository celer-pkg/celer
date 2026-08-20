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

#include "ink/strokes/internal/brush_tip_modeler_helpers.h"

#include <cmath>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/types/span.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/easing_implementation.h"
#include "ink/strokes/internal/noise_generator.h"
#include "ink/strokes/internal/stroke_input_modeler.h"
#include "ink/strokes/internal/type_matchers.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"
#include "ink/types/type_matchers.h"

namespace ink::strokes_internal {
namespace {

using ::testing::ElementsAre;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::IsEmpty;

MATCHER(NullNodeValueMatcher, "") { return IsNullBehaviorNodeValue(arg); }

class ProcessBehaviorNodeTest : public ::testing::Test {
 protected:
  StrokeInputModeler::State input_modeler_state_;
  ModeledStrokeInput current_input_;
  std::vector<float> stack_;
  BehaviorNodeContext context_ = {
      .input_modeler_state = input_modeler_state_,
      .current_input = current_input_,
      .brush_size = 1,
      .stack = stack_,
  };
};

TEST_F(ProcessBehaviorNodeTest, SourceNodeNormalizedPressure) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kNormalizedPressure,
      .source_value_range = {0, 1},
  };

  current_input_.pressure = 0.75f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  // If pressure data is missing, the source node emits a null value.
  stack_.clear();
  current_input_.pressure = StrokeInput::kNoPressure;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeTiltInRadians) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kTiltInRadians,
      .source_value_range = {0, Angle::Degrees(90).ValueInRadians()},
  };

  current_input_.tilt = Angle::Degrees(30);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(1.0f / 3.0f, 1e-5)));

  // If tilt data is missing, the source node emits a null value.
  stack_.clear();
  current_input_.tilt = StrokeInput::kNoTilt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeTiltXInRadians) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kTiltXInRadians,
      .source_value_range = {Angle::Degrees(-90).ValueInRadians(),
                             Angle::Degrees(90).ValueInRadians()},
  };

  current_input_.tilt = Angle::Degrees(30);
  current_input_.orientation = Angle::Degrees(60);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.589f, 0.001f)));

  // If tilt or orientation data is missing, the source node emits a null value.
  stack_.clear();
  current_input_.tilt = StrokeInput::kNoTilt;
  current_input_.orientation = Angle::Degrees(45);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));

  stack_.clear();
  current_input_.tilt = Angle::Degrees(45);
  current_input_.orientation = StrokeInput::kNoOrientation;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeTiltYInRadians) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kTiltYInRadians,
      .source_value_range = {Angle::Degrees(-90).ValueInRadians(),
                             Angle::Degrees(90).ValueInRadians()},
  };

  current_input_.tilt = Angle::Degrees(30);
  current_input_.orientation = Angle::Degrees(60);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.648f, 0.001f)));

  // If tilt or orientation data is missing, the source node emits a null value.
  stack_.clear();
  current_input_.tilt = StrokeInput::kNoTilt;
  current_input_.orientation = Angle::Degrees(45);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));

  stack_.clear();
  current_input_.tilt = Angle::Degrees(45);
  current_input_.orientation = StrokeInput::kNoOrientation;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeOrientationInRadians) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kOrientationInRadians,
      .source_value_range = {0, Angle::Degrees(360).ValueInRadians()},
  };

  current_input_.tilt = StrokeInput::kNoTilt;
  current_input_.orientation = Angle::Degrees(270);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.75f, 1e-5)));

  // If orientation data is missing, the source node emits a null value.
  stack_.clear();
  current_input_.orientation = StrokeInput::kNoOrientation;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));

  // If tilt is zero (vertical), then the stylus orientation is undefined (even
  // if orientation data is present), so the source node emits a null value.
  stack_.clear();
  current_input_.tilt = Angle::Degrees(0);
  current_input_.orientation = Angle::Degrees(270);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeOrientationAboutZeroInRadians) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kOrientationAboutZeroInRadians,
      .source_value_range = {Angle::Degrees(-180).ValueInRadians(),
                             Angle::Degrees(180).ValueInRadians()},
  };

  current_input_.tilt = StrokeInput::kNoTilt;
  current_input_.orientation = Angle::Degrees(270);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));

  // If orientation data is missing, the source node emits a null value.
  stack_.clear();
  current_input_.orientation = StrokeInput::kNoOrientation;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));

  // If tilt is zero (vertical), then the stylus orientation is undefined (even
  // if orientation data is present), so the source node emits a null value.
  stack_.clear();
  current_input_.tilt = Angle::Degrees(0);
  current_input_.orientation = Angle::Degrees(270);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeSpeedInMultiplesOfBrushSizePerSecond) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond,
      .source_value_range = {0, 1},
  };

  context_.brush_size = 10.0f;
  current_input_.velocity = {-3.0f, 4.0f};  // speed is 5 stroke units/s
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.5f, 1e-5)));

  stack_.clear();
  context_.brush_size = 20.0f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeVelocityXInMultiplesOfBrushSizePerSecond) {
  BrushBehavior::SourceNode source_node = {
      .source =
          BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond,
      .source_value_range = {-1, 1},
  };

  context_.brush_size = 10.0f;
  current_input_.velocity = {6.0f, 4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.8f, 1e-5)));

  stack_.clear();
  current_input_.velocity = {-6.0f, 4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.2f, 1e-5)));

  stack_.clear();
  context_.brush_size = 20.0f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.35f, 1e-5)));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeVelocityYInMultiplesOfBrushSizePerSecond) {
  BrushBehavior::SourceNode source_node = {
      .source =
          BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond,
      .source_value_range = {-1, 1},
  };

  context_.brush_size = 10.0f;
  current_input_.velocity = {6.0f, 4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.7f, 1e-5)));

  stack_.clear();
  current_input_.velocity = {6.0f, -4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.3f, 1e-5)));

  stack_.clear();
  context_.brush_size = 20.0f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.4f, 1e-5)));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeDirectionInRadians) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kDirectionInRadians,
      .source_value_range = {0, Angle::Degrees(360).ValueInRadians()},
  };

  context_.current_travel_direction = Angle::Degrees(-60);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(5.0f / 6.0f, 1e-5)));

  // If the direction is undefined, the source node emits a null value.
  stack_.clear();
  context_.current_travel_direction = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeDirectionAboutZeroInRadians) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kDirectionAboutZeroInRadians,
      .source_value_range = {Angle::Degrees(-180).ValueInRadians(),
                             Angle::Degrees(180).ValueInRadians()},
  };

  context_.current_travel_direction = Angle::Degrees(-60);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(1.0f / 3.0f, 1e-5)));

  // If the direction is undefined, the source node emits a null value.
  stack_.clear();
  context_.current_travel_direction = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeNormalizedDirectionX) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kNormalizedDirectionX,
      .source_value_range = {-1, 1},
  };

  context_.current_travel_direction = Angle::Degrees(-60);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.75f, 1e-5)));

  // If the direction is undefined, the source node emits a null value.
  stack_.clear();
  context_.current_travel_direction = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeNormalizedDirectionY) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kNormalizedDirectionY,
      .source_value_range = {-1, 1},
  };

  context_.current_travel_direction = Angle::Degrees(-150);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));

  // If the direction is undefined, the source node emits a null value.
  stack_.clear();
  context_.current_travel_direction = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeDistanceTraveledInMultiplesOfBrushSize) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize,
      .source_value_range = {0, 10},
  };
  context_.brush_size = 3;
  current_input_.traveled_distance = 15;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.5f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeTimeOfInputInSeconds) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kTimeOfInputInSeconds,
      .source_value_range = {0, 10},
  };
  current_input_.elapsed_time = Duration32::Seconds(7.5);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeTimeOfInputInMillis) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kTimeOfInputInMillis,
      .source_value_range = {0, 10000},
  };
  current_input_.elapsed_time = Duration32::Seconds(7.5);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodePredictedDistanceTraveledInMultiplesOfBrushSize) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kPredictedDistanceTraveledInMultiplesOfBrushSize,
      .source_value_range = {0, 10},
  };
  context_.brush_size = 3;
  current_input_.traveled_distance = 15;
  input_modeler_state_.total_real_distance = 9;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.2f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodePredictedTimeElapsedInSeconds) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kPredictedTimeElapsedInSeconds,
      .source_value_range = {0, 10},
  };
  current_input_.elapsed_time = Duration32::Seconds(15);
  input_modeler_state_.total_real_elapsed_time = Duration32::Seconds(9);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.6f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodePredictedTimeElapsedInMillis) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kPredictedTimeElapsedInMillis,
      .source_value_range = {0, 10000},
  };
  current_input_.elapsed_time = Duration32::Seconds(15);
  input_modeler_state_.total_real_elapsed_time = Duration32::Seconds(9);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.6f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeDistanceRemainingInMultiplesOfBrushSize) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize,
      .source_value_range = {0, 10},
  };
  context_.brush_size = 3;
  current_input_.traveled_distance = 9;
  input_modeler_state_.complete_traveled_distance = 15;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.2f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeTimeSinceInputInSeconds) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kTimeSinceInputInSeconds,
      .source_value_range = {0, 10},
  };
  current_input_.elapsed_time = Duration32::Seconds(3);
  input_modeler_state_.complete_elapsed_time = Duration32::Seconds(5);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.2f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeTimeSinceInputInMillis) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kTimeSinceInputInMillis,
      .source_value_range = {0, 10000},
  };
  current_input_.elapsed_time = Duration32::Seconds(3);
  input_modeler_state_.complete_elapsed_time = Duration32::Seconds(5);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.2f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeAccelerationInMultiplesOfBrushSizePerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kAccelerationInMultiplesOfBrushSizePerSecondSquared,
      .source_value_range = {0, 100},
  };
  context_.brush_size = 10;
  current_input_.acceleration = {300, -400};  // accel is 500 stroke units/s²
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.5f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeAccelerationXInMultiplesOfBrushSizePerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kAccelerationXInMultiplesOfBrushSizePerSecondSquared,
      .source_value_range = {0, 100},
  };
  context_.brush_size = 10;
  current_input_.acceleration = {300, -400};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.3f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeAccelerationYInMultiplesOfBrushSizePerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kAccelerationYInMultiplesOfBrushSizePerSecondSquared,
      .source_value_range = {0, -100},
  };
  context_.brush_size = 10;
  current_input_.acceleration = {300, -400};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.4f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeAccelerationForwardInMultiplesOfBrushSizePerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared,
      .source_value_range = {0, 100},
  };

  context_.brush_size = 10;
  current_input_.velocity = {8, 8};
  current_input_.acceleration = {500, 0};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f * std::sqrtf(2), 1e-5)));

  stack_.clear();
  current_input_.velocity = {8, 0};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.5f));

  stack_.clear();
  current_input_.velocity = {0, 8};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.0f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeAccelerationLateralInMultiplesOfBrushSizePerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared,
      .source_value_range = {-100, 100},
  };

  context_.brush_size = 10;
  current_input_.velocity = {8, 0};
  current_input_.acceleration = {0, 500};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  stack_.clear();
  current_input_.velocity = {-8, 0};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.25f));

  stack_.clear();
  current_input_.velocity = {0, 8};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.5f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeInputSpeedInCentimetersPerSecond) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kInputSpeedInCentimetersPerSecond,
      .source_value_range = {0, 1},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.velocity = {-3.0f, 4.0f};  // speed is 5 stroke units/s
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.5f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeInputVelocityXInCentimetersPerSecond) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond,
      .source_value_range = {-1, 1},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.velocity = {6.0f, 4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.8f, 1e-5)));

  stack_.clear();
  current_input_.velocity = {-6.0f, 4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.2f, 1e-5)));

  stack_.clear();
  input_modeler_state_.stroke_unit_length =
      PhysicalDistance::Centimeters(0.05f);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.35f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeInputVelocityYInCentimetersPerSecond) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond,
      .source_value_range = {-1, 1},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.velocity = {6.0f, 4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.7f, 1e-5)));

  stack_.clear();
  current_input_.velocity = {6.0f, -4.0f};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.3f, 1e-5)));

  stack_.clear();
  input_modeler_state_.stroke_unit_length =
      PhysicalDistance::Centimeters(0.05f);
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.4f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeInputDistanceTraveledInCentimeters) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kInputDistanceTraveledInCentimeters,
      .source_value_range = {0, 10},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.traveled_distance = 40;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.4f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodePredictedInputDistanceTraveledInCentimeters) {
  BrushBehavior::SourceNode source_node = {
      .source =
          BrushBehavior::Source::kPredictedInputDistanceTraveledInCentimeters,
      .source_value_range = {0, 1},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.traveled_distance = 50;
  input_modeler_state_.total_real_distance = 46;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.4f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeInputAccelerationInCentimetersPerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kInputAccelerationInCentimetersPerSecondSquared,
      .source_value_range = {0, 100},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.acceleration = {300, -400};  // accel is 500 stroke units/s²
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.5f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeInputAccelerationXInCentimetersPerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kInputAccelerationXInCentimetersPerSecondSquared,
      .source_value_range = {0, 100},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.acceleration = {300, -400};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.3f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeInputAccelerationYInCentimetersPerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kInputAccelerationYInCentimetersPerSecondSquared,
      .source_value_range = {0, -100},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.acceleration = {300, -400};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.4f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeInputAccelerationForwardInCentimetersPerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kInputAccelerationForwardInCentimetersPerSecondSquared,
      .source_value_range = {0, 100},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.velocity = {8, 8};
  current_input_.acceleration = {500, 0};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f * std::sqrtf(2), 1e-5)));

  stack_.clear();
  current_input_.velocity = {8, 0};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.5f, 1e-5)));

  stack_.clear();
  current_input_.velocity = {0, 8};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.0f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeInputAccelerationLateralInCentimetersPerSecondSquared) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::
          kInputAccelerationLateralInCentimetersPerSecondSquared,
      .source_value_range = {-100, 100},
  };

  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1f);
  current_input_.velocity = {8, 0};
  current_input_.acceleration = {0, 500};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.75f, 1e-5)));

  stack_.clear();
  current_input_.velocity = {-8, 0};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));

  stack_.clear();
  current_input_.velocity = {0, 8};
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.5f, 1e-5)));

  // If `stroke_unit_length` is indeterminate, the source node emits a null
  // value.
  stack_.clear();
  input_modeler_state_.stroke_unit_length = std::nullopt;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeDistanceRemainingAsFractionOfStrokeLength) {
  BrushBehavior::SourceNode source_node = {
      .source =
          BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength,
      .source_value_range = {0, 1},
  };

  current_input_.traveled_distance = 3.0f;
  input_modeler_state_.complete_traveled_distance = 12.0f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));
}

TEST_F(ProcessBehaviorNodeTest,
       SourceNodeDistanceRemainingAsFractionOfStrokeLengthZeroLength) {
  BrushBehavior::SourceNode source_node = {
      .source =
          BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength,
      .source_value_range = {0, 1},
  };

  // If there is zero distance remaining out of total stroke length of zero,
  // then the fraction of distance remaining isn't well-defined (0/0), so we
  // arbitrarily define that as 0% distance remaining.
  current_input_.traveled_distance = 0.0f;
  input_modeler_state_.complete_traveled_distance = 0.0f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.0f));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeOutOfRangeClamp) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kNormalizedPressure,
      .source_out_of_range_behavior = BrushBehavior::OutOfRange::kClamp,
      .source_value_range = {0.2, 0.6},
  };

  current_input_.pressure = 0.3f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));

  stack_.clear();
  current_input_.pressure = 0.1f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.0f, 1e-5)));

  stack_.clear();
  current_input_.pressure = 0.7f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(1.0f, 1e-5)));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeOutOfRangeRepeat) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kNormalizedPressure,
      .source_out_of_range_behavior = BrushBehavior::OutOfRange::kRepeat,
      .source_value_range = {0.2, 0.6},
  };

  current_input_.pressure = 0.3f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));

  stack_.clear();
  current_input_.pressure = 0.1f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.75f, 1e-5)));

  stack_.clear();
  current_input_.pressure = 0.7f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));
}

TEST_F(ProcessBehaviorNodeTest, SourceNodeOutOfRangeMirror) {
  BrushBehavior::SourceNode source_node = {
      .source = BrushBehavior::Source::kNormalizedPressure,
      .source_out_of_range_behavior = BrushBehavior::OutOfRange::kMirror,
      .source_value_range = {0.2, 0.6},
  };

  current_input_.pressure = 0.3f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));

  stack_.clear();
  current_input_.pressure = 0.1f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.25f, 1e-5)));

  stack_.clear();
  current_input_.pressure = 0.7f;
  ProcessBehaviorNode(source_node, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.75f, 1e-5)));
}

TEST_F(ProcessBehaviorNodeTest, ConstantNode) {
  ProcessBehaviorNode(BrushBehavior::ConstantNode{.value = 0.75f}, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));
  ProcessBehaviorNode(BrushBehavior::ConstantNode{.value = -0.5f}, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f, -0.5f));
}

TEST_F(ProcessBehaviorNodeTest, NoiseNodeDistanceInCentimeters) {
  NoiseGenerator reference_generator(12345);
  // Set the node up to use a copy of the reference generator.
  std::vector<NoiseGenerator> generators = {reference_generator};
  context_.noise_generators = absl::MakeSpan(generators);
  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1);
  context_.previous_input_metrics = InputMetrics{.traveled_distance = 0};
  NoiseNodeImplementation noise_impl = {
      .generator_index = 0,
      .vary_over = BrushBehavior::DampingSource::kDistanceInCentimeters,
      .base_period = 3.0,
  };

  // The noise node has a base period of 3cm, and we've set the length of one
  // stroke unit to 0.1cm above, so the base period is 30 stroke units.  After a
  // traveled distance equal to 75% of the base period (that is, 22.5 stroke
  // units), we should get the same random output as the reference generator
  // gives for an input of 0.75.
  current_input_.traveled_distance = 22.5f;
  ProcessBehaviorNode(noise_impl, context_);
  reference_generator.AdvanceInputBy(0.75f);
  EXPECT_THAT(stack_,
              ElementsAre(FloatEq(reference_generator.CurrentOutputValue())));
}

TEST_F(ProcessBehaviorNodeTest,
       NoiseNodeDistanceInCentimetersWithNoStrokeUnitLength) {
  NoiseGenerator reference_generator(23456);
  // Set the node up to use a copy of the reference generator.
  std::vector<NoiseGenerator> generators = {reference_generator};
  context_.noise_generators = absl::MakeSpan(generators);
  input_modeler_state_.stroke_unit_length = std::nullopt;
  context_.previous_input_metrics = InputMetrics{.traveled_distance = 0};
  NoiseNodeImplementation noise_impl = {
      .generator_index = 0,
      .vary_over = BrushBehavior::DampingSource::kDistanceInCentimeters,
      .base_period = 3.0,
  };

  // Since there's no mapping set between stroke units and physical units, the
  // noise node should treat any stroke distance traveled as if no physical
  // distance had been traveled. Thus, we should get the same random output as
  // the reference generator gives for an input of 0.
  current_input_.traveled_distance = 22.5f;
  ProcessBehaviorNode(noise_impl, context_);
  reference_generator.AdvanceInputBy(0.f);
  EXPECT_THAT(stack_,
              ElementsAre(FloatEq(reference_generator.CurrentOutputValue())));
}

TEST_F(ProcessBehaviorNodeTest, NoiseNodeDistanceInMultiplesOfBrushSize) {
  NoiseGenerator reference_generator(34567);
  // Set the node up to use a copy of the reference generator.
  std::vector<NoiseGenerator> generators = {reference_generator};
  context_.noise_generators = absl::MakeSpan(generators);
  context_.brush_size = 10;
  context_.previous_input_metrics = InputMetrics{.traveled_distance = 0};
  NoiseNodeImplementation noise_impl = {
      .generator_index = 0,
      .vary_over =
          BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize,
      .base_period = 3.0,
  };

  // After a traveled distance equal to 75% of the base period, we should get
  // the same random output as the reference generator gives for an input of
  // 0.75.
  current_input_.traveled_distance = 22.5f;
  ProcessBehaviorNode(noise_impl, context_);
  reference_generator.AdvanceInputBy(0.75f);
  EXPECT_THAT(stack_,
              ElementsAre(FloatEq(reference_generator.CurrentOutputValue())));
}

TEST_F(ProcessBehaviorNodeTest, NoiseNodeTimeInSeconds) {
  NoiseGenerator reference_generator(45678);
  // Set the node up to use a copy of the reference generator.
  std::vector<NoiseGenerator> generators = {reference_generator};
  context_.noise_generators = absl::MakeSpan(generators);
  context_.previous_input_metrics = InputMetrics{
      .elapsed_time = Duration32::Zero(),
  };
  NoiseNodeImplementation noise_impl = {
      .generator_index = 0,
      .vary_over = BrushBehavior::DampingSource::kTimeInSeconds,
      .base_period = 3.0,
  };

  // After an elapsed time equal to 75% of the base period, we should get the
  // same random output as the reference generator gives for an input of 0.75.
  current_input_.elapsed_time = Duration32::Seconds(2.25f);
  ProcessBehaviorNode(noise_impl, context_);
  reference_generator.AdvanceInputBy(0.75f);
  EXPECT_THAT(stack_,
              ElementsAre(FloatEq(reference_generator.CurrentOutputValue())));
}

TEST_F(ProcessBehaviorNodeTest, FallbackFilterNodePressure) {
  BrushBehavior::FallbackFilterNode filter_node = {
      .is_fallback_for = BrushBehavior::OptionalInputProperty::kPressure};

  // The stack is left unchanged if the input lacks the fallback-for property.
  current_input_.pressure = StrokeInput::kNoPressure;
  stack_.push_back(0.75f);
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  // The top of the stack is set to null if the input includes the fallback-for
  // property.
  current_input_.pressure = 0.5f;
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, FallbackFilterNodeTilt) {
  BrushBehavior::FallbackFilterNode filter_node = {
      .is_fallback_for = BrushBehavior::OptionalInputProperty::kTilt};

  // The stack is left unchanged if the input lacks the fallback-for property.
  current_input_.tilt = StrokeInput::kNoTilt;
  stack_.push_back(0.75f);
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  // The top of the stack is set to null if the input includes the fallback-for
  // property.
  current_input_.tilt = Angle::Degrees(30);
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, FallbackFilterNodeOrientation) {
  BrushBehavior::FallbackFilterNode filter_node = {
      .is_fallback_for = BrushBehavior::OptionalInputProperty::kOrientation};

  // The stack is left unchanged if the input lacks the fallback-for property.
  current_input_.orientation = StrokeInput::kNoOrientation;
  stack_.push_back(0.75f);
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  // The top of the stack is set to null if the input includes the fallback-for
  // property.
  current_input_.orientation = Angle::Degrees(120);
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, FallbackFilterNodeTiltXAndY) {
  BrushBehavior::FallbackFilterNode filter_node = {
      .is_fallback_for = BrushBehavior::OptionalInputProperty::kTiltXAndY};

  // For kTiltXAndY, both tilt and orientation data must be present. So the
  // stack is left unchanged if the input lacks either property.
  stack_.push_back(0.75f);
  current_input_.tilt = StrokeInput::kNoTilt;
  current_input_.orientation = Angle::Degrees(45);
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  current_input_.tilt = Angle::Degrees(45);
  current_input_.orientation = StrokeInput::kNoOrientation;
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  // The top of the stack is set to null if the input includes both properties.
  current_input_.tilt = Angle::Degrees(45);
  current_input_.orientation = Angle::Degrees(45);
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, ToolTypeFilterNode) {
  BrushBehavior::ToolTypeFilterNode filter_node = {
      .enabled_tool_types = {.unknown = true, .mouse = true, .stylus = true}};

  // The stack is left unchanged if the stroke's tool type is enabled in the
  // node.
  stack_.push_back(0.75f);
  input_modeler_state_.tool_type = StrokeInput::ToolType::kUnknown;
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  input_modeler_state_.tool_type = StrokeInput::ToolType::kMouse;
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  input_modeler_state_.tool_type = StrokeInput::ToolType::kStylus;
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));

  // The top of the stack is set to null if the stroke's tool type is disabled
  // in the node.
  input_modeler_state_.tool_type = StrokeInput::ToolType::kTouch;
  ProcessBehaviorNode(filter_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, DampingNodeDistanceInCentimeters) {
  std::vector<float> damped_values = {kNullBehaviorNodeValue};
  input_modeler_state_.stroke_unit_length = PhysicalDistance::Centimeters(0.1);
  context_.damped_values = absl::MakeSpan(damped_values);
  context_.previous_input_metrics = InputMetrics{.traveled_distance = 0};
  DampingNodeImplementation damping_impl = {
      .damping_index = 0,
      .damping_source = BrushBehavior::DampingSource::kDistanceInCentimeters,
      .damping_gap = 5.0f,
  };

  // The damped value remains null as long as the input remains null.
  current_input_.traveled_distance = 25;
  stack_.push_back(kNullBehaviorNodeValue);
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
  EXPECT_THAT(damped_values, ElementsAre(NullNodeValueMatcher()));
  context_.previous_input_metrics->traveled_distance =
      current_input_.traveled_distance;

  // The first non-null input snaps the damped value to that input value.
  current_input_.traveled_distance += 25;
  stack_[0] = 0.5f;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(0.5f));
  EXPECT_THAT(damped_values, ElementsAre(0.5f));
  context_.previous_input_metrics->traveled_distance =
      current_input_.traveled_distance;

  // If the input changes, it takes some distance for the damped value to
  // approach the new input.
  current_input_.traveled_distance += 25;
  stack_[0] = 0.0f;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.303f, 0.001f)));
  EXPECT_THAT(damped_values, ElementsAre(FloatNear(0.303f, 0.001f)));
  context_.previous_input_metrics->traveled_distance =
      current_input_.traveled_distance;

  // If the input becomes null again, the damped value remains at its previous
  // level.
  current_input_.traveled_distance += 25;
  stack_[0] = kNullBehaviorNodeValue;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.303f, 0.001f)));
  EXPECT_THAT(damped_values, ElementsAre(FloatNear(0.303f, 0.001f)));
}

TEST_F(ProcessBehaviorNodeTest,
       DampingNodeDistanceInCentimetersWithNoStrokeUnitLength) {
  std::vector<float> damped_values = {0.5f};
  input_modeler_state_.stroke_unit_length = std::nullopt;
  context_.damped_values = absl::MakeSpan(damped_values);
  context_.previous_input_metrics = InputMetrics{.traveled_distance = 0};
  DampingNodeImplementation damping_impl = {
      .damping_index = 0,
      .damping_source = BrushBehavior::DampingSource::kDistanceInCentimeters,
      .damping_gap = 5.0f,
  };

  // If the input changes, but no stroke_unit_length is set, the damped value
  // snaps to the new input.
  current_input_.traveled_distance = 25;
  stack_.push_back(0.75f);
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(0.75f));
  EXPECT_THAT(damped_values, ElementsAre(0.75f));
}

TEST_F(ProcessBehaviorNodeTest, DampingNodeDistanceInMultiplesOfBrushSize) {
  std::vector<float> damped_values = {kNullBehaviorNodeValue};
  context_.brush_size = 10;
  context_.damped_values = absl::MakeSpan(damped_values);
  context_.previous_input_metrics = InputMetrics{.traveled_distance = 0};
  DampingNodeImplementation damping_impl = {
      .damping_index = 0,
      .damping_source =
          BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize,
      .damping_gap = 5.0f,
  };

  // The damped value remains null as long as the input remains null.
  current_input_.traveled_distance = 25;
  stack_.push_back(kNullBehaviorNodeValue);
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
  EXPECT_THAT(damped_values, ElementsAre(NullNodeValueMatcher()));
  context_.previous_input_metrics->traveled_distance =
      current_input_.traveled_distance;

  // The first non-null input snaps the damped value to that input value.
  current_input_.traveled_distance += 25;
  stack_[0] = 0.5f;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(0.5f));
  EXPECT_THAT(damped_values, ElementsAre(0.5f));
  context_.previous_input_metrics->traveled_distance =
      current_input_.traveled_distance;

  // If the input changes, it takes some distance for the damped value to
  // approach the new input.
  current_input_.traveled_distance += 25;
  stack_[0] = 0.0f;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.303f, 0.001f)));
  EXPECT_THAT(damped_values, ElementsAre(FloatNear(0.303f, 0.001f)));
  context_.previous_input_metrics->traveled_distance =
      current_input_.traveled_distance;

  // If the input becomes null again, the damped value remains at its previous
  // level.
  current_input_.traveled_distance += 25;
  stack_[0] = kNullBehaviorNodeValue;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.303f, 0.001f)));
  EXPECT_THAT(damped_values, ElementsAre(FloatNear(0.303f, 0.001f)));
}

TEST_F(ProcessBehaviorNodeTest, DampingNodeTimeInSeconds) {
  std::vector<float> damped_values = {kNullBehaviorNodeValue};
  context_.damped_values = absl::MakeSpan(damped_values);
  context_.previous_input_metrics = InputMetrics{
      .elapsed_time = Duration32::Zero(),
  };
  DampingNodeImplementation damping_impl = {
      .damping_index = 0,
      .damping_source = BrushBehavior::DampingSource::kTimeInSeconds,
      .damping_gap = 0.5f,
  };

  // The damped value remains null as long as the input remains null.
  current_input_.elapsed_time = Duration32::Seconds(0.25);
  stack_.push_back(kNullBehaviorNodeValue);
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
  EXPECT_THAT(damped_values, ElementsAre(NullNodeValueMatcher()));
  context_.previous_input_metrics->elapsed_time = current_input_.elapsed_time;

  // The first non-null input snaps the damped value to that input value.
  current_input_.elapsed_time += Duration32::Seconds(0.25);
  stack_[0] = 0.5f;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(0.5f));
  EXPECT_THAT(damped_values, ElementsAre(0.5f));
  context_.previous_input_metrics->elapsed_time = current_input_.elapsed_time;

  // If the input changes, it takes time for the damped value to approach the
  // new input.
  current_input_.elapsed_time += Duration32::Seconds(0.25);
  stack_[0] = 0.0f;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.303f, 0.001f)));
  EXPECT_THAT(damped_values, ElementsAre(FloatNear(0.303f, 0.001f)));
  context_.previous_input_metrics->elapsed_time = current_input_.elapsed_time;

  // If the input becomes null again, the damped value remains at its previous
  // level.
  current_input_.elapsed_time += Duration32::Seconds(0.25);
  stack_[0] = kNullBehaviorNodeValue;
  ProcessBehaviorNode(damping_impl, context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.303f, 0.001f)));
  EXPECT_THAT(damped_values, ElementsAre(FloatNear(0.303f, 0.001f)));
}

TEST_F(ProcessBehaviorNodeTest, ResponseNode) {
  stack_.push_back(0.75f);
  ProcessBehaviorNode(
      EasingImplementation({EasingFunction::Predefined::kEaseInOut}), context_);
  EXPECT_THAT(stack_, ElementsAre(FloatNear(0.87f, 0.01f)));
}

TEST_F(ProcessBehaviorNodeTest, BinaryOpNodeSum) {
  BrushBehavior::BinaryOpNode binary_op_node = {
      .operation = BrushBehavior::BinaryOp::kSum};

  stack_.push_back(2.0f);
  stack_.push_back(3.0f);
  ProcessBehaviorNode(binary_op_node, context_);
  EXPECT_THAT(stack_, ElementsAre(5.0f));

  // `kSum` returns null when one of the two inputs is null.
  stack_.push_back(kNullBehaviorNodeValue);
  ProcessBehaviorNode(binary_op_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, BinaryOpNodeProduct) {
  BrushBehavior::BinaryOpNode binary_op_node = {
      .operation = BrushBehavior::BinaryOp::kProduct};

  stack_.push_back(2.0f);
  stack_.push_back(3.0f);
  ProcessBehaviorNode(binary_op_node, context_);
  EXPECT_THAT(stack_, ElementsAre(6.0f));

  // `kProduct` returns null when one of the two inputs is null.
  stack_.push_back(kNullBehaviorNodeValue);
  ProcessBehaviorNode(binary_op_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, InterpolationNodeLerp) {
  BrushBehavior::InterpolationNode interpolation_node = {
      .interpolation = BrushBehavior::Interpolation::kLerp,
  };

  stack_.push_back(0.25f);  // param
  stack_.push_back(2.0f);   // range start
  stack_.push_back(3.0f);   // range end
  ProcessBehaviorNode(interpolation_node, context_);
  EXPECT_THAT(stack_, ElementsAre(2.25f));

  // `kLerp` extrapolates when param is outside [0, 1].
  stack_.clear();
  stack_.push_back(1.25f);  // param
  stack_.push_back(2.0f);   // range start
  stack_.push_back(3.0f);   // range end
  ProcessBehaviorNode(interpolation_node, context_);
  EXPECT_THAT(stack_, ElementsAre(3.25f));
}

TEST_F(ProcessBehaviorNodeTest, InterpolationNodeInverseLerp) {
  BrushBehavior::InterpolationNode interpolation_node = {
      .interpolation = BrushBehavior::Interpolation::kInverseLerp,
  };

  stack_.push_back(2.25f);  // param
  stack_.push_back(2.0f);   // range start
  stack_.push_back(3.0f);   // range end
  ProcessBehaviorNode(interpolation_node, context_);
  EXPECT_THAT(stack_, ElementsAre(0.25f));

  // `kInverseLerp` inverse-extrapolates when param is outside [start, end].
  stack_.clear();
  stack_.push_back(1.75f);  // param
  stack_.push_back(3.0f);   // range start
  stack_.push_back(2.0f);   // range end
  ProcessBehaviorNode(interpolation_node, context_);
  EXPECT_THAT(stack_, ElementsAre(1.25f));

  // If the range endpoints are equal, `kInverseLerp` returns null (since the
  // result is undefined).
  stack_.clear();
  stack_.push_back(1.75f);  // param
  stack_.push_back(3.0f);   // range start
  stack_.push_back(3.0f);   // range end
  ProcessBehaviorNode(interpolation_node, context_);
  EXPECT_THAT(stack_, ElementsAre(NullNodeValueMatcher()));
}

TEST_F(ProcessBehaviorNodeTest, TargetNode) {
  std::vector<float> target_modifiers = {1.0f};
  context_.target_modifiers = absl::MakeSpan(target_modifiers);
  TargetNodeImplementation target_impl = {
      .target_index = 0,
      .target_modifier_range = {0.5f, 1.5f},
  };

  stack_.push_back(0.75f);
  ProcessBehaviorNode(target_impl, context_);
  EXPECT_THAT(stack_, IsEmpty());
  EXPECT_THAT(target_modifiers, ElementsAre(1.25f));

  // The target modifier will remain unchanged when the input value is null.
  stack_.push_back(kNullBehaviorNodeValue);
  ProcessBehaviorNode(target_impl, context_);
  EXPECT_THAT(stack_, IsEmpty());
  EXPECT_THAT(target_modifiers, ElementsAre(1.25f));
}

TEST_F(ProcessBehaviorNodeTest, PolarTargetNode) {
  std::vector<float> target_modifiers = {1.0f, 2.0f};
  context_.target_modifiers = absl::MakeSpan(target_modifiers);
  PolarTargetNodeImplementation polar_target_impl = {
      .target_x_index = 0,
      .target_y_index = 1,
      .angle_range = {0.0f, kHalfTurn.ValueInRadians()},
      .magnitude_range = {0.0f, 10.0f},
  };

  stack_.push_back(0.5f);   // angle input
  stack_.push_back(0.75f);  // magnitude input
  ProcessBehaviorNode(polar_target_impl, context_);
  EXPECT_THAT(stack_, IsEmpty());
  EXPECT_THAT(target_modifiers,
              ElementsAre(FloatNear(0.0f, 1e-5), FloatNear(7.5f, 1e-5)));

  // The target modifiers will remain unchanged when either input value is null.
  stack_.push_back(kNullBehaviorNodeValue);  // angle input
  stack_.push_back(0.25f);                   // magnitude input
  ProcessBehaviorNode(polar_target_impl, context_);
  EXPECT_THAT(stack_, IsEmpty());
  EXPECT_THAT(target_modifiers,
              ElementsAre(FloatNear(0.0f, 1e-5), FloatNear(7.5f, 1e-5)));

  // The target modifiers will remain unchanged when either input value is null.
  stack_.push_back(0.25f);                   // angle input
  stack_.push_back(kNullBehaviorNodeValue);  // magnitude input
  ProcessBehaviorNode(polar_target_impl, context_);
  EXPECT_THAT(stack_, IsEmpty());
  EXPECT_THAT(target_modifiers,
              ElementsAre(FloatNear(0.0f, 1e-5), FloatNear(7.5f, 1e-5)));
}

TEST(CreateTipStateTest, HasPassedInPosition) {
  EXPECT_THAT(CreateTipState({0, 0}, Angle(), BrushTip{}, 1.f, {}, {}).position,
              PointEq({0, 0}));
  EXPECT_THAT(
      CreateTipState({-1, 2}, Angle(), BrushTip{}, 1.f, {}, {}).position,
      PointEq({-1, 2}));
  EXPECT_THAT(CreateTipState({6, 8}, Angle(), BrushTip{}, 1.f, {}, {}).position,
              PointEq({6, 8}));
}

BrushTip MakeBaseBrushTip() {
  return {
      .scale = {2, 0.5},
      .corner_rounding = 0.25,
      .slant = -kFullTurn / 8,
      .pinch = 0.3,
      .rotation = -kQuarterTurn,
      .opacity_multiplier = 0.7f,
  };
}

TEST(CreateTipStateTest, HasBasePropertiesWithoutBehaviors) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 3.f;
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size, {}, {});

  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(state.slant, AngleEq(brush_tip.slant));
  EXPECT_FLOAT_EQ(state.pinch, brush_tip.pinch);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));
  EXPECT_FLOAT_EQ(state.opacity_multiplier, brush_tip.opacity_multiplier);
}

TEST(CreateTipStateTest, WithBehaviorTargetingWidth) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.f;
  float width_multiplier = 1.5f;
  BrushTipState state = CreateTipState(
      {0, 0}, Angle(), brush_tip, brush_size,
      {BrushBehavior::Target::kWidthMultiplier}, {width_multiplier});

  // Only the width should be affected by the multiplier:
  EXPECT_FLOAT_EQ(state.width,
                  width_multiplier * brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_multiplier = 5.f;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kWidthMultiplier},
                         {clamp_multiplier});
  EXPECT_FLOAT_EQ(state.width,
                  /**clamped to 2*/ 2 * brush_tip.scale.x * brush_size);
}

TEST(CreateTipStateTest, WithBehaviorTargetingHeight) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 3.f;
  float height_multiplier = 1.75f;
  BrushTipState state = CreateTipState(
      {0, 0}, Angle(), brush_tip, brush_size,
      {BrushBehavior::Target::kHeightMultiplier}, {height_multiplier});

  // Only the height should be affected by the multiplier:
  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height,
                  height_multiplier * brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_multiplier = -4.f;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kHeightMultiplier},
                         {clamp_multiplier});
  EXPECT_FLOAT_EQ(state.height, /**clamped to 0*/ 0);
}

TEST(CreateTipStateTest, WithBehaviorTargetingSize) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float size_multiplier = 1.3;
  BrushTipState state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                                       {BrushBehavior::Target::kSizeMultiplier},
                                       {size_multiplier});

  // Both width and height should be affected by the multiplier:
  EXPECT_FLOAT_EQ(state.width,
                  size_multiplier * brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height,
                  size_multiplier * brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_multiplier = 5.f;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kSizeMultiplier},
                         {clamp_multiplier});
  EXPECT_FLOAT_EQ(state.width,
                  /**clamped to 2*/ 2 * brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height,
                  /**clamped to 2*/ 2 * brush_tip.scale.y * brush_size);
}

TEST(CreateTipStateTest, WithBehaviorTargetingSlant) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float slant_offset_in_radians = 0.3;
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                     {BrushBehavior::Target::kSlantOffsetInRadians},
                     {slant_offset_in_radians});

  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(state.slant, AngleEq(brush_tip.slant +
                                   Angle::Radians(slant_offset_in_radians)));

  float clamp_offset_in_radians = kFullTurn.ValueInRadians();
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kSlantOffsetInRadians},
                         {clamp_offset_in_radians});

  EXPECT_THAT(state.slant, kQuarterTurn);
}

TEST(CreateTipStateTest, WithBehaviorTargetingPinch) {
  BrushTip brush_tip = MakeBaseBrushTip();
  brush_tip.behaviors = {};
  float brush_size = 2.5f;
  float pinch_offset = 0.3;
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                     {BrushBehavior::Target::kPinchOffset}, {pinch_offset});

  EXPECT_FLOAT_EQ(state.pinch, brush_tip.pinch + pinch_offset);
  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_offset = 5;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kPinchOffset}, {clamp_offset});
  EXPECT_FLOAT_EQ(state.pinch, 1);  // clamped to 1
}

TEST(CreateTipStateTest, WithBehaviorTargetingRotation) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float rotation_offset_in_radians = 0.3;
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                     {BrushBehavior::Target::kRotationOffsetInRadians},
                     {rotation_offset_in_radians});

  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(
      state.rotation,
      AngleEq((brush_tip.rotation + Angle::Radians(rotation_offset_in_radians))
                  .Normalized()));
}

TEST(CreateTipStateTest, WithBehaviorTargetingCornerRounding) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float rounding_offset = 0.3;
  BrushTipState state = CreateTipState(
      {0, 0}, Angle(), brush_tip, brush_size,
      {BrushBehavior::Target::kCornerRoundingOffset}, {rounding_offset});

  EXPECT_FLOAT_EQ(state.percent_radius,
                  brush_tip.corner_rounding + rounding_offset);
  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_offset = -5;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kCornerRoundingOffset},
                         {clamp_offset});
  EXPECT_FLOAT_EQ(state.percent_radius, /**clamped to 0*/ 0);
}

TEST(CreateTipStateTest, WithBehaviorTargetingTextureAnimationProgress) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float texture_animation_progress_offset = -3.25;
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                     {BrushBehavior::Target::kTextureAnimationProgressOffset},
                     {texture_animation_progress_offset});
  // The final progress offset should be computed mod 1.
  EXPECT_FLOAT_EQ(state.texture_animation_progress_offset, 0.75);
}

TEST(CreateTipStateTest, WithBehaviorTargetingHue) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float hue_offset_in_radians = 0.3f;
  BrushTipState state = CreateTipState(
      {0, 0}, Angle(), brush_tip, brush_size,
      {BrushBehavior::Target::kHueOffsetInRadians}, {hue_offset_in_radians});

  EXPECT_FLOAT_EQ(state.hue_offset_in_full_turns,
                  hue_offset_in_radians / kFullTurn.ValueInRadians());
  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  Angle normalize_offset = kFullTurn * 1.5f;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kHueOffsetInRadians},
                         {normalize_offset.ValueInRadians()});
  EXPECT_FLOAT_EQ(state.hue_offset_in_full_turns, 0.5);
}

TEST(CreateTipStateTest, WithBehaviorTargetingSaturation) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float saturation_multiplier = 1.3;
  BrushTipState state = CreateTipState(
      {0, 0}, Angle(), brush_tip, brush_size,
      {BrushBehavior::Target::kSaturationMultiplier}, {saturation_multiplier});

  EXPECT_FLOAT_EQ(state.saturation_multiplier, saturation_multiplier);
  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_multiplier = 3.f;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kSaturationMultiplier},
                         {clamp_multiplier});
  EXPECT_FLOAT_EQ(state.saturation_multiplier, 2.f);
}

TEST(CreateTipStateTest, WithBehaviorTargetingLuminosity) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float luminosity_offset = 0.3;
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                     {BrushBehavior::Target::kLuminosity}, {luminosity_offset});

  EXPECT_FLOAT_EQ(state.luminosity_shift, luminosity_offset);
  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_offset = 2.f;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kLuminosity}, {clamp_offset});
  EXPECT_FLOAT_EQ(state.luminosity_shift, 1.f);
}

TEST(CreateTipStateTest, WithBehaviorTargetingOpacity) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 2.5f;
  float opacity_multiplier = 1.3;
  BrushTipState state = CreateTipState(
      {0, 0}, Angle(), brush_tip, brush_size,
      {BrushBehavior::Target::kOpacityMultiplier}, {opacity_multiplier});

  EXPECT_FLOAT_EQ(state.opacity_multiplier,
                  brush_tip.opacity_multiplier * opacity_multiplier);
  EXPECT_FLOAT_EQ(state.width, brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));

  float clamp_multiplier = 3.f;
  state = CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                         {BrushBehavior::Target::kOpacityMultiplier},
                         {clamp_multiplier});
  EXPECT_FLOAT_EQ(state.opacity_multiplier, 2.f);
}

TEST(CreateTipStateTest, WithBehaviorsTargetingTheSameProperty) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 3.f;
  float modifiers[] = {1.5f, 0.8f};
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                     {BrushBehavior::Target::kWidthMultiplier,
                      BrushBehavior::Target::kWidthMultiplier},
                     modifiers);

  // Only the width should be affected by the sum of the modifiers:
  EXPECT_FLOAT_EQ(state.width,
                  modifiers[0] * modifiers[1] * brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));
}

TEST(CreateTipStateTest, WithBehaviorTargetingEachProperty) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 1.f;
  float modifiers[] = {0.9f, 1.2f};
  BrushTipState state =
      CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                     {BrushBehavior::Target::kWidthMultiplier,
                      BrushBehavior::Target::kHeightMultiplier},
                     modifiers);

  EXPECT_FLOAT_EQ(state.width, modifiers[0] * brush_tip.scale.x * brush_size);
  EXPECT_FLOAT_EQ(state.height, modifiers[1] * brush_tip.scale.y * brush_size);
  EXPECT_FLOAT_EQ(state.percent_radius, brush_tip.corner_rounding);
  EXPECT_THAT(state.rotation, AngleEq(brush_tip.rotation));
}

TEST(CreateTipStateTest, WidthIsClampedZeroToTwiceBaseValue) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 3.f;

  EXPECT_FLOAT_EQ(CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                                 {BrushBehavior::Target::kWidthMultiplier,
                                  BrushBehavior::Target::kWidthMultiplier},
                                 {-0.9f, 1.7f})
                      .width,
                  0);
  EXPECT_FLOAT_EQ(CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                                 {BrushBehavior::Target::kWidthMultiplier,
                                  BrushBehavior::Target::kWidthMultiplier},
                                 {1.8f, 1.7f})
                      .width,
                  2 * brush_tip.scale.x * brush_size);
}

TEST(CreateTipStateTest, HeightIsClampedZeroToTwiceBaseValue) {
  BrushTip brush_tip = MakeBaseBrushTip();
  float brush_size = 3.f;

  EXPECT_FLOAT_EQ(CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                                 {BrushBehavior::Target::kHeightMultiplier,
                                  BrushBehavior::Target::kHeightMultiplier},
                                 {0.5f, -0.3f})
                      .height,
                  0);
  EXPECT_FLOAT_EQ(CreateTipState({0, 0}, Angle(), brush_tip, brush_size,
                                 {BrushBehavior::Target::kHeightMultiplier,
                                  BrushBehavior::Target::kHeightMultiplier},
                                 {1.2f, 1.9f})
                      .height,
                  2 * brush_tip.scale.y * brush_size);
}

TEST(ModeledStrokeInputLerpTest, ZeroT) {
  ModeledStrokeInput a = {.position = {0, 0},
                          .velocity = {0, 0},
                          .traveled_distance = 73.f,
                          .elapsed_time = Duration32::Seconds(52),
                          .pressure = .3f,
                          .tilt = kQuarterTurn,
                          .orientation = kFullTurn / 16};
  ModeledStrokeInput b = {.position = {10, 10},
                          .velocity = {5, 5},
                          .traveled_distance = 79.f,
                          .elapsed_time = Duration32::Seconds(56),
                          .pressure = .7f,
                          .tilt = kFullTurn / 8,
                          .orientation = kFullTurn / 8};

  ModeledStrokeInput result = Lerp(a, b, 0);
  EXPECT_THAT(result, ModeledStrokeInputEq(a));
}

TEST(ModeledStrokeInputLerpTest, OneT) {
  ModeledStrokeInput a = {.position = {0, 0},
                          .velocity = {0, 0},
                          .traveled_distance = 73.f,
                          .elapsed_time = Duration32::Seconds(52),
                          .pressure = .3f,
                          .tilt = kQuarterTurn,
                          .orientation = kFullTurn / 16};
  ModeledStrokeInput b = {.position = {10, 10},
                          .velocity = {5, 5},
                          .traveled_distance = 79.f,
                          .elapsed_time = Duration32::Seconds(56),
                          .pressure = .7f,
                          .tilt = kFullTurn / 8,
                          .orientation = kFullTurn / 8};

  ModeledStrokeInput result = Lerp(a, b, 1);
  EXPECT_THAT(result, ModeledStrokeInputEq(b));
}

TEST(ModeledStrokeInputLerpTest, TBetweenZeroAndOne) {
  ModeledStrokeInput a = {.position = {0, 0},
                          .velocity = {0, 0},
                          .traveled_distance = 73.f,
                          .elapsed_time = Duration32::Seconds(52),
                          .pressure = .3f,
                          .tilt = kQuarterTurn,
                          .orientation = kFullTurn / 16};
  ModeledStrokeInput b = {.position = {10, 10},
                          .velocity = {5, 5},
                          .traveled_distance = 79.f,
                          .elapsed_time = Duration32::Seconds(56),
                          .pressure = .7f,
                          .tilt = kFullTurn / 8,
                          .orientation = kFullTurn / 8};

  ModeledStrokeInput result = Lerp(a, b, 0.2);
  EXPECT_THAT(result.position, PointEq({2, 2}));
  EXPECT_THAT(result.velocity, VecEq({1, 1}));
  EXPECT_FLOAT_EQ(result.traveled_distance, 74.2);
  EXPECT_THAT(result.elapsed_time, Duration32Eq(Duration32::Seconds(52.8)));
  EXPECT_FLOAT_EQ(result.pressure, 0.38);
  EXPECT_THAT(result.tilt, AngleEq(0.45 * kHalfTurn));
  EXPECT_THAT(result.orientation, AngleEq(0.15 * kHalfTurn));
}

TEST(ModeledStrokeInputLerpTest, AboveOneT) {
  ModeledStrokeInput a = {.position = {0, 0},
                          .velocity = {0, 0},
                          .traveled_distance = 73.f,
                          .elapsed_time = Duration32::Seconds(52),
                          .pressure = .3f,
                          .tilt = kQuarterTurn,
                          .orientation = kFullTurn / 16};
  ModeledStrokeInput b = {.position = {10, 10},
                          .velocity = {5, 5},
                          .traveled_distance = 79.f,
                          .elapsed_time = Duration32::Seconds(56),
                          .pressure = .7f,
                          .tilt = kFullTurn / 8,
                          .orientation = kFullTurn / 8};

  ModeledStrokeInput result = Lerp(a, b, 1.1);
  EXPECT_THAT(result.position, PointEq({11, 11}));
  EXPECT_THAT(result.velocity, VecEq({5.5, 5.5}));
  EXPECT_FLOAT_EQ(result.traveled_distance, 79.6);
  EXPECT_THAT(result.elapsed_time, Duration32Eq(Duration32::Seconds(56.4)));
  EXPECT_FLOAT_EQ(result.pressure, 0.74);
  EXPECT_THAT(result.tilt, AngleEq(0.225 * kHalfTurn));
  EXPECT_THAT(result.orientation, AngleEq(0.2625 * kHalfTurn));
}

TEST(ModeledStrokeInputLerpTest, BelowZeroT) {
  ModeledStrokeInput a = {.position = {0, 0},
                          .velocity = {0, 0},
                          .traveled_distance = 73.f,
                          .elapsed_time = Duration32::Seconds(52),
                          .pressure = .3f,
                          .tilt = kQuarterTurn,
                          .orientation = kFullTurn / 16};
  ModeledStrokeInput b = {.position = {10, 10},
                          .velocity = {5, 5},
                          .traveled_distance = 79.f,
                          .elapsed_time = Duration32::Seconds(56),
                          .pressure = .7f,
                          .tilt = kFullTurn / 8,
                          .orientation = kFullTurn / 8};

  ModeledStrokeInput result = Lerp(a, b, -0.1);
  EXPECT_THAT(result.position, PointEq({-1, -1}));
  EXPECT_THAT(result.velocity, VecEq({-.5, -.5}));
  EXPECT_FLOAT_EQ(result.traveled_distance, 72.4);
  EXPECT_THAT(result.elapsed_time, Duration32Eq(Duration32::Seconds(51.6)));
  EXPECT_FLOAT_EQ(result.pressure, 0.26);
  EXPECT_THAT(result.tilt, AngleEq(0.525 * kHalfTurn));
  EXPECT_THAT(result.orientation, AngleEq(0.1125 * kHalfTurn));
}

}  // namespace
}  // namespace ink::strokes_internal
