# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.action_runner.action.click import ClickAction
from crossbench.action_runner.action.enums import ReadyState
from crossbench.benchmarks.loading.config.login.base import PresetLoginBlock

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.benchmarks.loading.page.interactive import InteractivePage
  from crossbench.cli.config.secrets import UsernamePassword
  from crossbench.runner.actions import Actions
  from crossbench.runner.run import Run

GOOGLE_LOGIN_URL: str = (
    "https://accounts.google.com/Logout?"
    "continue=https%3A%2F%2Faccounts.google.com%2Fv3%2Fsignin%2Fidentifier%3F"
    "flowName%3DGlifWebSignIn%26flowEntry%3DServiceLogin")

SUCCESSFUL_LOGIN_REDIRECT: str = "https://myaccount.google.com"

ADD_PASSKEY_REDIRECT: str = (
    "https://accounts.google.com/v3/signin/speedbump/passkeyenrollment")
SKIP_PASSKEY_ACTION: ClickAction = ClickAction.parse({
    "action": "click",
    "pos": {
        "selector": "xpath///span[contains(text(), 'Not now')]",
        "required": True,
        "wait": True,
    },
    "source": "js",
})

ADD_RECOVERY_PHONE_REDIRECT: str = "https://gds.google.com/web/recoveryoptions"
SKIP_RECOVERY_PHONE: ClickAction = ClickAction.parse({
    "action": "click",
    "pos": {
        "selector": "[aria-label='Cancel']",
        "required": True,
        "wait": True,
    },
    "source": "js",
})

ADD_HOME_ADDRESS_REDIRECT: str = "https://gds.google.com/web/homeaddress"
SKIP_HOME_ADDRESS: ClickAction = ClickAction.parse({
    "action": "click",
    "pos": {
        "selector": "[aria-label='Skip']",
        "required": True,
        "wait": True,
    },
    "source": "js",
})

SUSPICIOUS_ACTIVITY_URL: str = "https://myaccount.google.com/notifications"
CHCEK_SUSPICIOUS_ACTIVITY: ClickAction = ClickAction.parse({
    "action": "click",
    "pos": {
        "selector": "[aria-label='Check activity']",
        "required": True,
    },
    "source": "js",
})

CLICK_YES_IT_WAS_ME: ClickAction = ClickAction.parse({
    "action": "click",
    "pos": {
        "selector": "xpath///button[./span[text()='Yes, it was me']]",
        "required": True,
        "wait": True,
    },
    "source": "js",
    "verify": "xpath///body[not(./button[./span[text()='Yes, it was me']])]",
})


class GoogleLogin(PresetLoginBlock):
  """Google-specific login steps."""

  def _submit_login_field(self, action: Actions, secret: UsernamePassword,
                          aria_label: str, input_val: str,
                          button_name: str) -> None:
    if secret.is_interactive:
      return
    action.wait_js_condition(
        ("return "
         f"document.querySelector(\"[aria-label='{aria_label}']\") != null &&"
         f"document.getElementById({repr(button_name)}) != null;"),
        0.2,
        timeout=10)
    action.js("const inputField ="
              f" document.querySelector(\"[aria-label='{aria_label}']\");"
              f"inputField.value = {repr(input_val)};"
              f"document.getElementById({repr(button_name)}).click();")

  def timeout(self, secret: UsernamePassword) -> dt.timedelta:
    if secret.is_interactive:
      return dt.timedelta(seconds=60)
    return dt.timedelta(seconds=10)

  @override
  def run_with(self, runner: ActionRunner, run: Run,
               page: InteractivePage) -> None:
    secret: UsernamePassword | None = run.secrets.google
    if not secret:
      raise RuntimeError("No google login provided")

    if self.is_logged_in(run, secret, strict=True):
      return

    with run.actions("Login", measure=False) as action:
      action.show_url(GOOGLE_LOGIN_URL)
      self._submit_login_field(action, secret, "Email or phone",
                               secret.username, "identifierNext")
      self._submit_login_field(action, secret, "Enter your password",
                               secret.password, "passwordNext")

      action.wait_js_condition(
          ("return !document.URL.startsWith("
           "'https://accounts.google.com/v3/signin/challenge/pwd');"), 0.2,
          self.timeout(secret))
      action.wait_for_ready_state(ReadyState.COMPLETE, self.timeout(secret))

      wait_range = run.wait_range(0.2, self.timeout(secret))
      for _, _, time_left in wait_range.wait_with_backoff():
        current_url = action.current_url()

        if current_url.startswith(SUCCESSFUL_LOGIN_REDIRECT):
          break

        if current_url.startswith(ADD_PASSKEY_REDIRECT):
          logging.info("Dismissing passkey enrollment page.")
          self._dismiss_login_page(action, runner, run, SKIP_PASSKEY_ACTION,
                                   ADD_PASSKEY_REDIRECT, time_left)

        if current_url.startswith(ADD_RECOVERY_PHONE_REDIRECT):
          logging.info("Dismissing account recovery page.")
          self._dismiss_login_page(action, runner, run, SKIP_RECOVERY_PHONE,
                                   ADD_RECOVERY_PHONE_REDIRECT, time_left)

        if current_url.startswith(ADD_HOME_ADDRESS_REDIRECT):
          logging.info("Dismissing add home address page.")
          self._dismiss_login_page(action, runner, run, SKIP_HOME_ADDRESS,
                                   ADD_HOME_ADDRESS_REDIRECT, time_left)

      self._clear_suspicious_activity(action, runner, run)


  def _dismiss_login_page(self, action: Actions, runner: ActionRunner, run: Run,
                          click_action: ClickAction, current_url: str,
                          timeout: dt.timedelta) -> None:
    runner.click(run, click_action)
    action.wait_js_condition(
        f"return !document.URL.startsWith('{current_url}');", 0.2, timeout)
    action.wait_for_ready_state(ReadyState.COMPLETE, timeout)

  def _clear_suspicious_activity(self, action: Actions, runner: ActionRunner,
                                 run: Run):
    has_suspicious_activity = action.js(
        "return document.querySelector("
        "\"[aria-label='Check activity']\") != null;")

    if not has_suspicious_activity:
      return

    runner.click(run, CHCEK_SUSPICIOUS_ACTIVITY)
    runner.click(run, CLICK_YES_IT_WAS_ME)
