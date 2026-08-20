// Copyright 2023 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "no_op_context_delegate.h"

namespace libvafake {

void NoOpContextDelegate::SetRenderTarget(const FakeSurface& surface) {}

void NoOpContextDelegate::EnqueueWork(
    const std::vector<const FakeBuffer*>& buffers) {}

void NoOpContextDelegate::Run() {}

}  // namespace libvafake