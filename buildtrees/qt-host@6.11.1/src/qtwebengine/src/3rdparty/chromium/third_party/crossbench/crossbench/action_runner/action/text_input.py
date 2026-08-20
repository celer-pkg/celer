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
from crossbench.parse import DurationParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class TextInputAction(InputSourceAction):
  TYPE: ActionType = ActionType.TEXT_INPUT

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "text",
        type=ObjectParser.non_empty_str,
        required=False)
    parser.add_argument(
        "keyevent",
        type=ObjectParser.non_empty_str,
        required=False,
        help="Keyevent code name to trigger on Android, instead of text. "
        "See https://developer.android.com/reference/android/view/KeyEvent")
    parser.add_argument(
        "duration",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta())
    return parser

  def __init__(self,
               source: InputSource,
               duration: dt.timedelta,
               text: Optional[str] = None,
               keyevent: Optional[str] = None,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._text: str | None = text
    self._keyevent: str | None = keyevent
    super().__init__(source, duration, timeout, index)

  @property
  def text(self) -> Optional[str]:
    return self._text

  @property
  def keyevent(self) -> Optional[str]:
    return self._keyevent

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.text_input(run, self)

  @override
  def validate(self) -> None:
    super().validate()
    if bool(self.text) + bool(self.keyevent) != 1:
      raise ValueError(
          f"Exactly one of {self}.text or {self}.keyevent can be specified.")

  @override
  def validate_duration(self) -> None:
    # A text input action is allowed to have a zero duration.
    return

  @override
  def supported_input_sources(self) -> tuple[InputSource, ...]:
    return (InputSource.JS, InputSource.KEYBOARD)

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    if text := self._text:
      details["text"] = text
    if keyevent := self._keyevent:
      details["keyevent"] = keyevent
    return details
