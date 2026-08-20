// Copyright 2022 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fake_context.h"

#include "av1_decoder_delegate.h"
#include "base/logging.h"
#include "fake_config.h"
#include "h264_decoder_delegate.h"
#include "no_op_context_delegate.h"
#include "vpx_decoder_delegate.h"

namespace {

std::unique_ptr<libvafake::ContextDelegate> CreateDelegate(
    const libvafake::FakeConfig& config,
    int picture_width,
    int picture_height) {
  const char* use_no_op_context_delegate_env_var =
      getenv("USE_NO_OP_CONTEXT_DELEGATE");
  if (use_no_op_context_delegate_env_var &&
      strcmp(use_no_op_context_delegate_env_var, "1") == 0) {
    return std::make_unique<libvafake::NoOpContextDelegate>();
  }

  if (config.GetEntrypoint() != VAEntrypointVLD) {
    return nullptr;
  }

  switch (config.GetProfile()) {
    case VAProfileVP8Version0_3:
    case VAProfileVP9Profile0:
      return std::make_unique<libvafake::VpxDecoderDelegate>(
          picture_width, picture_height, config.GetProfile());
    case VAProfileAV1Profile0:
      return std::make_unique<libvafake::Av1DecoderDelegate>(
          config.GetProfile());
    case VAProfileH264ConstrainedBaseline:
    case VAProfileH264Main:
      return std::make_unique<libvafake::H264DecoderDelegate>(
          picture_width, picture_height, config.GetProfile());
    default:
      break;
  }

  return nullptr;
}

}  // namespace

namespace libvafake {

FakeContext::FakeContext(FakeContext::IdType id,
                         const FakeConfig& config,
                         int picture_width,
                         int picture_height,
                         int flag,
                         std::vector<VASurfaceID> render_targets)
    : id_(id),
      config_(config),
      picture_width_(picture_width),
      picture_height_(picture_height),
      flag_(flag),
      render_targets_(std::move(render_targets)),
      delegate_(CreateDelegate(config_, picture_width_, picture_height_)) {}
FakeContext::~FakeContext() = default;

FakeContext::IdType FakeContext::GetID() const {
  return id_;
}

const FakeConfig& FakeContext::GetConfig() const {
  return config_;
}

int FakeContext::GetPictureWidth() const {
  return picture_width_;
}

int FakeContext::GetPictureHeight() const {
  return picture_height_;
}

int FakeContext::GetFlag() const {
  return flag_;
}

const std::vector<VASurfaceID>& FakeContext::GetRenderTargets() const {
  return render_targets_;
}

void FakeContext::BeginPicture(const FakeSurface& surface) const {
  CHECK(delegate_);
  delegate_->SetRenderTarget(surface);
}

void FakeContext::RenderPicture(
    const std::vector<const FakeBuffer*>& buffers) const {
  CHECK(delegate_);
  delegate_->EnqueueWork(buffers);
}

void FakeContext::EndPicture() const {
  CHECK(delegate_);
  delegate_->Run();
}

}  // namespace libvafake
