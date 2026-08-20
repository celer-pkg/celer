# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import sys
from typing import Final

from crossbench.plt.arch import MachineArch
from crossbench.plt.base import Platform, SubprocessError
from crossbench.plt.linux import LinuxPlatform
from crossbench.plt.linux_ssh import LinuxSshPlatform
from crossbench.plt.macos import MacOSPlatform
from crossbench.plt.win import WinPlatform


def _get_default() -> Platform:
  if sys.platform == "linux":
    return LinuxPlatform()
  if sys.platform == "darwin":
    return MacOSPlatform()
  if sys.platform == "win32":
    return WinPlatform()
  raise NotImplementedError(f"Unsupported platform: {sys.platform}")


PLATFORM: Final[Platform] = _get_default()

__all__ = [
    "LinuxPlatform",
    "LinuxSshPlatform",
    "MachineArch",
    "MacOSPlatform",
    "Platform",
    "SubprocessError",
    "WinPlatform",
]
