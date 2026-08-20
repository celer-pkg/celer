# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.action_runner.action.action import Action
from crossbench.action_runner.action.action_type import ActionType

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.runner.run import Run


class CloseAllTabsAction(Action):
  TYPE: ActionType = ActionType.CLOSE_ALL_TABS

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.close_all_tabs(run, self)
