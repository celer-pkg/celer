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
from crossbench.parse import DurationParser

if TYPE_CHECKING:
  import datetime as dt

  from crossbench.config import ConfigParser
  from crossbench.types import JsonDict


class BaseDurationAction(Action):

  def __init__(self,
               duration: dt.timedelta,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._duration: dt.timedelta = duration
    super().__init__(timeout, index)

  @property
  @override
  def duration(self) -> dt.timedelta:
    return self._duration

  @override
  def validate(self) -> None:
    super().validate()
    self.validate_duration()

  def validate_duration(self) -> None:
    if self.duration.total_seconds() <= 0:
      raise ValueError(
          f"{self}.duration should be positive, but got {self.duration}")

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["duration"] = self.duration.total_seconds()
    return details


class DurationAction(BaseDurationAction):
  TYPE: ActionType = ActionType.WAIT

  @classmethod
  @override
  @functools.cache
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "duration", type=DurationParser.positive_duration, required=True)
    return parser
