// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_LOGGING_H_
#define BASE_LOGGING_H_

#include <errno.h>
#include <string.h>

#include <iostream>

#define CHECK(x)                                 \
  do {                                           \
    if (!(x)) {                                  \
      std::cerr << "CHECK(" << #x << ") failed"; \
      __builtin_trap();                          \
      __builtin_unreachable();                   \
    }                                            \
  } while (0)

#define CHECK_LT(val1, val2)                                            \
  do {                                                                  \
    if (!((val1) < (val2))) {                                           \
      std::cerr << "CHECK_LT(" << #val1 << ", " << #val2 << ") failed"; \
      __builtin_trap();                                                 \
      __builtin_unreachable();                                          \
    }                                                                   \
  } while (0)

#define CHECK_GE(val1, val2)                                            \
  do {                                                                  \
    if (!((val1) >= (val2))) {                                          \
      std::cerr << "CHECK_GE(" << #val1 << ", " << #val2 << ") failed"; \
      __builtin_trap();                                                 \
      __builtin_unreachable();                                          \
    }                                                                   \
  } while (0)

#define CHECK_LE(val1, val2)                                            \
  do {                                                                  \
    if (!((val1) <= (val2))) {                                          \
      std::cerr << "CHECK_LE(" << #val1 << ", " << #val2 << ") failed"; \
      __builtin_trap();                                                 \
      __builtin_unreachable();                                          \
    }                                                                   \
  } while (0)

#define CHECK_GT(val1, val2)                                            \
  do {                                                                  \
    if (!((val1) > (val2))) {                                           \
      std::cerr << "CHECK_GT(" << #val1 << ", " << #val2 << ") failed"; \
      __builtin_trap();                                                 \
      __builtin_unreachable();                                          \
    }                                                                   \
  } while (0)

#define CHECK_EQ(val1, val2)                                            \
  do {                                                                  \
    if (!((val1) == (val2))) {                                          \
      std::cerr << "CHECK_EQ(" << #val1 << ", " << #val2 << ") failed"; \
      __builtin_trap();                                                 \
      __builtin_unreachable();                                          \
    }                                                                   \
  } while (0)

#define CHECK_NE(val1, val2)                                            \
  do {                                                                  \
    if (!((val1) != (val2))) {                                          \
      std::cerr << "CHECK_NE(" << #val1 << ", " << #val2 << ") failed"; \
      __builtin_trap();                                                 \
      __builtin_unreachable();                                          \
    }                                                                   \
  } while (0)

// Based on Chromium's //base/posix/eintr_wrapper.h.
#define HANDLE_EINTR(x)                                     \
  ([&]() -> decltype(x) {                                   \
    decltype(x) eintr_wrapper_result;                       \
    do {                                                    \
      eintr_wrapper_result = (x);                           \
    } while (eintr_wrapper_result == -1 && errno == EINTR); \
    return eintr_wrapper_result;                            \
  })()

#endif  // BASE_LOGGING_H_