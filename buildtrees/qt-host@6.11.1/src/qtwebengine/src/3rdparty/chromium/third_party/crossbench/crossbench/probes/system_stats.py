# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.probes.polling import PollingProbe
from crossbench.probes.probe_error import ProbeValidationError

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv


class SystemStatsProbe(PollingProbe):
  """
  General-purpose probe to periodically collect system-wide CPU and memory
  stats on unix systems.
  """
  NAME = "system.stats"
  CMD = ("ps", "-a", "-e", "-o", "pcpu,pmem,args", "-r")
  IS_GENERAL_PURPOSE = True

  def __init__(
      self, interval: dt.timedelta = dt.timedelta(seconds=0.1)) -> None:
    super().__init__(self.CMD, interval)

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    if not (browser.platform.is_linux or browser.platform.is_macos):
      raise ProbeValidationError(self, "Only supported on macOS and linux.")
