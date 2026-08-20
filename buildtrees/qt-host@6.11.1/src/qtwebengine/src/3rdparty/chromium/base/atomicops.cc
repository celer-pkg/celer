// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/atomicops.h"

#include <atomic>

#include "base/memory/aligned_memory.h"

namespace base::subtle {

#if __cpp_lib_atomic_ref >= 201806L
template<typename T>
static bool RelaxedAtomicSubWrite(uint8_t*& dst_byte_ptr, const uint8_t*& src_byte_ptr, size_t &bytes)
{
  if (std::atomic_ref<T>::is_always_lock_free) {
    if (bytes >= sizeof(T) && IsAligned(dst_byte_ptr, std::atomic_ref<T>::required_alignment)) {
      std::atomic_ref<T>(*reinterpret_cast<T*>(dst_byte_ptr))
          .store(*reinterpret_cast<const T*>(src_byte_ptr), std::memory_order_relaxed);
      UNSAFE_BUFFERS(dst_byte_ptr+=sizeof(T));
      UNSAFE_BUFFERS(src_byte_ptr+=sizeof(T));
      bytes-=sizeof(T);
      return true;
    }
  }
  return false;
}

// Non-quite atomic memcpy, that at least writes as large chunks as we can atomically, so a non-atomic reader is almost guarenteed atomic field reads.
void RelaxedAtomicWriteMemcpy(base::span<uint8_t> dst,
                              base::span<const uint8_t> src) {
  CHECK_EQ(dst.size(), src.size());
  size_t bytes = dst.size();
  uint8_t* dst_byte_ptr = dst.data();
  const uint8_t* src_byte_ptr = src.data();
  // Alignment for uintptr_t atomics that we use in the ideal case.
  constexpr size_t kDesiredAlignment =
      std::atomic_ref<uintptr_t>::required_alignment;
  // Assert this function even does anything useful
  static_assert(std::atomic_ref<uintptr_t>::is_always_lock_free);

  // Copy up until `dst_byte_ptr` is properly aligned for the ideal case.
  while (bytes > 0 && !IsAligned(dst_byte_ptr, kDesiredAlignment)) {
    if constexpr(!std::same_as<uintptr_t, uint32_t>) {
      if (RelaxedAtomicSubWrite<uint32_t>(dst_byte_ptr, src_byte_ptr, bytes))
        continue;
    }
    if (RelaxedAtomicSubWrite<uint16_t>(dst_byte_ptr, src_byte_ptr, bytes))
        continue;

    if constexpr(std::atomic_ref<uint8_t>::is_always_lock_free) {
      std::atomic_ref<uint8_t>(*dst_byte_ptr)
          .store(*src_byte_ptr, std::memory_order_relaxed);
    } else {
      // No atomic byte writes without locks, and since the readers do not use atomic and thus no locks; this is the best we can do.
      *dst_byte_ptr = *src_byte_ptr;
    }
    // SAFETY: We check above that `dst_byte_ptr` and `src_byte_ptr` point
    // to spans of sufficient size.
    UNSAFE_BUFFERS(++dst_byte_ptr);
    UNSAFE_BUFFERS(++src_byte_ptr);
    --bytes;
  }

  // Ideal case where both `src_byte_ptr` and `dst_byte_ptr` are both properly
  // aligned and the largest possible atomic is used for copying.
  if (IsAligned(src_byte_ptr, kDesiredAlignment)) {
    while (bytes >= sizeof(uintptr_t)) {
      std::atomic_ref<uintptr_t>(*reinterpret_cast<uintptr_t*>(dst_byte_ptr))
          .store(*reinterpret_cast<const uintptr_t*>(src_byte_ptr),
                std::memory_order_relaxed);
      // SAFETY: We check above that `dst_byte_ptr` and `src_byte_ptr` point
      // to spans of sufficient size.
      UNSAFE_BUFFERS(dst_byte_ptr += sizeof(uintptr_t));
      UNSAFE_BUFFERS(src_byte_ptr += sizeof(uintptr_t));
      bytes -= sizeof(uintptr_t);
    }
  }

  // Copy what's left after the happy-case byte-by-byte.
  while (bytes > 0) {
    if constexpr(!std::same_as<uintptr_t, uint32_t>) {
      if (RelaxedAtomicSubWrite<uint32_t>(dst_byte_ptr, src_byte_ptr, bytes))
        continue;
    }
    if (RelaxedAtomicSubWrite<uint16_t>(dst_byte_ptr, src_byte_ptr, bytes))
        continue;

    if constexpr(!std::atomic_ref<uint8_t>::is_always_lock_free) {
      // No atomic byte writes without locks, and since the readers do not use atomic and thus no locks; this is the best we can do.
      *dst_byte_ptr = *src_byte_ptr;
    } else {
      std::atomic_ref<uint8_t>(*dst_byte_ptr)
          .store(*src_byte_ptr, std::memory_order_relaxed);
    }
    // SAFETY: We check above that `dst_byte_ptr` and `src_byte_ptr` point
    // to spans of sufficient size.
    UNSAFE_BUFFERS(++dst_byte_ptr);
    UNSAFE_BUFFERS(++src_byte_ptr);
    --bytes;
  }
}
#else
void RelaxedAtomicWriteMemcpy(base::span<uint8_t> dst,
                              base::span<const uint8_t> src) {
  CHECK_EQ(dst.size(), src.size());
  size_t bytes = dst.size();
  uint8_t* dst_byte_ptr = dst.data();
  const uint8_t* src_byte_ptr = src.data();
  // Type-punning atomic types is also UB, so stay with the old simple UB.
  ::memcpy(dst_byte_ptr, src_byte_ptr, bytes);
}
#endif

}  // namespace base::subtle
