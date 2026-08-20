// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_GAMEPAD_PUBLIC_MOJOM_GAMEPAD_HARDWARE_BUFFER_H_
#define DEVICE_GAMEPAD_PUBLIC_MOJOM_GAMEPAD_HARDWARE_BUFFER_H_

#include "base/memory/shared_memory_safety_checker.h"
#include "device/base/synchronization/shared_memory_seqlock_buffer.h"
#include "device/gamepad/public/cpp/gamepads.h"

namespace device {

using GamepadHardwareBuffer = SharedMemorySeqLockBuffer<Gamepads>;

}  // namespace device

#if defined(COMPILER_MSVC) && !defined(__clang__)
SKIP_SHARED_MEMORY_SAFETY_CHECK_FOR(device::GamepadHardwareBuffer)
#else
// GamepadHardwareBuffer is used in shared memory, so it must be trivially
// copyable.
static_assert(std::is_trivially_copyable_v<device::GamepadHardwareBuffer>);
#endif  // defined(COMPILER_MSVC) && !defined(__clang__)

#endif  // DEVICE_GAMEPAD_PUBLIC_MOJOM_GAMEPAD_HARDWARE_BUFFER_H_
