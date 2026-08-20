// Copyright 2024-2025 Google LLC
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

#ifndef INK_STORAGE_BRUSH_H_
#define INK_STORAGE_BRUSH_H_

#include <functional>
#include <optional>
#include <string>

#include "absl/status/statusor.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/storage/proto/brush.pb.h"
#include "ink/storage/proto/brush_family.pb.h"

namespace ink {

// Provides a bitmap for a given client texture `id`, if one exists. The
// returned string is the bytes of the PNG-encoded bitmap, or std::nullopt if
// the bitmap is not available.
using TextureBitmapProvider =
    std::function<std::optional<std::string>(const std::string& id)>;

// Provides a new client texture ID for a given encoded texture ID, and is
// responsible for receiving (e.g. by storing) the corresponding bitmap.
// `bitmap` is the bytes of the PNG-encoded bitmap that the proto associated
// with `encoded_id`, or the empty string if there was no associated bitmap.
using ClientTextureIdProviderAndBitmapReceiver =
    std::function<absl::StatusOr<std::string>(const std::string& encoded_id,
                                              const std::string& bitmap)>;

// Provides a new client texture ID for a given encoded texture ID.
using ClientTextureIdProvider =
    std::function<absl::StatusOr<std::string>(const std::string& encoded_id)>;

// Populates the given proto by encoding the given brush object.
//
// The proto need not be empty before calling these; they will effectively clear
// the proto first.
void EncodeBrush(
    const Brush& brush, proto::Brush& brush_proto_out,
    TextureBitmapProvider get_bitmap = [](const std::string& id) {
      return std::nullopt;
    });
void EncodeBrushFamily(
    const BrushFamily& family, proto::BrushFamily& family_proto_out,
    TextureBitmapProvider get_bitmap = [](const std::string& id) {
      return std::nullopt;
    });
void EncodeBrushFamilyTextureMap(
    const BrushFamily& family,
    google::protobuf::Map<std::string, std::string>& texture_id_to_bitmap_out,
    TextureBitmapProvider get_bitmap);
void EncodeBrushCoat(const BrushCoat& coat, proto::BrushCoat& coat_proto_out);
void EncodeBrushPaint(const BrushPaint& paint,
                      proto::BrushPaint& paint_proto_out);
void EncodeBrushTip(const BrushTip& tip, proto::BrushTip& tip_proto_out);
void EncodeBrushBehaviorNode(const BrushBehavior::Node& node,
                             proto::BrushBehavior::Node& node_proto_out);

// Decodes the proto into a brush object. Returns an error if the proto is
// invalid.
absl::StatusOr<Brush> DecodeBrush(
    const proto::Brush& brush_proto,
    ClientTextureIdProviderAndBitmapReceiver get_client_texture_id =
        [](const std::string& encoded_id, const std::string& bitmap) {
          return encoded_id;
        });
absl::StatusOr<BrushFamily> DecodeBrushFamily(
    const proto::BrushFamily& family_proto,
    ClientTextureIdProviderAndBitmapReceiver get_client_texture_id =
        [](const std::string& encoded_id, const std::string& bitmap) {
          return encoded_id;
        });
absl::StatusOr<BrushCoat> DecodeBrushCoat(
    const proto::BrushCoat& coat_proto,
    ClientTextureIdProvider get_client_texture_id =
        [](const std::string& encoded_id) { return encoded_id; });
absl::StatusOr<BrushPaint> DecodeBrushPaint(
    const proto::BrushPaint& paint_proto,
    ClientTextureIdProvider get_client_texture_id =
        [](const std::string& encoded_id) { return encoded_id; });
absl::StatusOr<BrushTip> DecodeBrushTip(const proto::BrushTip& tip_proto);
absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorNode(
    const proto::BrushBehavior::Node& node_proto);

}  // namespace ink

#endif  // INK_STORAGE_BRUSH_H_
