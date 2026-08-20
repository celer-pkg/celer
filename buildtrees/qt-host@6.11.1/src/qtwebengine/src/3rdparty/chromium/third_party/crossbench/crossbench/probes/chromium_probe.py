# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.probes.probe import Probe

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv


class ChromiumProbe(Probe):

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    self.expect_browser(browser, BrowserAttributes.CHROMIUM_BASED)

  @override
  def attach(self, browser: Browser) -> None:
    self.expect_browser(browser, BrowserAttributes.CHROMIUM_BASED)
    super().attach(browser)
