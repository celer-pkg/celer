# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Optional

from crossbench.cli.config.env import EnvConfig, ValidationMode
from crossbench.env.base import BaseEnv

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.runner.run import Run


class RunEnv(BaseEnv):

  def __init__(self,
               run: Run,
               config: Optional[EnvConfig] = None,
               validation_mode: ValidationMode = ValidationMode.THROW) -> None:
    self._run: Run = run
    self._browser: Browser = run.browser
    super().__init__(self._browser.platform, config, validation_mode)

  def validate(self) -> None:
    pass
