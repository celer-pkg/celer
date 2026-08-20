# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.base_tab_action import BaseTabAction

if TYPE_CHECKING:
  from crossbench.action_runner.action.action import ActionT
  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run


class CloseTabAction(BaseTabAction):
  TYPE: ActionType = ActionType.CLOSE_TAB

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    return parser

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.close_tab(run, self)
