// Copyright 2023 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SCOPED_BO_MAPPING_FACTORY_H_
#define SCOPED_BO_MAPPING_FACTORY_H_

#include <gbm.h>

#include <memory>
#include <mutex>

#include "base/logging.h"
#include "base/scoped_fd.h"

namespace libvafake {

struct GbmDeviceDeleter {
  void operator()(gbm_device* device);
};

using ScopedGbmDevice = std::unique_ptr<gbm_device, GbmDeviceDeleter>;

class ScopedBOMappingFactory;

// ScopedBOMapping tracks the CPU mapping of a minigbm Buffer Object (BO).
// Upon destruction, it unmaps and destroys the Buffer Object.
//
// Notes:
//
// - Only a ScopedBOMappingFactory can create valid ScopedBOMapping instances.
//   Upon destruction, the ScopedBOMapping requests the ScopedBOMappingFactory
//   to unmap and destroy the Buffer Object. This is done to ensure that the GBM
//   device is protected from concurrent operations on multiple threads.
//   Therefore, the ScopedBOMappingFactory that creates a ScopedBOMapping must
//   outlive it.
//
// - ScopedBOMapping instances can be used from any thread, but they are NOT
//   thread-safe, i.e., access to them must be synchronized externally.
//   Additionally, access to different ScopedBOMappings that refer to the same
//   dma-buf must also be synchronized externally.
class ScopedBOMapping {
 public:
  // A ScopedAccess can be used to ensure cache-coherent CPU read/write access
  // to a Buffer Object mapping. The intended usage is as follows:
  //
  // ScopedBOMapping mapping = factory.Create(import_data);
  // {
  //   const auto access = mapping.BeginAccess();
  //   /* Read/write using access.GetData() and access.GetStride() */
  // }
  //
  // ScopedAccess instances themselves are thread-safe but:
  //
  // - Concurrent reads/writes to the mapped data must be synchronized
  //   externally.
  //
  // - Different ScopedAccess instances corresponding to the same buffer object
  //   must be synchronized externally.
  //
  // Note: a ScopedBOMapping must outlive any ScopedAccess instances produced by
  // it.
  class ScopedAccess {
   public:
    // Not copyable nor movable (move ctors are deleted by default).
    ScopedAccess(const ScopedAccess&) = delete;
    ScopedAccess& operator=(const ScopedAccess&) = delete;
    ~ScopedAccess();

    uint8_t* GetData(size_t plane) const;
    uint32_t GetStride(size_t plane) const;

   private:
    // Only ScopedBOMapping should be able to create ScopedAccess instances.
    friend class ScopedBOMapping;

    explicit ScopedAccess(const ScopedBOMapping& mapping);

    const ScopedBOMapping& mapping_;
  };

  // Creates an invalid ScopedBOMapping.
  ScopedBOMapping();

  // Not copyable but movable (the copy ctors are deleted by default).
  ScopedBOMapping(ScopedBOMapping&& other);
  ScopedBOMapping& operator=(ScopedBOMapping&& other);
  ~ScopedBOMapping();

  bool IsValid() const { return !!scoped_bo_mapping_factory_; }

  explicit operator bool() const { return IsValid(); }

  ScopedAccess BeginAccess() const;

 private:
  // Contains metadata for each element of a plane retrieved from minigbm.
  struct Plane {
    Plane(uint32_t stride, void* addr, void* mmap_data, int prime_fd);
    Plane(Plane&& other);
    Plane& operator=(Plane&& other);
    ~Plane();

    uint32_t stride;
    void* addr;
    void* mmap_data;
    base::ScopedFD prime_fd;
  };
  // Needed so that GBMDeviceHolder can create ScopedBOMappings.
  friend class ScopedBOMappingFactory;

  ScopedBOMapping(ScopedBOMappingFactory* scoped_bo_mapping_factory,
                  std::vector<Plane> planes,
                  struct gbm_bo* bo_import);

  ScopedBOMappingFactory* scoped_bo_mapping_factory_;
  std::vector<Plane> planes_;
  struct gbm_bo* bo_import_;
};

// A ScopedBOMappingFactory provides thread-safe access to minigbm in order to
// import dma-bufs and map them for CPU access.
//
// ScopedBOMappingFactory instances are thread-safe.
class ScopedBOMappingFactory {
 public:
  explicit ScopedBOMappingFactory(int drm_fd);
  ScopedBOMappingFactory(const ScopedBOMappingFactory&) = delete;
  ScopedBOMappingFactory& operator=(const ScopedBOMappingFactory&) = delete;
  ~ScopedBOMappingFactory();

  // Imports and maps the dma-buf referenced by |import_data|. This method
  // always returns a valid mapping. If the dma-buf can't be imported, it
  // crashes.
  ScopedBOMapping Create(gbm_import_fd_modifier_data import_data);

 private:
  // Needed so that the ScopedBOMapping can call UnmapAndDestroyBufferObject().
  friend class ScopedBOMapping;

  // Unmaps all the |planes| of the buffer object referenced by |bo_import|.
  void UnmapAndDestroyBufferObject(std::vector<ScopedBOMapping::Plane> planes,
                                   struct gbm_bo* bo_import);

  std::mutex lock_;
  const ScopedGbmDevice gbm_device_;
};

}  // namespace libvafake

#endif  // SCOPED_BO_MAPPING_FACTORY_H_
