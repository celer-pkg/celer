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

#include <jni.h>

#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/easing_function.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_throw_util.h"

namespace {

using ink::BrushBehavior;
using ink::brush_internal::ValidateBrushBehaviorNode;
using ink::brush_internal::ValidateBrushBehaviorTopLevel;
using ink::jni::CastToBrushBehavior;
using ink::jni::CastToBrushBehaviorNode;
using ink::jni::CastToEasingFunction;
using ink::jni::DeleteNativeBrushBehavior;
using ink::jni::DeleteNativeBrushBehaviorNode;
using ink::jni::NewNativeBrushBehavior;
using ink::jni::NewNativeBrushBehaviorNode;
using ink::jni::NewNativeEasingFunction;
using ink::jni::ThrowExceptionFromStatus;

jlong ValidateAndHoistNodeOrThrow(BrushBehavior::Node node, JNIEnv* env) {
  if (absl::Status status = ValidateBrushBehaviorNode(node); !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return 0;
  }
  return NewNativeBrushBehaviorNode(std::move(node));
}

// Helper type for visitor pattern. This is a quick way of constructing a
// visitor instance with overloaded operator() from an initializer list of
// lambdas. An advantage of using this approach with visitor is that the
// compiler will check that the overloads are exhaustive.
template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

// Type-deduction guide required for overloads{...} to work properly as a
// constructor in some C++ versions.
template <class... Ts>
overloads(Ts...) -> overloads<Ts...>;

static constexpr int kSourceNode = 0;
static constexpr int kConstantNode = 1;
static constexpr int kNoiseNode = 2;
static constexpr int kFallbackFilterNode = 3;
static constexpr int kToolTypeFilterNode = 4;
static constexpr int kDampingNode = 5;
static constexpr int kResponseNode = 6;
static constexpr int kBinaryOpNode = 7;
static constexpr int kInterpolationNode = 8;
static constexpr int kTargetNode = 9;
static constexpr int kPolarTargetNode = 10;

}  // namespace

extern "C" {

JNI_METHOD(brush, BrushBehaviorNative, jlong, createFromOrderedNodes)
(JNIEnv* env, jobject thiz, jlongArray node_native_pointer_array) {
  std::vector<BrushBehavior::Node> nodes;
  const jsize num_nodes = env->GetArrayLength(node_native_pointer_array);
  nodes.reserve(num_nodes);
  jlong* node_pointers =
      env->GetLongArrayElements(node_native_pointer_array, nullptr);
  ABSL_CHECK(node_pointers != nullptr);
  for (jsize i = 0; i < num_nodes; ++i) {
    nodes.push_back(CastToBrushBehaviorNode(node_pointers[i]));
  }
  env->ReleaseLongArrayElements(
      node_native_pointer_array, node_pointers,
      // No need to copy back the array, which is not modified.
      JNI_ABORT);
  BrushBehavior behavior{.nodes = std::move(nodes)};
  if (absl::Status status = ValidateBrushBehaviorTopLevel(behavior);
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return 0;
  }
  return NewNativeBrushBehavior(std::move(behavior));
}

JNI_METHOD(brush, BrushBehaviorNative, void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeBrushBehavior(native_pointer);
}

JNI_METHOD(brush, BrushBehaviorNative, jint, getNodeCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushBehavior(native_pointer).nodes.size();
}

JNI_METHOD(brush, BrushBehaviorNative, jlong, newCopyOfNode)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint index) {
  return NewNativeBrushBehaviorNode(
      CastToBrushBehavior(native_pointer).nodes[index]);
}

// Functions for dealing with BrushBehavior::Nodes. Note that on the C++ side,
// BrushBehavior::Node is a variant, so all of the native pointers are pointers
// to the same type (the variant BrushBehavior::Node).

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createSource)
(JNIEnv* env, jobject thiz, jint source, jfloat source_value_start,
 jfloat source_value_end, jint source_out_of_range_behavior) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::SourceNode{
          .source = static_cast<BrushBehavior::Source>(source),
          .source_out_of_range_behavior =
              static_cast<BrushBehavior::OutOfRange>(
                  source_out_of_range_behavior),
          .source_value_range = {source_value_start, source_value_end},
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createConstant)
(JNIEnv* env, jobject thiz, jfloat value) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::ConstantNode{.value = value}, env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createNoise)
(JNIEnv* env, jobject thiz, jint seed, jint vary_over, jfloat base_period) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::NoiseNode{
          .seed = static_cast<uint32_t>(seed),
          .vary_over = static_cast<BrushBehavior::DampingSource>(vary_over),
          .base_period = base_period,
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createFallbackFilter)
(JNIEnv* env, jobject thiz, jint is_fallback_for) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::FallbackFilterNode{
          .is_fallback_for = static_cast<BrushBehavior::OptionalInputProperty>(
              is_fallback_for),
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createToolTypeFilter)
(JNIEnv* env, jobject thiz, jboolean mouse_enabled, jboolean touch_enabled,
 jboolean stylus_enabled, jboolean unknown_enabled) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::ToolTypeFilterNode{
          .enabled_tool_types = {.unknown = unknown_enabled ? true : false,
                                 .mouse = mouse_enabled ? true : false,
                                 .touch = touch_enabled ? true : false,
                                 .stylus = stylus_enabled ? true : false},
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createDamping)
(JNIEnv* env, jobject thiz, jint damping_source, jfloat damping_gap) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::DampingNode{
          .damping_source =
              static_cast<BrushBehavior::DampingSource>(damping_source),
          .damping_gap = damping_gap,
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createResponse)
(JNIEnv* env, jobject thiz, jlong easing_function_native_pointer) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::ResponseNode{
          .response_curve =
              CastToEasingFunction(easing_function_native_pointer),
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createBinaryOp)
(JNIEnv* env, jobject thiz, jint operation) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::BinaryOpNode{
          .operation = static_cast<BrushBehavior::BinaryOp>(operation),
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createInterpolation)
(JNIEnv* env, jobject thiz, jint interpolation) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::InterpolationNode{
          .interpolation =
              static_cast<BrushBehavior::Interpolation>(interpolation),
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createTarget)
(JNIEnv* env, jobject thiz, jint target, jfloat target_modifier_start,
 jfloat target_modifier_end) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::TargetNode{
          .target = static_cast<BrushBehavior::Target>(target),
          .target_modifier_range = {target_modifier_start, target_modifier_end},
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong, createPolarTarget)
(JNIEnv* env, jobject thiz, jint polar_target, jfloat angle_range_start,
 jfloat angle_range_end, jfloat magnitude_range_start,
 jfloat magnitude_range_end) {
  return ValidateAndHoistNodeOrThrow(
      BrushBehavior::PolarTargetNode{
          .target = static_cast<BrushBehavior::PolarTarget>(polar_target),
          .angle_range = {angle_range_start, angle_range_end},
          .magnitude_range = {magnitude_range_start, magnitude_range_end},
      },
      env);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, void, free)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  DeleteNativeBrushBehaviorNode(node_native_pointer);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getNodeType)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  constexpr auto visitor = overloads{
      [](const BrushBehavior::SourceNode&) { return kSourceNode; },
      [](const BrushBehavior::ConstantNode&) { return kConstantNode; },
      [](const BrushBehavior::NoiseNode&) { return kNoiseNode; },
      [](const BrushBehavior::FallbackFilterNode&) {
        return kFallbackFilterNode;
      },
      [](const BrushBehavior::ToolTypeFilterNode&) {
        return kToolTypeFilterNode;
      },
      [](const BrushBehavior::DampingNode&) { return kDampingNode; },
      [](const BrushBehavior::ResponseNode&) { return kResponseNode; },
      [](const BrushBehavior::BinaryOpNode&) { return kBinaryOpNode; },
      [](const BrushBehavior::InterpolationNode&) {
        return kInterpolationNode;
      },
      [](const BrushBehavior::TargetNode&) { return kTargetNode; },
      [](const BrushBehavior::PolarTargetNode&) { return kPolarTargetNode; }};
  return std::visit(visitor, CastToBrushBehaviorNode(node_native_pointer));
}

// SourceNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getSourceInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::SourceNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .source);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getSourceValueRangeStart)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::SourceNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .source_value_range[0];
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getSourceValueRangeEnd)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::SourceNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .source_value_range[1];
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getSourceOutOfRangeBehaviorInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::SourceNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .source_out_of_range_behavior);
}

// ConstantNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getConstantValue)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::ConstantNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .value;
}

// NoiseNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getNoiseSeed)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::NoiseNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .seed);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getNoiseVaryOverInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::NoiseNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .vary_over);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getNoiseBasePeriod)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::NoiseNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .base_period;
}

// FallbackFilterNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint,
           getFallbackFilterIsFallbackForInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::FallbackFilterNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .is_fallback_for);
}

// ToolTypeFilterNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jboolean,
           getToolTypeFilterMouseEnabled)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::ToolTypeFilterNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .enabled_tool_types.mouse;
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jboolean,
           getToolTypeFilterTouchEnabled)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::ToolTypeFilterNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .enabled_tool_types.touch;
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jboolean,
           getToolTypeFilterStylusEnabled)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::ToolTypeFilterNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .enabled_tool_types.stylus;
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jboolean,
           getToolTypeFilterUnknownEnabled)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::ToolTypeFilterNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .enabled_tool_types.unknown;
}

// DampingNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getDampingSourceInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::DampingNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .damping_source);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getDampingGap)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::DampingNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .damping_gap;
}

// ResponseNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jlong,
           newCopyOfResponseEasingFunction)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return NewNativeEasingFunction(
      std::get<BrushBehavior::ResponseNode>(
          CastToBrushBehaviorNode(node_native_pointer))
          .response_curve);
}

// BinaryOpNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getBinaryOperationInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::BinaryOpNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .operation);
}

// InterpolationNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getInterpolationInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::InterpolationNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .interpolation);
}

// TargetNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getTargetInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::TargetNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .target);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getTargetModifierRangeStart)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::TargetNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .target_modifier_range[0];
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getTargetModifierRangeEnd)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::TargetNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .target_modifier_range[1];
}

// PolarTargetNode accessors:

JNI_METHOD(brush, BrushBehaviorNodeNative, jint, getPolarTargetInt)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return static_cast<jint>(std::get<BrushBehavior::PolarTargetNode>(
                               CastToBrushBehaviorNode(node_native_pointer))
                               .target);
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat,
           getPolarTargetAngleRangeStart)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::PolarTargetNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .angle_range[0];
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat, getPolarTargetAngleRangeEnd)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::PolarTargetNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .angle_range[1];
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat,
           getPolarTargetMagnitudeRangeStart)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::PolarTargetNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .magnitude_range[0];
}

JNI_METHOD(brush, BrushBehaviorNodeNative, jfloat,
           getPolarTargetMagnitudeRangeEnd)
(JNIEnv* env, jobject thiz, jlong node_native_pointer) {
  return std::get<BrushBehavior::PolarTargetNode>(
             CastToBrushBehaviorNode(node_native_pointer))
      .magnitude_range[1];
}

}  // extern "C"
