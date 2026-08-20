# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
from typing import TYPE_CHECKING, Optional, Type

from typing_extensions import override

from crossbench.action_runner.action.action import ACTION_TIMEOUT, ActionT
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.bond import BondAction
from crossbench.action_runner.action.enums import WindowTarget
from crossbench.bond.bond import AddBotsConfig

if TYPE_CHECKING:
  import datetime as dt

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run


class MeetCreateAction(BondAction):
  TYPE: ActionType = ActionType.MEET_CREATE

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument("bots", type=AddBotsConfig)
    parser.add_argument(
        "target", type=WindowTarget.parse, default=WindowTarget.SELF)
    return parser

  def __init__(self,
               bots: Optional[AddBotsConfig] = None,
               target: WindowTarget = WindowTarget.SELF,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._bots = bots
    self._target = target
    super().__init__(timeout, index)

  @property
  def bots(self) -> Optional[AddBotsConfig]:
    return self._bots

  @property
  def target(self) -> WindowTarget:
    return self._target

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.bond.meet_create(run, self)
