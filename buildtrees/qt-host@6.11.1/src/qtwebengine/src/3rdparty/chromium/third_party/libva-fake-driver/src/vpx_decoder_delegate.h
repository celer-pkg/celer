// Copyright 2023 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VPX_DECODER_DELEGATE_H_
#define VPX_DECODER_DELEGATE_H_

#include <va/va.h>

#include <memory>

#include "context_delegate.h"

struct vpx_codec_ctx;

namespace libvafake {

// Class used for libvpx software decoding.
class VpxDecoderDelegate : public ContextDelegate {
 public:
  VpxDecoderDelegate(int picture_width_hint,
                     int picture_height_hint,
                     VAProfile profile);
  VpxDecoderDelegate(const VpxDecoderDelegate&) = delete;
  VpxDecoderDelegate& operator=(const VpxDecoderDelegate&) = delete;
  ~VpxDecoderDelegate() override;

  // ContextDelegate implementation.
  void SetRenderTarget(const FakeSurface& surface) override;
  void EnqueueWork(const std::vector<const FakeBuffer*>& buffers) override;
  void Run() override;

 private:
  const FakeBuffer* encoded_data_buffer_ = nullptr;
  const FakeSurface* render_target_ = nullptr;
  std::unique_ptr<vpx_codec_ctx> vpx_codec_;
};

}  // namespace libvafake

#endif  // VPX_DECODER_DELEGATE_H_