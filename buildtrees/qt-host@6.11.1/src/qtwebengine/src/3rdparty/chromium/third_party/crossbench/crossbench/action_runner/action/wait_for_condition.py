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

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class WaitForConditionAction(Action):
  TYPE: ActionType = ActionType.WAIT_FOR_CONDITION

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "condition", type=ObjectParser.non_empty_str, required=True)
    return parser

  def __init__(self,
               condition: str,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._condition = condition
    super().__init__(timeout, index)

  @property
  def condition(self) -> str:
    return self._condition

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.wait_for_condition(run, self)

  @override
  def validate(self) -> None:
    super().validate()
    if not self.condition:
      raise ValueError(f"{self}.condition is missing.")
    if "return" not in self.condition:
      raise ValueError(
          f"Missing return statement in condition: {self.condition}")

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["condition"] = self.condition
    return details
