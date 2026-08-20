# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from crossbench.action_runner.action import all as i_action
  from crossbench.action_runner.action.bond import BondAction
  from crossbench.runner.run import Run


class BondActionNotImplementedError(NotImplementedError):

  def __init__(self,
               runner: BondActionRunner,
               action: BondAction,
               msg_context: str = "") -> None:
    self.runner = runner
    self.action = action

    if msg_context:
      msg_context = f", context: {msg_context}"
    message = (f"{str(action.TYPE)}-action "
               f"not implemented in {type(runner).__name__}{msg_context}")
    super().__init__(message)


class BondActionRunner:

  def meet_create(self, run: Run, action: i_action.MeetCreateAction):
    del run
    raise BondActionNotImplementedError(self, action)

  def meet_script(self, run: Run, action: i_action.MeetScriptAction):
    del run
    raise BondActionNotImplementedError(self, action)
