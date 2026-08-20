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

#include "ink/brush/brush.h"

#include <cmath>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/substitute.h"
#include "ink/brush/brush_family.h"
#include "ink/color/color.h"

namespace ink {
namespace {

absl::Status ValidateBrushSize(float size) {
  if (!std::isfinite(size) || size <= 0) {
    return absl::InvalidArgumentError(absl::Substitute(
        "`size` must be a finite and positive value. Received: $0", size));
  }

  return absl::OkStatus();
}

absl::Status ValidateBrushEpsilon(float epsilon) {
  if (!std::isfinite(epsilon) || epsilon <= 0) {
    return absl::InvalidArgumentError(absl::Substitute(
        "`epsilon` must be a finite and positive value. Received: $0",
        epsilon));
  }

  return absl::OkStatus();
}

absl::Status ValidateBrushSizeRelativeToEpsilon(float size, float epsilon) {
  if (size < epsilon) {
    return absl::InvalidArgumentError(
        absl::Substitute("`size` must be greater than or equal to `epsilon`. "
                         "Received size: $0 epsilon: $1",
                         size, epsilon));
  }

  return absl::OkStatus();
}

}  // namespace

Brush::Brush(const BrushFamily& family, const Color& color, float size,
             float epsilon)
    : family_(family), color_(color), size_(size), epsilon_(epsilon) {}

absl::StatusOr<Brush> Brush::Create(const BrushFamily& family,
                                    const Color& color, float size,
                                    float epsilon) {
  absl::Status status = ValidateBrushSize(size);
  if (!status.ok()) {
    return status;
  }
  if (status = ValidateBrushEpsilon(epsilon); !status.ok()) {
    return status;
  }
  if (status = ValidateBrushSizeRelativeToEpsilon(size, epsilon);
      !status.ok()) {
    return status;
  }
  return Brush(family, color, size, epsilon);
}

absl::Status Brush::SetSize(float size) {
  absl::Status status = ValidateBrushSize(size);
  if (!status.ok()) {
    return status;
  }
  if (status = ValidateBrushSizeRelativeToEpsilon(size, epsilon_);
      !status.ok()) {
    return status;
  }
  size_ = size;
  return absl::OkStatus();
}

absl::Status Brush::SetEpsilon(float epsilon) {
  absl::Status status = ValidateBrushEpsilon(epsilon);
  if (!status.ok()) {
    return status;
  }
  if (status = ValidateBrushSizeRelativeToEpsilon(size_, epsilon);
      !status.ok()) {
    return status;
  }
  epsilon_ = epsilon;
  return absl::OkStatus();
}

std::string Brush::ToFormattedString() const {
  return absl::StrCat("Brush(color=", color_, ", size=", size_,
                      ", epsilon=", epsilon_, ", family=", family_, ")");
}

}  // namespace ink
