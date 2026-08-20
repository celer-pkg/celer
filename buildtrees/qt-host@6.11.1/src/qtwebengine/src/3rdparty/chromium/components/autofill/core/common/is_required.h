// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_COMMON_IS_REQUIRED_H_
#define COMPONENTS_AUTOFILL_CORE_COMMON_IS_REQUIRED_H_

#if !defined(COMPILER_MSVC) &&  \
    (!defined(__GNUC__) || (__GNUC__ >= 12))
#define IS_REQUIRED_SUPPORTED 1
#endif

namespace autofill::internal {

// Auxiliary type to mark members of a struct as required:
//
//   struct Foo {
//     int bar = IsRequired();
//   };
struct IsRequired {
  // This function is not defined and consteval. Therefore, any evaluation will
  // fail and fail at compile time.
  template <typename T>
  consteval operator T();  // NOLINT
};

}  // namespace autofill::internal

#endif  // COMPONENTS_AUTOFILL_CORE_COMMON_IS_REQUIRED_H_
