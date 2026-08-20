// Copyright 2024 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef H264_DECODER_DELEGATE_H_
#define H264_DECODER_DELEGATE_H_

#include <va/va.h>

#include "base/lru_cache.h"
#include "context_delegate.h"
#include "openh264/wels/codec_api.h"

namespace libvafake {

// Class used for H264 software decoding.
class H264DecoderDelegate : public ContextDelegate {
 public:
  explicit H264DecoderDelegate(int picture_width_hint,
                               int picture_height_hint,
                               VAProfile profile);
  H264DecoderDelegate(const H264DecoderDelegate&) = delete;
  H264DecoderDelegate& operator=(const H264DecoderDelegate&) = delete;
  ~H264DecoderDelegate() override;

  // ContextDelegate implementation.
  void SetRenderTarget(const FakeSurface& surface) override;
  void EnqueueWork(const std::vector<const FakeBuffer*>& buffers) override;
  void Run() override;

 private:
  void OnFrameReady(unsigned char* pData[3], SBufferInfo* pDstInfo);

  ISVCDecoder* svc_decoder_ = nullptr;
  const VAProfile profile_;

  std::vector<const FakeBuffer*> slice_data_buffers_;
  std::vector<const FakeBuffer*> slice_param_buffers_;

  const FakeSurface* render_target_{nullptr};
  const FakeBuffer* pic_param_buffer_{nullptr};
  const FakeBuffer* matrix_buffer_{nullptr};

  uint32_t current_ts_ = 0;
  base::LRUCache<uint32_t, const FakeSurface*> ts_to_render_target_;
};

}  // namespace libvafake

#endif  // H264_DECODER_DELEGATE_H_