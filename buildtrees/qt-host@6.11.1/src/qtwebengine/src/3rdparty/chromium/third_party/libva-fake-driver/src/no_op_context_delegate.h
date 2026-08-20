// Copyright 2023 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NO_OP_CONTEXT_DELEGATE_H_
#define NO_OP_CONTEXT_DELEGATE_H_

#include "context_delegate.h"

namespace libvafake {

// A ContextDelegate that doesn't do anything. Intended as a fake
// ContextDelegate for unit testing purposes.
class NoOpContextDelegate : public ContextDelegate {
 public:
  NoOpContextDelegate() = default;
  NoOpContextDelegate(const NoOpContextDelegate&) = delete;
  NoOpContextDelegate& operator=(const NoOpContextDelegate&) = delete;
  ~NoOpContextDelegate() override = default;

  // ContextDelegate implementation.
  void SetRenderTarget(const FakeSurface& surface) override;
  void EnqueueWork(const std::vector<const FakeBuffer*>& buffers) override;
  void Run() override;
};

}  // namespace libvafake

#endif  // NO_OP_CONTEXT_DELEGATE_H_