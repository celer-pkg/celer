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

#ifndef INK_RENDERING_SKIA_NATIVE_TEXTURE_BITMAP_STORE_H_
#define INK_RENDERING_SKIA_NATIVE_TEXTURE_BITMAP_STORE_H_

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"

namespace ink {

// Interface for loading texture bitmap data for texture ids. Implementations
// of this define the mapping between the texture IDs used by a client and the
// corresponding bitmap data.
class TextureBitmapStore {
 public:
  virtual ~TextureBitmapStore() = default;

  // Retrieve a `Bitmap` for the given texture id. This may be called
  // synchronously during rendering, so loading of texture files from disk and
  // decoding them into bitmap data should be done in advance. The result may be
  // cached by consumers, so this should return a deterministic result for a
  // given input.
  virtual absl::StatusOr<sk_sp<SkImage>> GetTextureBitmap(
      absl::string_view texture_id) const = 0;
};

}  // namespace ink

#endif  // INK_RENDERING_SKIA_NATIVE_TEXTURE_BITMAP_STORE_H_
