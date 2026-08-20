# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
from typing import TYPE_CHECKING, Optional, Type

from typing_extensions import override

from crossbench.action_runner.action.action import (ACTION_TIMEOUT, Action,
                                                    ActionT)
from crossbench.action_runner.action.action_type import ActionType
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  import datetime as dt

  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class MeminfoAction(Action):
  TYPE: ActionType = ActionType.MEMINFO

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument("browser", type=ObjectParser.bool, default=True)
    parser.add_argument(
        "packages", type=ObjectParser.non_empty_str, default=(), is_list=True)
    parser.add_argument("system", type=ObjectParser.bool, default=False)
    parser.add_argument("title", type=ObjectParser.non_empty_str, default=None)
    return parser

  def __init__(self,
               browser: bool = True,
               packages: tuple[str, ...] = tuple(),
               title: Optional[str] = None,
               system: bool = False,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._browser = browser
    self._packages = packages
    self._system = system
    self._title = title
    super().__init__(timeout, index)

  @override
  def validate(self) -> None:
    super().validate()
    if not self._browser and not self._packages:
      raise ValueError(
          f"{self} must specify at least one of 'browser' or 'packages'")

  @property
  def browser(self) -> bool:
    return self._browser

  @property
  def packages(self) -> tuple[str, ...]:
    return self._packages

  @property
  def system(self) -> bool:
    return self._system

  @property
  def title(self) -> Optional[str]:
    return self._title

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    details["browser"] = self.browser
    details["system"] = self.system
    if self.packages:
      details["packages"] = list(self.packages)
    if self.title:
      details["title"] = self.title
    return details

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.dump_meminfo(run, self)
