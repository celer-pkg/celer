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

#ifndef INK_GEOMETRY_INTERNAL_MODULO_H_
#define INK_GEOMETRY_INTERNAL_MODULO_H_

namespace ink::geometry_internal {

// Returns `a` mod `b`, with the result being in the range [0, `b`). This is
// different from `std::fmodf`, which returns a negative result if `a` is
// negative. `b` must be finite and strictly greater than zero. Returns NaN if
// `a` is infinite or NaN.
float FloatModulo(float a, float b);

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_MODULO_H_
