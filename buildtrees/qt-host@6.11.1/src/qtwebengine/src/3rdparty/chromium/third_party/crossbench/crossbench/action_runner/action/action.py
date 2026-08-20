# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import functools
import json
from typing import TYPE_CHECKING, Any, Self, Type, TypeVar

from typing_extensions import override

from crossbench import exception
from crossbench.action_runner.action.action_type import ActionType
from crossbench.config import ConfigObject, ConfigParser, UnusedPropertiesMode
from crossbench.parse import DurationParser, NumberParser, ObjectParser

if TYPE_CHECKING:
  import urllib.parse as urlparse

  from crossbench.action_runner.base import ActionRunner
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class ActionTypeConfigParser(ConfigParser):
  """Custom ConfigParser for ActionType that works on
  Action Configs. This way we can pop the 'value' or 'type' key from the
  config dict."""

  def __init__(self) -> None:
    super().__init__(
        ActionType, unused_properties_mode=UnusedPropertiesMode.IGNORE)
    self.add_argument(
        "action",
        aliases=("type",),
        type=ObjectParser.non_empty_str,
        required=True)

  def new_instance_from_kwargs(self, kwargs: dict[str, Any]) -> ActionType:
    return ActionType(kwargs["action"])  # type: ignore


_ACTION_TYPE_CONFIG_PARSER = ActionTypeConfigParser()

ACTION_TIMEOUT = dt.timedelta(seconds=20)

# Lazily initialized Action class lookup.
ACTIONS: dict[ActionType, Type[Action]] = {}

# TODO: remove once pytype is fixed/replaced since it gets confused with Self
# annotations on classmethods with decorators.
ActionT = TypeVar("ActionT", bound="Action")

class Action(ConfigObject, metaclass=abc.ABCMeta):
  TYPE: ActionType = ActionType.GET

  @classmethod
  @override
  def parse_str(cls, value: str) -> Action:
    return ACTIONS[ActionType.GET].parse_str(value)

  @classmethod
  @override
  def parse_any_url(cls, url: urlparse.ParseResult, **kwargs) -> Action:
    cls.expect_no_extra_kwargs(kwargs)
    return ACTIONS[ActionType.GET].parse_any_url(url)

  @classmethod
  @override
  def parse_dict(cls, config: dict[str, Any], **kwargs) -> Self:
    action_type: ActionType = _ACTION_TYPE_CONFIG_PARSER.parse(config)
    action_cls: Type[Self] = ACTIONS[action_type]  # type: ignore
    # Drop _ACTION_TYPE_CONFIG_PARSER arguments/aliases and avoid warnings
    config = dict(config)
    config.pop("action", None)
    config.pop("type", None)

    with exception.annotate_argparsing(
        f"Parsing Action details  ...{{ action: \"{action_type}\", ...}}:"):
      action = action_cls.config_parser().parse(config, **kwargs)
    assert isinstance(action, cls), f"Expected {cls} but got {type(action)}"
    return action

  @classmethod
  @override
  @functools.cache
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = ConfigParser(cls)
    parser.add_argument("index", type=NumberParser.positive_zero_int, default=0)
    parser.add_argument(
        "timeout",
        type=DurationParser.positive_duration,
        default=ACTION_TIMEOUT)
    return parser

  def __init__(self,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._timeout: dt.timedelta = timeout
    self._index = index
    self.validate()

  @property
  def index(self) -> int:
    return self._index

  @property
  def duration(self) -> dt.timedelta:
    return dt.timedelta()

  @property
  def timeout(self) -> dt.timedelta:
    return self._timeout

  @property
  def has_timeout(self) -> bool:
    return self._timeout != dt.timedelta.max

  @abc.abstractmethod
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    pass

  @override
  def validate(self) -> None:
    if self._timeout.total_seconds() < 0:
      raise ValueError(
          f"{self}.timeout should be positive, but got {self.timeout}")

  def to_json(self) -> JsonDict:
    return {"type": str(self.TYPE), "timeout": self.timeout.total_seconds()}

  def __str__(self) -> str:
    return type(self).__name__

  def __eq__(self, other: object) -> bool:
    if isinstance(other, Action):
      return self.to_json() == other.to_json()
    return False

  def __hash__(self) -> int:
    return hash(json.dumps(self.to_json()))
