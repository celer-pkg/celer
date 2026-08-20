# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.action_runner.action.action import (ACTION_TIMEOUT, Action,
                                                    ActionT)
from crossbench.action_runner.action.action_type import ActionType
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  import datetime as dt
  import re

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run


class WaitForDownloadAction(Action):
  TYPE: ActionType = ActionType.WAIT_FOR_DOWNLOAD

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "pattern",
        type=ObjectParser.regexp,
        help="A regexp to search downloaded file names",
        required=True)
    return parser

  def __init__(self,
               pattern: re.Pattern,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._pattern = pattern
    super().__init__(timeout, index)

  @property
  def pattern(self) -> re.Pattern:
    return self._pattern

  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.wait_for_download(run, self)
