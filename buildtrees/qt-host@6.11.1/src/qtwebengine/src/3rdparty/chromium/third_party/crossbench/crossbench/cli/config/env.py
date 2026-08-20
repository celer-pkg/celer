# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
import datetime as dt
import enum
from typing import (TYPE_CHECKING, Any, Callable, ClassVar, Optional, Self,
                    TypeAlias)

from typing_extensions import override

from crossbench.config import ConfigObject, ConfigParser
from crossbench.parse import DurationParser, NumberParser, ObjectParser
from crossbench.str_enum_with_help import StrEnumWithHelp

if TYPE_CHECKING:
  Number: TypeAlias = float | int

@enum.unique
class ValidationMode(StrEnumWithHelp):
  THROW = ("throw", "Strict mode, throw and abort on env issues")
  PROMPT = ("prompt", "Prompt to accept potential env issues")
  WARN = ("warn", "Only display a warning for env issue")
  SKIP = ("skip", "Don't perform any env validation")


def merge_bool(name: str, left: Optional[bool],
               right: Optional[bool]) -> Optional[bool]:
  if left is None:
    return right
  if right is None:
    return left
  if left != right:
    raise ValueError(f"Conflicting merge values for {name}: "
                     f"{left} vs. {right}")
  return left


def merge_number_max(name: str, left: Optional[Number],
                     right: Optional[Number]) -> Optional[Number]:
  del name
  if left is None:
    return right
  if right is None:
    return left
  return max(left, right)


def merge_number_min(name: str, left: Optional[Number],
                     right: Optional[Number]) -> Optional[Number]:
  del name
  if left is None:
    return right
  if right is None:
    return left
  return min(left, right)


def merge_str_list(name: str, left: Optional[list[str]],
                   right: Optional[list[str]]) -> Optional[list[str]]:
  del name
  if left is None:
    return right
  if right is None:
    return left
  return left + right


def merge_duration_max(name: str, left: Optional[dt.timedelta],
                       right: Optional[dt.timedelta]) -> Optional[dt.timedelta]:
  del name
  if not left:
    return right
  if not right:
    return left
  return max(left, right)


ENV_CONFIG_PRESETS: dict[str, "EnvConfig"] = {}


@dataclasses.dataclass(frozen=True)
class EnvConfig(ConfigObject):
  IGNORE: ClassVar[None] = None

  browser_allow_background: bool | None = IGNORE
  browser_allow_existing_process: bool | None = IGNORE
  browser_is_headless: bool | None = IGNORE
  cpu_max_usage_percent: float | None = IGNORE
  cpu_min_relative_speed: float | None = IGNORE
  disk_min_free_space_gib: float | None = IGNORE
  power_use_battery: bool | None = IGNORE
  require_probes: bool | None = IGNORE
  screen_allow_autobrightness: bool | None = IGNORE
  screen_brightness_percent: int | None = IGNORE
  screen_refresh_rate: int | None = IGNORE
  system_allow_monitoring: bool | None = IGNORE
  system_forbidden_process_names: list[str] | None = IGNORE
  system_min_uptime: dt.timedelta | None = IGNORE

  @classmethod
  def default(cls) -> EnvConfig:
    return ENV_CONFIG_PRESETS["default"]

  @classmethod
  @override
  def parse_str(cls, value: str) -> EnvConfig:
    value = ObjectParser.non_empty_str(value)
    if preset := ENV_CONFIG_PRESETS.get(value):
      return preset
    raise argparse.ArgumentTypeError(
        f"Unknown host config preset {repr(value)}. "
        f"Choices are {','.join(ENV_CONFIG_PRESETS.keys())}")

  @classmethod
  @override
  def parse_dict(cls, config: dict[str, Any], **kwargs) -> Self:
    if "env" in config:
      config = config["env"]
    return super().parse_dict(config, **kwargs)

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = ConfigParser(cls)
    parser.add_argument("browser_allow_background", type=ObjectParser.bool)
    parser.add_argument(
        "browser_allow_existing_process",
        type=ObjectParser.bool,
        default=cls.IGNORE)
    parser.add_argument("browser_is_headless", type=ObjectParser.bool)
    parser.add_argument(
        "cpu_max_usage_percent",
        type=NumberParser.int_range(0, 100),
        default=cls.IGNORE)
    parser.add_argument(
        "cpu_min_relative_speed",
        type=NumberParser.int_range(0, 1),
        default=cls.IGNORE)
    parser.add_argument(
        "disk_min_free_space_gib",
        type=NumberParser.positive_float,
        default=cls.IGNORE)
    parser.add_argument("power_use_battery", type=ObjectParser.bool)
    parser.add_argument("require_probes", type=ObjectParser.bool)
    parser.add_argument(
        "screen_allow_autobrightness",
        type=ObjectParser.bool,
        default=cls.IGNORE)
    parser.add_argument("screen_brightness_percent", type=int)
    parser.add_argument(
        "screen_refresh_rate",
        type=NumberParser.int_range(30, 240),
        default=cls.IGNORE)
    parser.add_argument("system_allow_monitoring", type=ObjectParser.bool)
    parser.add_argument(
        "system_forbidden_process_names", type=str, is_list=True)
    parser.add_argument(
        "system_min_uptime", type=DurationParser.positive_or_zero_duration)
    return parser

  def merge(self, other: EnvConfig) -> EnvConfig:
    mergers: dict[str, Callable[[str, Any, Any], Any]] = {
        "browser_allow_background": merge_bool,
        "browser_allow_existing_process": merge_bool,
        "browser_is_headless": merge_bool,
        "cpu_max_usage_percent": merge_number_min,
        "cpu_min_relative_speed": merge_number_max,
        "disk_min_free_space_gib": merge_number_max,
        "power_use_battery": merge_bool,
        "require_probes": merge_bool,
        "screen_allow_autobrightness": merge_bool,
        "screen_brightness_percent": merge_number_max,
        "screen_refresh_rate": merge_number_max,
        "system_allow_monitoring": merge_bool,
        "system_forbidden_process_names": merge_str_list,
        "system_min_uptime": merge_duration_max
    }
    kwargs = {}
    for name, merger in mergers.items():
      self_value = getattr(self, name)
      other_value = getattr(other, name)
      kwargs[name] = merger(name, self_value, other_value)
    return EnvConfig(**kwargs)


_config_default = EnvConfig()
_config_strict = EnvConfig(
    cpu_max_usage_percent=98,
    cpu_min_relative_speed=1,
    system_allow_monitoring=False,
    browser_allow_existing_process=False,
    require_probes=True,
    system_min_uptime=dt.timedelta(minutes=5))
_config_battery: EnvConfig = _config_strict.merge(
    EnvConfig(power_use_battery=True))
_config_power: EnvConfig = _config_strict.merge(
    EnvConfig(power_use_battery=False))
_config_catan: EnvConfig = _config_strict.merge(
    EnvConfig(
        screen_brightness_percent=65,
        system_forbidden_process_names=["terminal", "iterm2"],
        screen_allow_autobrightness=False))

ENV_CONFIG_PRESETS.update({
    "default": _config_default,
    "strict": _config_strict,
    "battery": _config_battery,
    "power": _config_power,
    "catan": _config_catan,
})
