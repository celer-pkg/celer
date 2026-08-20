# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
from typing import TYPE_CHECKING, Self

from crossbench.action_runner.android_input_action_runner import \
    AndroidInputActionRunner
from crossbench.action_runner.chromeos_input_action_runner import \
    ChromeOSInputActionRunner
from crossbench.action_runner.default_action_runner import DefaultActionRunner
from crossbench.config import ConfigEnum, ConfigObject, ConfigParser

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.plt.base import Platform


class ActionRunnerType(ConfigEnum):
  AUTO = (
      "auto",
      "Uses the best-fit default action runner based on the browser platform.")
  BASIC = ("basic", str(DefaultActionRunner.__doc__))
  ANDROID = ("android", str(AndroidInputActionRunner.__doc__))
  CHROMEOS = ("chromeos", str(ChromeOSInputActionRunner.__doc__))


@dataclasses.dataclass(frozen=True)
class ActionRunnerConfig(ConfigObject):
  type: ActionRunnerType = ActionRunnerType.AUTO

  @classmethod
  def parse_str(cls, value: str) -> Self:
    runner_type: ActionRunnerType = ActionRunnerType.parse(value)
    return cls(type=runner_type)

  @classmethod
  def config_parser(cls) -> ConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "type", type=ActionRunnerType, default=ActionRunnerType.AUTO)
    return parser

  def instantiate(self, platform: Platform) -> ActionRunner:
    match self.type:
      case ActionRunnerType.ANDROID:
        return AndroidInputActionRunner()
      case ActionRunnerType.CHROMEOS:
        return ChromeOSInputActionRunner()
      case ActionRunnerType.BASIC:
        # TODO: rename
        return DefaultActionRunner()
      case ActionRunnerType.AUTO:
        return self.instantiate_default(platform)
      case _:
        raise ValueError(f"Unsupported action runner type: {self.type}")

  def instantiate_default(self, platform: Platform) -> ActionRunner:
    if platform.is_android:
      return AndroidInputActionRunner()
    if platform.is_chromeos:
      return ChromeOSInputActionRunner()
    return DefaultActionRunner()
