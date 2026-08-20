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
from crossbench.benchmarks.loading.input_source import InputSource
from crossbench.parse import DurationParser, NumberParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class ScrollAction(InputSourceAction):
  TYPE: ActionType = ActionType.SCROLL

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument("distance", type=NumberParser.any_float, default=500)
    parser.add_argument(
        "duration",
        type=DurationParser.positive_duration,
        default=dt.timedelta(seconds=1))
    parser.add_argument("selector", type=ObjectParser.non_empty_str)
    parser.add_argument("required", type=ObjectParser.bool, default=False)
    return parser

  def __init__(self,
               source: InputSource,
               distance: float = 500.0,
               duration: dt.timedelta = dt.timedelta(seconds=1),
               selector: Optional[str] = None,
               required: bool = False,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._distance = distance

    # TODO: convert to custom selector object.
    self._selector = selector
    self._required = required
    super().__init__(source, duration, timeout, index)

  @property
  def distance(self) -> float:
    return self._distance

  @property
  def selector(self) -> Optional[str]:
    return self._selector

  @property
  def required(self) -> bool:
    return self._required

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.scroll(run, self)

  @override
  def validate(self) -> None:
    super().validate()
    if not self.distance:
      raise ValueError(f"{self}.distance is not provided")

    if self.required and not self.selector:
      raise ValueError(
          "'required' can only be used when a selector is specified")

  @override
  def supported_input_sources(self) -> tuple[InputSource, ...]:
    return (InputSource.JS, InputSource.TOUCH)

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["distance"] = str(self.distance)
    return details
