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

/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INK_TYPES_NUMBERS_H_
#define INK_TYPES_NUMBERS_H_

#if __cplusplus >= 202002L
#include <numbers>
#else
#include <cmath>
#endif

namespace ink {
namespace numbers {

// Constant definitions for π and √2.
//
// M_PI and friends are not available by default with MSVC STL.
#if __cplusplus >= 202002L
inline constexpr double kPi = std::numbers::pi;
inline constexpr double kSqrt2 = std::numbers::sqrt2;
#else
inline constexpr double kPi = M_PI;
inline constexpr double kSqrt2 = M_SQRT2;
#endif

}  // namespace numbers
}  // namespace ink

#endif  // INK_TYPES_NUMBERS_H_
