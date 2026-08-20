// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/354829279): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#ifndef UI_GFX_GEOMETRY_DOUBLE4_H_
#define UI_GFX_GEOMETRY_DOUBLE4_H_

#include <type_traits>

namespace gfx {

// This header defines Double4 type for vectorized SIMD operations used in
// optimized transformation code. The type should be only used for local
// variables, or inline function parameters or return values. Don't use the
// type in other cases (e.g. for class data members) due to constraints
// (e.g. alignment).
//
// Here are some examples of usages:
//
//   double matrix[4][4] = ...;
//   // The scalar value will be applied to all components.
//   Double4 c0 = Load(matrix[0]) + 5;
//   Double4 c1 = Load(matrix[1]) * Double4{1, 2, 3, 4};
//
//   Double4 v = c0 * c1;
//   // s0/s1/s2/s3 are preferred to x/y/z/w for consistency.
//   double a = v.s0 + Sum(c1);
//   // v.s3210 is equivalent to {v.s3, v.s2, v.s1, v.s0}.
//   // Should use this form instead of __builtin_shufflevector() etc.
//   Double4 swapped = {v[3], v[2], v[1], v[0]};
//
//   // Logical operations.
//   bool b1 = AllTrue(swapped == c0);
//   // & is preferred to && to reduce branches.
//   bool b2 = AllTrue((c0 == c1) & (c0 == v) & (c0 >= swapped));
//
//   Store(swapped, matrix_[2]);
//   Store(v, matrix_[3]);
//
// We use the gcc extension (supported by clang) instead of the clang extension
// to make sure the code can compile with either gcc or clang.
//
// For more details, see
//   https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html

#if !defined(__GNUC__) && !defined(__clang__)
struct DoubleBoolean4 {
    alignas(16) int64_t b[4];
    DoubleBoolean4 operator&(const DoubleBoolean4& o) const
    { DoubleBoolean4 out;
        out.b[0] = b[0] & o.b[0];
        out.b[1] = b[1] & o.b[1];
        out.b[2] = b[2] & o.b[2];
        out.b[3] = b[3] & o.b[3];
        return out;
    }
};

struct Double4 {
    alignas(16) double d[4];
    DoubleBoolean4 operator==(const Double4& o) const
    { return { d[0] == o.d[0],
               d[1] == o.d[1],
               d[2] == o.d[2],
               d[3] == o.d[3] };
    }
    double& operator[](int i) { return d[i]; }
    void operator*=(const Double4& o)
    { d[0] *= o.d[0];
      d[1] *= o.d[1];
      d[2] *= o.d[2];
      d[3] *= o.d[3];
    }
    void operator*=(double o)
    { d[0] *= o;
      d[1] *= o;
      d[2] *= o;
      d[3] *= o;
    }
    void operator+=(const Double4& o)
    { d[0] += o.d[0];
      d[1] += o.d[1];
      d[2] += o.d[2];
      d[3] += o.d[3];
    }
    void operator-=(const Double4& o)
    { d[0] -= o.d[0];
      d[1] -= o.d[1];
      d[2] -= o.d[2];
      d[3] -= o.d[3];
    }
    Double4 operator*(const Double4& o) const
    { return { d[0] * o.d[0],
               d[1] * o.d[1],
               d[2] * o.d[2],
               d[3] * o.d[3] };
    }
    Double4 operator*(double o) const
    { return { d[0] * o,
               d[1] * o,
               d[2] * o,
               d[3] * o };
    }
    Double4 operator/(double o) const
    { return { d[0] / o,
               d[1] / o,
               d[2] / o,
               d[3] / o };
    }
    Double4 operator+(const Double4& o) const
    { return { d[0] + o.d[0],
               d[1] + o.d[1],
               d[2] + o.d[2],
               d[3] + o.d[3] };
    }
    Double4 operator+(double o) const
    { return { d[0] + o,
               d[1] + o,
               d[2] + o,
               d[3] + o };
    }
    Double4 operator-(const Double4& o) const
    { return { d[0] - o.d[0],
               d[1] - o.d[1],
               d[2] - o.d[2],
               d[3] - o.d[3] };
    }
    friend Double4 operator*(double d, const Double4 &o)
    { return o * d; }
    friend Double4 operator/(double d, const Double4 &o)
    { return { d / o.d[0],
               d / o.d[1],
               d / o.d[2],
               d / o.d[3] };
    }
};

ALWAYS_INLINE double Sum(Double4 v) {
  return v.d[0] + v.d[1] + v.d[2] + v.d[3];
}

ALWAYS_INLINE Double4 LoadDouble4(const double s[4]) {
  return Double4({s[0], s[1], s[2], s[3]});
}

ALWAYS_INLINE void StoreDouble4(Double4 v, double d[4]) {
  d[0] = v.d[0];
  d[1] = v.d[1];
  d[2] = v.d[2];
  d[3] = v.d[3];
}

ALWAYS_INLINE int64_t AllTrue(DoubleBoolean4 v) {
  return v.b[0] & v.b[1] & v.b[2] & v.b[3];
}

#else
typedef double __attribute__((vector_size(4 * sizeof(double)))) Double4;
typedef float __attribute__((vector_size(4 * sizeof(float)))) Float4;

ALWAYS_INLINE double Sum(Double4 v) {
  return v[0] + v[1] + v[2] + v[3];
}

ALWAYS_INLINE Double4 LoadDouble4(const double s[4]) {
  return Double4{s[0], s[1], s[2], s[3]};
}

ALWAYS_INLINE void StoreDouble4(Double4 v, double d[4]) {
  d[0] = v[0];
  d[1] = v[1];
  d[2] = v[2];
  d[3] = v[3];
}

// The parameter should be the result of Double4/Float4 operations that would
// produce bool results if they were original scalar operators, e.g.
//   auto b4 = double4_a == double4_b;
// A zero value of a component of |b4| means false, otherwise true.
// This function checks whether all 4 components in |b4| are true.
// |&| instead of |&&| is used to avoid branches, which results shorter and
// faster code in most cases. It's used like:
//   if (AllTrue(double4_a == double4_b))
//     ...
//   if (AllTrue((double4_a1 == double4_b1) & (double4_a2 == double4_b2)))
//     ...
typedef int64_t __attribute__((vector_size(4 * sizeof(int64_t))))
DoubleBoolean4;
ALWAYS_INLINE int64_t AllTrue(DoubleBoolean4 b4) {
  return b4[0] & b4[1] & b4[2] & b4[3];
}

typedef int32_t __attribute__((vector_size(4 * sizeof(int32_t)))) FloatBoolean4;
ALWAYS_INLINE int32_t AllTrue(FloatBoolean4 b4) {
  return b4[0] & b4[1] & b4[2] & b4[3];
}
#endif
}  // namespace gfx

#endif  // UI_GFX_GEOMETRY_DOUBLE4_H_
