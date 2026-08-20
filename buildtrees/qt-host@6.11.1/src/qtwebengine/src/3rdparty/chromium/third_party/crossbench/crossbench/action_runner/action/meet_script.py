# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.action_runner.action.action import ACTION_TIMEOUT, ActionT
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.bond import BondAction
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  import datetime as dt

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run


# This action is different from the `JsAction` in that it is not executed on the
# client side, but rather on the server side. This allows for more complex
# interactions with the Meet API, such as directing bots to present, or pin a
# participant.
class MeetScriptAction(BondAction):
  TYPE: ActionType = ActionType.MEET_SCRIPT

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "script", type=ObjectParser.non_empty_str, required=True)
    return parser

  def __init__(self,
               script: str,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._script = script
    super().__init__(timeout, index)

  @property
  def script(self) -> str:
    return self._script

  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.bond.meet_script(run, self)
