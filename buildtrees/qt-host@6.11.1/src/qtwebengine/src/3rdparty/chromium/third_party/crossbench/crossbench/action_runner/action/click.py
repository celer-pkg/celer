# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import functools
from typing import TYPE_CHECKING, Optional, Type

from typing_extensions import override

from crossbench.action_runner.action.action import ACTION_TIMEOUT, ActionT
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.base_input_source import InputSourceAction
from crossbench.action_runner.action.position import (PositionConfig,
                                                      SelectorConfig)
from crossbench.benchmarks.loading.input_source import InputSource
from crossbench.parse import DurationParser, NumberParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class ClickAction(InputSourceAction):
  TYPE: ActionType = ActionType.CLICK

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "position",
        aliases=("pos", "selector"),
        type=PositionConfig,
        required=True)
    parser.add_argument(
        "duration",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta())
    parser.add_argument("attempts", type=NumberParser.positive_int, default=1)
    parser.add_argument("verify", type=ObjectParser.non_empty_str)

    return parser

  def __init__(self,
               source: InputSource,
               position: PositionConfig,
               attempts: int = 1,
               duration: dt.timedelta = dt.timedelta(),
               verify: Optional[str] = None,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._position = position
    self._attempts = attempts
    self._verify = verify
    super().__init__(source, duration, timeout, index)

  @property
  def position(self) -> PositionConfig:
    return self._position

  @property
  def selector(self) -> SelectorConfig:
    if selector := self._position.selector:
      return selector
    raise ValueError(f"{self.position} has no selector")

  @property
  def attempts(self) -> int:
    return self._attempts

  @property
  def verify(self) -> Optional[str]:
    return self._verify

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.click(run, self)

  @override
  def validate(self) -> None:
    super().validate()

    if self._input_source is InputSource.JS and self.position.coordinates:
      raise ValueError("X,Y Coordinates cannot be used with JS click source.")

    if self._attempts != 1:
      if not self.position.selector:
        raise ValueError(
            "multiple attempts can only be used with a selector position.")
      if not self.position.selector.required:
        raise ValueError("non-required clicks can not have multiple attempts.")

  @override
  def validate_duration(self) -> None:
    # A click action is allowed to have a zero duration.
    return

  @override
  def supported_input_sources(self) -> tuple[InputSource, ...]:
    return (InputSource.JS, InputSource.TOUCH, InputSource.MOUSE)

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["position"] = self._position.to_json()
    if self._verify:
      details["verify"] = self._verify
    details["attempts"] = self._attempts
    return details
