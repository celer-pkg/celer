# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.probes.probe import Probe


class ProbeMissingDataError(ValueError):
  pass


class ProbeValidationError(ValueError):

  def __init__(self, probe: Probe, message: str) -> None:
    self.probe = probe
    super().__init__(f"Probe({probe.NAME}): {message}")


class ProbeIncompatibleBrowser(ProbeValidationError):

  def __init__(self,
               probe: Probe,
               browser: Browser,
               message: str = "Incompatible browser") -> None:
    super().__init__(probe, f"{message}, got {browser.attributes()}")
