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
from crossbench.parse import NumberParser, ObjectParser

if TYPE_CHECKING:
  import datetime as dt

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class WaitForElementAction(Action):
  TYPE: ActionType = ActionType.WAIT_FOR_ELEMENT

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "selector", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "expected_count",
        type=NumberParser.positive_int,
        required=False,
        default=1)
    parser.add_argument("or_more", type=bool, required=False, default=False)
    return parser

  def __init__(self,
               selector: str,
               expected_count: int,
               or_more: bool,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._selector = selector
    self._expected_count = expected_count
    self._or_more = or_more
    super().__init__(timeout, index)

  @property
  def selector(self) -> str:
    return self._selector

  @property
  def expected_count(self) -> int:
    return self._expected_count

  @property
  def or_more(self) -> bool:
    return self._or_more

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.wait_for_element(run, self)

  @override
  def validate(self) -> None:
    super().validate()
    if not self.selector:
      raise ValueError(f"{self}.selector is missing.")
    NumberParser.positive_int(self.expected_count, "expected_count")

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["selector"] = self.selector
    details["expected_count"] = self.expected_count
    details["or_more"] = self.or_more
    return details
