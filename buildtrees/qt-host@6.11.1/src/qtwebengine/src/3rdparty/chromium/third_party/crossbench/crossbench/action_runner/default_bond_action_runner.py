# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.action_runner.action.enums import ReadyState
from crossbench.action_runner.action.get import GetAction
from crossbench.action_runner.bond_base import BondActionRunner
from crossbench.bond.bond import BondClient
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  from crossbench.action_runner.action import all as i_action
  from crossbench.action_runner.base import ActionRunner
  from crossbench.browsers.browser import Browser
  from crossbench.runner.run import Run


class DefaultBondActionRunner(BondActionRunner):

  def __init__(self, action_runner: ActionRunner) -> None:
    self._action_runner: ActionRunner = action_runner
    self._bond_client: BondClient | None = None

  def bond_client(self, run: Run) -> BondClient:
    if not self._bond_client:
      secret = run.secrets.bond
      if not secret:
        raise RuntimeError("No bond service account secret provided")
      self._bond_client = BondClient(secret)
    return self._bond_client

  def teardown(self) -> None:
    if self._bond_client:
      self._bond_client.teardown()
      self._bond_client = None

  def get_current_conference_code(self, browser: Browser) -> str:
    url = ObjectParser.url(browser.current_url)
    if url.hostname != "meet.google.com":
      raise RuntimeError(f"Unsupported URL for Bond action: {url.geturl()}")
    # Conference code is url path without leading '/'
    return url.path[1:]

  def _timeout_from_deadline(self, deadline: dt.datetime):
    timeout = deadline - dt.datetime.now()
    if timeout <= dt.timedelta(0):
      # This should only happen if we have multiple requests in an action, and
      # the first completes but exactly uses up all the deadline.
      raise TimeoutError("A previous request used up the timeout")
    return timeout

  @override
  def meet_create(self, run: Run, action: i_action.MeetCreateAction) -> None:
    deadline = dt.datetime.now() + action.timeout
    bond_client = self.bond_client(run)
    conference_code = bond_client.create_meeting(timeout=action.timeout)
    if action.bots:
      bond_client.add_bots(
          conference_code,
          action.bots,
          timeout=self._timeout_from_deadline(deadline))
    url = f"https://meet.google.com/{conference_code}"
    self._action_runner.get(
        run,
        GetAction(
            url,
            ready_state=ReadyState.COMPLETE,
            target=action.target,
            timeout=self._timeout_from_deadline(deadline)))

  def meet_script(self, run: Run, action: i_action.MeetScriptAction) -> None:
    conference_code = self.get_current_conference_code(run.browser)
    bond_client = self.bond_client(run)
    bond_client.run_script(conference_code, action.script, action.timeout)
