# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import functools
from typing import TYPE_CHECKING, Self, Type

from typing_extensions import override

from crossbench.action_runner.action.action import ACTION_TIMEOUT, ActionT
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.base_duration import BaseDurationAction
from crossbench.action_runner.action.enums import ReadyState, WindowTarget
from crossbench.parse import DurationParser, ObjectParser

if TYPE_CHECKING:
  from urllib.parse import ParseResult

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict

class GetAction(BaseDurationAction):
  TYPE: ActionType = ActionType.GET

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    return cls(url=ObjectParser.fuzzy_url_str(value))

  @classmethod
  @override
  def parse_any_url(cls, url: ParseResult, **kwargs) -> Self:
    cls.expect_no_extra_kwargs(kwargs)
    return cls(url=url.geturl())

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument("url", type=ObjectParser.fuzzy_url_str, required=True)
    parser.add_argument(
        "duration",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta())
    parser.add_argument(
        "ready_state", type=ReadyState.parse, default=ReadyState.ANY)
    parser.add_argument(
        "target", type=WindowTarget.parse, default=WindowTarget.SELF)
    return parser

  def __init__(self,
               url: str,
               duration: dt.timedelta = dt.timedelta(),
               timeout: dt.timedelta = ACTION_TIMEOUT,
               ready_state: ReadyState = ReadyState.ANY,
               target: WindowTarget = WindowTarget.SELF,
               index: int = 0) -> None:
    if not url:
      raise ValueError(f"{self}.url is missing")
    self._url: str = url
    self._ready_state = ready_state
    self._target = target
    super().__init__(duration, timeout, index)

  @override
  def validate_duration(self) -> None:
    if self.ready_state != ReadyState.ANY:
      if self.duration != dt.timedelta():
        raise ValueError(
            f"Expected empty duration with ReadyState {self.ready_state} "
            f"but got: {self.duration}")
      self._duration = dt.timedelta()

  @property
  def url(self) -> str:
    return self._url

  @property
  def ready_state(self) -> ReadyState:
    return self._ready_state

  @property
  @override
  def duration(self) -> dt.timedelta:
    return self._duration

  @property
  def target(self) -> WindowTarget:
    return self._target

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.get(run, self)

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["url"] = self.url
    details["ready_state"] = str(self.ready_state)
    details["target"] = str(self.target)
    return details
