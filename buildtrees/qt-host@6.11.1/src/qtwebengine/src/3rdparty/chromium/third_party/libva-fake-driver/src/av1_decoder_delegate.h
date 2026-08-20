// Copyright 2024 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AV1_DECODER_DELEGATE_H_
#define AV1_DECODER_DELEGATE_H_

#include <va/va.h>

#include <memory>

#include "context_delegate.h"

struct Dav1dContext;

namespace libvafake {

// Class used for libdav1d software decoding.
class Av1DecoderDelegate : public ContextDelegate {
 public:
  explicit Av1DecoderDelegate(VAProfile profile);
  Av1DecoderDelegate(const Av1DecoderDelegate&) = delete;
  Av1DecoderDelegate& operator=(const Av1DecoderDelegate&) = delete;
  ~Av1DecoderDelegate() override;

  // ContextDelegate implementation.
  void SetRenderTarget(const FakeSurface& surface) override;
  void EnqueueWork(const std::vector<const FakeBuffer*>& buffers) override;
  void Run() override;

 private:
  struct Dav1dContextDeleter {
    void operator()(Dav1dContext* ptr);
  };
  const FakeBuffer* encoded_data_buffer_ = nullptr;
  const FakeSurface* render_target_ = nullptr;
  std::unique_ptr<Dav1dContext, Dav1dContextDeleter> dav1d_context_;
};

}  // namespace libvafake

#endif  // AV1_DECODER_DELEGATE_H_