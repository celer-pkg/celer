// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_SERVICE_DISPLAY_EMBEDDER_SKIA_OUTPUT_DEVICE_BUFFER_QUEUE_H_
#define COMPONENTS_VIZ_SERVICE_DISPLAY_EMBEDDER_SKIA_OUTPUT_DEVICE_BUFFER_QUEUE_H_

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "base/cancelable_callback.h"
#if defined(__GNUC__) && __GNUC__ < 11
#include "base/containers/flat_set.h"
#endif
#include "base/memory/raw_ptr.h"
#include "base/time/default_tick_clock.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "components/viz/service/display_embedder/output_presenter.h"
#include "components/viz/service/display_embedder/skia_output_device.h"
#include "components/viz/service/viz_service_export.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/config/gpu_driver_bug_workarounds.h"

namespace gpu {
class SharedImageRepresentationFactory;
}

namespace viz {

class SkiaOutputSurfaceDependency;

class VIZ_SERVICE_EXPORT SkiaOutputDeviceBufferQueue : public SkiaOutputDevice {
 public:
  class OverlayData {
   public:
#if defined(__GNUC__) && __GNUC__ < 11
    OverlayData() = default;
#else
    OverlayData() = delete;
    OverlayData(OverlayData&& other) = delete;
#endif
    OverlayData(
        std::unique_ptr<gpu::OverlayImageRepresentation> representation,
        std::unique_ptr<gpu::OverlayImageRepresentation::ScopedReadAccess>
            scoped_read_access,
        bool is_root_render_pass)
        : representation_(std::move(representation)),
          scoped_read_access_(std::move(scoped_read_access)),
#if defined(__GNUC__) && __GNUC__ < 11
          ref_(1),
#endif
          is_root_render_pass_(is_root_render_pass) {
      DCHECK(representation_);
      DCHECK(scoped_read_access_);
    }

#if defined(__GNUC__) && __GNUC__ < 11
    OverlayData(OverlayData&& other) { *this = std::move(other); }

    ~OverlayData() { Reset(); }

    OverlayData& operator=(OverlayData&& other) {
      DCHECK(!IsInUseByWindowServer());
      DCHECK(!ref_);
      DCHECK(!scoped_read_access_);
      DCHECK(!representation_);
      scoped_read_access_ = std::move(other.scoped_read_access_);
      representation_ = std::move(other.representation_);
      ref_ = other.ref_;
      other.ref_ = 0;
      is_root_render_pass_ = other.is_root_render_pass_;
      return *this;
    }
#else
    ~OverlayData() = default;

    OverlayData& operator=(OverlayData&& other) = delete;
#endif
    bool IsInUseByWindowServer() const {
#if BUILDFLAG(IS_APPLE)
      if (!scoped_read_access_) {
        return false;
      }
      // The root render pass buffers are managed by SkiaRenderer so we don't
      // care
      // if they're in use by the window server.
      if (is_root_render_pass_) {
        return false;
      }
      return scoped_read_access_->IsInUseByWindowServer();
#else
      return false;
#endif
    }

    void Ref() const { ++ref_; }
    void Unref() const {
#if defined(__GNUC__) && __GNUC__ < 11
      DCHECK_GT(ref_, 0);
      if (ref_ > 1) {
        --ref_;
      } else if (ref_ == 1) {
        DCHECK(!IsInUseByWindowServer());
        Reset();
      }
#else
      // Unref should only be called when there is more than one reference.
      DCHECK_GT(ref_, 1);
      --ref_;
#endif
    }
    void OnReuse()
        const { /*
     // This is a proxy check for single-buffered overlay.
          if (representation_->usage().Has(
                     gpu::SHARED_IMAGE_USAGE_CONCURRENT_READ_WRITE) &&
                             base::FeatureList::IsEnabled(
                                         kRestartReadAccessForConcurrentReadWrite))
     {
                                                // If this is a single-buffered
     overlay, want to restart read access to
                                                      // pick up any new write
     fences for this frame. scoped_read_access_.reset(); scoped_read_access_ =
     representation_->BeginScopedReadAccess();
                                                                      }
                                                                      */
    }

    void OnContextLost() const { representation_->OnContextLost(); }

    bool unique() const { return ref_ == 1; }
    const gpu::Mailbox& mailbox() const { return representation_->mailbox(); }
    gpu::OverlayImageRepresentation::ScopedReadAccess* scoped_read_access()
        const {
      return scoped_read_access_.get();
    }

    bool IsRootRenderPass() const { return is_root_render_pass_; }

   private:
#if defined(__GNUC__) && __GNUC__ < 11
    void Reset() const {
      scoped_read_access_.reset();
      representation_.reset();
      ref_ = 0;
    }
#endif
    mutable std::unique_ptr<gpu::OverlayImageRepresentation> representation_;
    mutable std::unique_ptr<gpu::OverlayImageRepresentation::ScopedReadAccess>
        scoped_read_access_;
    mutable int ref_ = 1;
    mutable bool is_root_render_pass_ = false;
  };

  SkiaOutputDeviceBufferQueue(
      std::unique_ptr<OutputPresenter> presenter,
      SkiaOutputSurfaceDependency* deps,
      gpu::SharedImageRepresentationFactory* representation_factory,
      scoped_refptr<gpu::MemoryTracker> memory_tracker,
      const DidSwapBufferCompleteCallback& did_swap_buffer_complete_callback,
      const ReleaseOverlaysCallback& release_overlays_callback);

  ~SkiaOutputDeviceBufferQueue() override;

  SkiaOutputDeviceBufferQueue(const SkiaOutputDeviceBufferQueue&) = delete;
  SkiaOutputDeviceBufferQueue& operator=(const SkiaOutputDeviceBufferQueue&) =
      delete;

  // SkiaOutputDevice overrides.
  void Present(const std::optional<gfx::Rect>& update_rect,
               BufferPresentedCallback feedback,
               OutputSurfaceFrame frame) override;
  bool Reshape(const ReshapeParams& params) override;
  void SetViewportSize(const gfx::Size& viewport_size) override;
  SkSurface* BeginPaint(
      std::vector<GrBackendSemaphore>* end_semaphores) override;
  void EndPaint() override;

  void ScheduleOverlays(SkiaOutputSurface::OverlayList overlays) override;

  // SkiaOutputDevice override
  void SetVSyncDisplayID(int64_t display_id) override;

  base::OneShotTimer& OverlaysReclaimTimerForTesting() {
    return reclaim_overlays_timer_;
  }
  void SetSwapTimeClockForTesting(base::TickClock* clock) {
    swap_time_clock_ = clock;
  }

 private:
  friend class SkiaOutputDeviceBufferQueueTest;

  // Used as callback for SwapBuffersAsync and PostSubBufferAsync to finish
  // operation
  void DoFinishSwapBuffers(const gfx::Size& size,
                           OutputSurfaceFrame frame,
                           std::vector<gpu::Mailbox> overlay_mailboxes,
                           gfx::SwapCompletionResult result);
  void PostReleaseOverlays();
  void ReleaseOverlays();

  gfx::Size GetSwapBuffersSize();

  // Given an overlay mailbox, returns the corresponding OverlayData* from
  // |overlays_|. Inserts an OverlayData if mailbox is not in |overlays_|.
  const OverlayData* GetOrCreateOverlayData(const gpu::Mailbox& mailbox,
                                            bool is_root_render_pass,
                                            bool* is_existing = nullptr);

  std::unique_ptr<OutputPresenter> presenter_;
  const gpu::GpuDriverBugWorkarounds workarounds_;

  scoped_refptr<gpu::SharedContextState> context_state_;
  const raw_ptr<gpu::SharedImageRepresentationFactory> representation_factory_;
  // Format of images
  gfx::ColorSpace color_space_;
  gfx::Size image_size_;
  gfx::Size viewport_size_;
  gfx::OverlayTransform overlay_transform_ = gfx::OVERLAY_TRANSFORM_NONE;

  // Mailboxes of scheduled overlays for the next SwapBuffers call.
  std::vector<gpu::Mailbox> pending_overlay_mailboxes_;
  // Mailboxes of committed overlays for the last SwapBuffers call.
  std::vector<gpu::Mailbox> committed_overlay_mailboxes_;

  // Heterogeneous lookup for unordered set is nice feature sadly only from
  // gcc-11 upwards
#if defined(__GNUC__) && __GNUC__ < 11
  class OverlayDataComparator {
   public:
    using is_transparent = void;
    bool operator()(const OverlayData& lhs, const OverlayData& rhs) const;
    bool operator()(const OverlayData& lhs, const gpu::Mailbox& rhs) const;
    bool operator()(const gpu::Mailbox& lhs, const OverlayData& rhs) const;
  };
  base::flat_set<OverlayData, OverlayDataComparator> overlays_;
#else
  struct OverlayDataHash {
    using is_transparent = void;
    std::size_t operator()(const OverlayData& o) const;
    std::size_t operator()(const gpu::Mailbox& m) const;
  };

  struct OverlayDataKeyEqual {
    using is_transparent = void;
    bool operator()(const OverlayData& lhs, const OverlayData& rhs) const;
    bool operator()(const OverlayData& lhs, const gpu::Mailbox& rhs) const;
    bool operator()(const gpu::Mailbox& lhs, const OverlayData& rhs) const;
  };

  // A set for all overlays. The set uses overlay_data.mailbox() as the unique
  // key.
  std::unordered_set<OverlayData, OverlayDataHash, OverlayDataKeyEqual>
      overlays_;
#endif
  bool has_overlays_scheduled_but_swap_not_finished_ = false;
  raw_ptr<const base::TickClock> swap_time_clock_ =
      base::DefaultTickClock::GetInstance();
  base::TimeTicks last_swap_time_;
  base::OneShotTimer reclaim_overlays_timer_;
  static constexpr base::TimeDelta kDelayForOverlaysReclaim = base::Seconds(1);

  base::WeakPtrFactory<SkiaOutputDeviceBufferQueue> weak_ptr_{this};
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_SERVICE_DISPLAY_EMBEDDER_SKIA_OUTPUT_DEVICE_BUFFER_QUEUE_H_
