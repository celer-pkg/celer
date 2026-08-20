# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.action_runner.action.action import (ACTION_TIMEOUT, Action,
                                                    ActionT)
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.enums import ReadyState

if TYPE_CHECKING:
  import datetime as dt

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class WaitForReadyStateAction(Action):
  TYPE: ActionType = ActionType.WAIT_FOR_READY_STATE

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "ready_state", type=ReadyState.parse, default=ReadyState.COMPLETE)
    return parser

  def __init__(self,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               ready_state: ReadyState = ReadyState.COMPLETE,
               index: int = 0) -> None:
    self._ready_state = ready_state
    super().__init__(timeout, index)

  @property
  def ready_state(self) -> ReadyState:
    return self._ready_state

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.wait_for_ready_state(run, self)

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["ready_state"] = str(self.ready_state)
    return details
