# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from subprocess import Popen

  from crossbench import plt


class WakeLock:
  """
  Prevent the system from going to sleep while running the benchmark.
  """

  def __init__(self, platform: plt.Platform) -> None:
    self._process: Popen | None = None
    self._platform = platform

  def __enter__(self) -> None:
    if self._platform.is_macos:
      self._process = self._platform.popen("caffeinate", "-imdsu")
      atexit.register(self.stop_process)
    # TODO: Add linux support

  def __exit__(self, exc_type, exc_value, exc_traceback) -> None:
    self.stop_process()

  def stop_process(self) -> None:
    atexit.unregister(self.stop_process)
    if self._process:
      self._process.kill()
      self._process = None
