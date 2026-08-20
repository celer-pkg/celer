// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_COMPILER_OPERATION_TYPER_H_
#define V8_COMPILER_OPERATION_TYPER_H_

#include "src/base/flags.h"
#include "src/compiler/opcodes.h"
#include "src/compiler/turbofan-types.h"

#define TYPER_SUPPORTED_MACHINE_BINOP_LIST(V) \
  V(Int32Add)                                 \
  V(Int32LessThanOrEqual)                     \
  V(Int64Add)                                 \
  V(Int32Sub)                                 \
  V(Int64Sub)                                 \
  V(Load)                                     \
  V(Uint32Div)                                \
  V(Uint64Div)                                \
  V(Uint32LessThan)                           \
  V(Uint32LessThanOrEqual)                    \
  V(Uint64LessThan)                           \
  V(Uint64LessThanOrEqual)                    \
  V(Word32And)                                \
  V(Word32Equal)                              \
  V(Word32Or)                                 \
  V(Word32Shl)                                \
  V(Word32Shr)                                \
  V(Word64And)                                \
  V(Word64Shl)                                \
  V(Word64Shr)

namespace v8 {
namespace internal {

// Forward declarations.
class Isolate;
class RangeType;
class Zone;

namespace compiler {

// Forward declarations.
class Operator;
class Type;
class TypeCache;

class V8_EXPORT_PRIVATE OperationTyper {
 public:
  OperationTyper(JSHeapBroker* broker, Zone* zone);

  // Typing Phi.
  compiler::Type Merge(compiler::Type left, compiler::Type right);

  compiler::Type ToPrimitive(compiler::Type type);
  compiler::Type ToNumber(compiler::Type type);
  compiler::Type ToNumberConvertBigInt(compiler::Type type);
  compiler::Type ToBigInt(compiler::Type type);
  compiler::Type ToBigIntConvertNumber(compiler::Type type);
  compiler::Type ToNumeric(compiler::Type type);
  compiler::Type ToBoolean(compiler::Type type);

  compiler::Type WeakenRange(compiler::Type current_range, compiler::Type previous_range);

// Unary operators.
#define DECLARE_METHOD(Name) compiler::Type Name(compiler::Type type);
  SIMPLIFIED_NUMBER_UNOP_LIST(DECLARE_METHOD)
  SIMPLIFIED_BIGINT_UNOP_LIST(DECLARE_METHOD)
  SIMPLIFIED_SPECULATIVE_NUMBER_UNOP_LIST(DECLARE_METHOD)
  SIMPLIFIED_SPECULATIVE_BIGINT_UNOP_LIST(DECLARE_METHOD)
  DECLARE_METHOD(ConvertReceiver)
#undef DECLARE_METHOD

// Numeric binary operators.
#define DECLARE_METHOD(Name) compiler::Type Name(compiler::Type lhs, compiler::Type rhs);
  SIMPLIFIED_NUMBER_BINOP_LIST(DECLARE_METHOD)
  SIMPLIFIED_BIGINT_BINOP_LIST(DECLARE_METHOD)
  SIMPLIFIED_SPECULATIVE_NUMBER_BINOP_LIST(DECLARE_METHOD)
  SIMPLIFIED_SPECULATIVE_BIGINT_BINOP_LIST(DECLARE_METHOD)
  TYPER_SUPPORTED_MACHINE_BINOP_LIST(DECLARE_METHOD)
#undef DECLARE_METHOD

  compiler::Type ChangeUint32ToUint64(compiler::Type input);

  // Comparison operators.
  compiler::Type SameValue(compiler::Type lhs, compiler::Type rhs);
  compiler::Type SameValueNumbersOnly(compiler::Type lhs, compiler::Type rhs);
  compiler::Type StrictEqual(compiler::Type lhs, compiler::Type rhs);

  // Check operators.
  compiler::Type CheckBounds(compiler::Type index, compiler::Type length);
  compiler::Type CheckFloat64Hole(compiler::Type type);
  compiler::Type CheckNumber(compiler::Type type);
  compiler::Type CheckNumberOrUndefined(compiler::Type type);
  compiler::Type CheckNumberFitsInt32(compiler::Type type);
  compiler::Type ConvertTaggedHoleToUndefined(compiler::Type type);

  compiler::Type TypeTypeGuard(const Operator* sigma_op, compiler::Type input);

  enum ComparisonOutcomeFlags {
    kComparisonTrue = 1,
    kComparisonFalse = 2,
    kComparisonUndefined = 4
  };

  compiler::Type singleton_false() const { return singleton_false_; }
  compiler::Type singleton_true() const { return singleton_true_; }

 private:
  using ComparisonOutcome = base::Flags<ComparisonOutcomeFlags>;

  ComparisonOutcome Invert(ComparisonOutcome);
  compiler::Type Invert(compiler::Type);
  compiler::Type FalsifyUndefined(ComparisonOutcome);

  compiler::Type Rangify(compiler::Type);
  compiler::Type AddRanger(double lhs_min, double lhs_max, double rhs_min,
                 double rhs_max);
  compiler::Type SubtractRanger(double lhs_min, double lhs_max, double rhs_min,
                      double rhs_max);
  compiler::Type MultiplyRanger(double lhs_min, double lhs_max, double rhs_min,
                      double rhs_max);

  Zone* zone() const { return zone_; }

  Zone* const zone_;
  TypeCache const* cache_;

  compiler::Type infinity_;
  compiler::Type minus_infinity_;
  compiler::Type singleton_NaN_string_;
  compiler::Type singleton_zero_string_;
  compiler::Type singleton_false_;
  compiler::Type singleton_true_;
  compiler::Type signed32ish_;
  compiler::Type unsigned32ish_;
  compiler::Type singleton_empty_string_;
  compiler::Type truish_;
  compiler::Type falsish_;
};

}  // namespace compiler
}  // namespace internal
}  // namespace v8

#endif  // V8_COMPILER_OPERATION_TYPER_H_
