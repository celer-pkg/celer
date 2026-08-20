# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
from subprocess import Popen, TimeoutExpired
from typing import TYPE_CHECKING, Final, Optional

import psutil

if TYPE_CHECKING:
  from crossbench.plt.base import Platform, ProcessLike
  from crossbench.plt.signals import Signals

PROCESS_NOT_FOUND_EXCEPTIONS: Final = (psutil.NoSuchProcess,
                                       psutil.AccessDenied,
                                       psutil.ZombieProcess, ProcessLookupError)


def terminate_gracefully(platform: Platform,
                         process: ProcessLike,
                         timeout: int = 1,
                         signal: Optional[Signals] = None) -> None:
  """Graceful process termination
    1. Send the provided signal or SIGTERM by default
    2. Wait for the process to terminate
    3. Kill the process
  """
  if not signal:
    signal = platform.signals.SIGTERM

  try:
    platform.send_signal(process, signal)
    # TODO(392938079): support timeout on more process types
    if isinstance(process, Popen):
      process.wait(timeout)
    return
  except TimeoutExpired as e:
    logging.debug("Got timeout while waiting "
                  "for process shutdown (%s): %s", process, e)
  except PROCESS_NOT_FOUND_EXCEPTIONS as e:  # pylint: disable=broad-except
    logging.debug("Ignoring exception during process termination: %s", e)
  finally:
    try:
      platform.kill(process)
    except PROCESS_NOT_FOUND_EXCEPTIONS:
      pass
