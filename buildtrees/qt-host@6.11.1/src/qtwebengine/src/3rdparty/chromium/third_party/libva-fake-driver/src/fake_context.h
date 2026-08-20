// Copyright 2022 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FAKE_CONTEXT_H_
#define FAKE_CONTEXT_H_

#include <va/va.h>

#include <memory>
#include <vector>

namespace libvafake {

class ContextDelegate;
class FakeSurface;
class FakeBuffer;
class FakeConfig;

// Class used for tracking a VAContext and all information relevant to it.
// All objects of this class are immutable, but three of the methods must be
// synchronized externally: BeginPicture(), RenderPicture(), and EndPicture().
// The other methods are thread-safe and may be called concurrently with any of
// those three methods.
class FakeContext {
 public:
  using IdType = VAContextID;

  // Note: |config| must outlive the FakeContext.
  FakeContext(IdType id,
              const FakeConfig& config,
              int picture_width,
              int picture_height,
              int flag,
              std::vector<VASurfaceID> render_targets);
  FakeContext(const FakeContext&) = delete;
  FakeContext& operator=(const FakeContext&) = delete;
  ~FakeContext();

  IdType GetID() const;
  const FakeConfig& GetConfig() const;
  int GetPictureWidth() const;
  int GetPictureHeight() const;
  int GetFlag() const;
  const std::vector<VASurfaceID>& GetRenderTargets() const;

  void BeginPicture(const FakeSurface& surface) const;
  void RenderPicture(const std::vector<const FakeBuffer*>& buffers) const;
  void EndPicture() const;

 private:
  const IdType id_;
  const FakeConfig& config_;

  const int picture_width_;
  const int picture_height_;
  const int flag_;
  const std::vector<VASurfaceID> render_targets_;
  const std::unique_ptr<ContextDelegate> delegate_;
};

}  // namespace libvafake

#endif  // FAKE_CONTEXT_H_
