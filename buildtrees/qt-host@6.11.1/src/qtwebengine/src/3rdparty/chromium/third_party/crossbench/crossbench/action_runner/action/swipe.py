# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import functools
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.action_runner.action.action import ACTION_TIMEOUT, ActionT
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.base_duration import DurationAction
from crossbench.parse import NumberParser

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class SwipeAction(DurationAction):
  TYPE: ActionType = ActionType.SWIPE

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "start_x",
        aliases=("startx",),
        type=NumberParser.any_int,
        required=True)
    parser.add_argument(
        "start_y",
        aliases=("starty",),
        type=NumberParser.any_int,
        required=True)
    parser.add_argument(
        "end_x", aliases=("endx",), type=NumberParser.any_int, required=True)
    parser.add_argument(
        "end_y", aliases=("endy",), type=NumberParser.any_int, required=True)
    return parser

  def __init__(self,
               start_x: int,
               start_y: int,
               end_x: int,
               end_y: int,
               duration: dt.timedelta = dt.timedelta(seconds=1),
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._start_x: int = start_x
    self._start_y: int = start_y
    self._end_x: int = end_x
    self._end_y: int = end_y
    super().__init__(duration, timeout, index)

  @property
  def start_x(self) -> int:
    return self._start_x

  @property
  def start_y(self) -> int:
    return self._start_y

  @property
  def end_x(self) -> int:
    return self._end_x

  @property
  def end_y(self) -> int:
    return self._end_y

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.swipe(run, self)

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["start_x"] = self._start_x
    details["start_y"] = self._start_y
    details["end_x"] = self._end_x
    details["end_y"] = self._end_y
    return details
