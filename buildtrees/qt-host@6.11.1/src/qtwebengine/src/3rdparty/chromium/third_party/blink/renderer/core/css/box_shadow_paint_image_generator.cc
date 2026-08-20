// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/css/box_shadow_paint_image_generator.h"

#include "third_party/blink/renderer/core/frame/local_frame.h"

namespace blink {

namespace {

BoxShadowPaintImageGenerator::BoxShadowPaintImageGeneratorCreateFunction*
    g_create_function_bspig = nullptr;

}  // namespace

// static
void BoxShadowPaintImageGenerator::Init(
    BoxShadowPaintImageGeneratorCreateFunction* create_function) {
  DCHECK(!g_create_function_bspig);
  g_create_function_bspig = create_function;
}

BoxShadowPaintImageGenerator* BoxShadowPaintImageGenerator::Create(
    LocalFrame& local_root) {
  DCHECK(g_create_function_bspig);
  DCHECK(local_root.IsLocalRoot());
  return g_create_function_bspig(local_root);
}

}  // namespace blink
