# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
import datetime as dt
import functools
from typing import (TYPE_CHECKING, Any, Final, Iterator, Optional, Self,
                    Sequence, cast)

from typing_extensions import override

from crossbench import exception
from crossbench.action_runner.action.action import Action
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.all import ACTIONS_TUPLE
from crossbench.action_runner.action.get import GetAction
from crossbench.action_runner.action.wait_for_ready_state import \
    WaitForReadyStateAction
from crossbench.config import ConfigError, ConfigObject, ConfigParser
from crossbench.parse import NumberParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.benchmarks.loading.page.interactive import InteractivePage
  from crossbench.runner.run import Run

assert ACTIONS_TUPLE, "import failed"

LOGIN_LABEL: Final[str] = "login"


@dataclasses.dataclass(frozen=True)
class ActionBlock(ConfigObject):
  label: str = "default"
  index: int = 0
  actions: tuple[Action, ...] = tuple()

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    raise NotImplementedError("Cannot create action blocks from strings")

  @classmethod
  def parse_other(cls, value: Any, **kwargs) -> Self:
    if isinstance(value, (tuple, list)):
      return cls.parse_sequence(value, **kwargs)
    return super().parse_other(value, **kwargs)

  @classmethod
  @override
  def parse_dict(  # pylint: disable=arguments-differ
      cls,
      config: dict[str, Any],
      label: Optional[str] = None,
      index: Optional[int] = None,
      **kwargs) -> Self:
    return cls.config_parser().parse(config, label=label, index=index, **kwargs)

  @classmethod
  @override
  @functools.cache
  def config_parser(cls) -> ConfigParser[Self]:  # type: ignore #override
    parser = ConfigParser(cls)
    parser.add_argument("label", type=cls._parse_block_label, default="default")
    parser.add_argument(
        "index", type=NumberParser.positive_zero_int, default=0, required=False)
    # TODO: enable passing index
    parser.add_argument("actions", type=Action, required=True, is_list=True)
    return parser

  @classmethod
  def parse_sequence(cls,
                     config: Sequence[dict[str, Any]],
                     label: Optional[str] = None,
                     index: Optional[int] = None) -> Self:
    with exception.annotate_argparsing(
        "Parsing default block action sequence:"):
      return cls.parse_dict({"actions": config}, label=label, index=index)
    raise exception.UnreachableError()

  @classmethod
  def _parse_block_label(cls, value: Any) -> Optional[str]:
    if not value:
      return None
    label = ObjectParser.non_empty_str(value)
    if label == LOGIN_LABEL:
      raise ConfigError(
          f"Block label {repr(label)} is reserved for login blocks")
    return value

  @classmethod
  def from_url(cls, url: str, duration: dt.timedelta) -> ActionBlock:
    actions: tuple[Action, ...] = (GetAction(url, duration),)
    if not duration:
      actions += (WaitForReadyStateAction(),)
    return ActionBlock(actions=actions)

  @override
  def validate(self) -> None:
    super().validate()
    self.validate_actions()

  def validate_actions(self) -> None:
    ObjectParser.non_empty_sequence(self.actions, "actions")
    # TODO: enable validating action indices
    # for index, action in enumerate(self.actions):
    #   if index != action.index:
    #     raise ValueError(
    #         f"action[{index}].index should be {index}, "
    #         f"but got {action.index}")
    if not self.actions:
      raise argparse.ArgumentTypeError("Invalid block without actions")

  def run_with(self, runner: ActionRunner, run: Run,
               page: InteractivePage) -> None:
    del page
    runner.run_block(run, self)

  def to_json(self) -> dict[str, Any]:
    return {
        "label": self.label,
        "actions": [action.to_json() for action in self.actions]
    }

  @property
  def duration(self) -> dt.timedelta:
    total_duration = dt.timedelta()
    for action in self.actions:
      if duration := action.duration:
        total_duration += duration
    return total_duration

  @property
  def is_login(self) -> bool:
    return False

  def __iter__(self) -> Iterator[Action]:
    yield from self.actions

  def __len__(self) -> int:
    return len(self.actions)

  @property
  def first_url(self) -> str:
    for action in self.actions:
      if action.TYPE == ActionType.GET:
        return cast(GetAction, action).url
    raise RuntimeError("No GET action with an URL found.")


@dataclasses.dataclass(frozen=True)
class ActionBlockListConfig(ConfigObject):
  blocks: tuple[ActionBlock, ...] = tuple()

  def to_argument_value(self) -> tuple[ActionBlock, ...]:
    return self.blocks

  @classmethod
  def parse_other(cls, value: Any) -> Self:
    if isinstance(value, (tuple, list)):
      return cls.parse_sequence(value)
    return super().parse_other(value)

  @classmethod
  def parse_sequence(cls, config: Sequence[dict[str, Any]]) -> Self:
    """Parse either a sequence of blocks or a sequence of actions for an
    implicit default block.

    Blocks:
    [{ "label": "block 1", "actions": [...]}, ... ]
    [ "block 1": [{ "action": ...}, ...], "block 2": [ ... ] ]

    Default block actions:
    [{ "action": "get", ...}, { "action": ...}, ...]
    """
    config = ObjectParser.non_empty_sequence(config, "actions")
    info = "action block"
    if cls._is_default_block_actions(config):
      info = "default actions"
      config = [{"actions": config}]
    if not cls._is_block_sequence_config(config):
      raise ValueError(
          "Invalid data: Expected a list of either blocks or actions.")

    def block_config_data_gen():
      for index, block_config in enumerate(config):
        with exception.annotate_argparsing(f"Parsing {info} ...[{index}]"):
          block_config = ObjectParser.dict(block_config, f"blocks[{index}]")
          label = block_config.get("label")
          yield index, label, block_config

    return cls._parse_blocks(block_config_data_gen())

  @classmethod
  def _is_block_sequence_config(cls, config: Sequence[dict[str, Any]]) -> bool:
    return "label" in config[0] or "actions" in config[0]

  @classmethod
  def _is_default_block_actions(cls, config: Sequence[dict[str, Any]]) -> bool:
    sample = config[0]
    return isinstance(sample, str) or "action" in sample

  @classmethod
  @override
  def parse_dict(cls, config: dict[str, Any], **kwargs) -> Self:
    config = ObjectParser.non_empty_dict(config, "blocks")

    def block_config_data_gen():
      for index, (label, block_data) in enumerate(config.items()):
        with exception.annotate_argparsing(
            f"Parsing action block  ...[{label}]"):
          yield index, label, block_data

    return cls._parse_blocks(block_config_data_gen())

  @classmethod
  def _parse_blocks(cls, block_config_data_gen) -> Self:
    blocks: list[ActionBlock] = []
    for index, label, block_data in block_config_data_gen:
      block = cls._parse_block(index, label, block_data)
      blocks.append(block)
    return cls(tuple(blocks))

  @classmethod
  def _parse_block(cls, index: int, label: str, block_data: Any) -> ActionBlock:
    if isinstance(block_data, dict):
      # Early warning for better usability.
      if inner_label := block_data.get("label"):
        if inner_label != label:
          raise ConfigError(
              "ActionBlock inside a dict cannot have a 'label' property, "
              f"but got label={repr(inner_label)}")
    return ActionBlock.parse(block_data, label=label, index=index)

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    raise NotImplementedError("Cannot create action blocks from strings")

  @override
  def validate(self) -> None:
    super().validate()
    if not self.blocks:
      raise ValueError("Missing action blocks.")
    ObjectParser.non_empty_sequence(self.blocks, "blocks")
    for index, block in enumerate(self.blocks):
      if index != block.index:
        raise ValueError(
            f"blocks[{index}].index should be {index}, but got {block.index}")
