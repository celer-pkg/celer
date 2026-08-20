# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
import os
from typing import TYPE_CHECKING, Any, Optional, Set, Type

from selenium import webdriver
from selenium.webdriver.safari.options import Options as SafariOptions
from selenium.webdriver.safari.service import Service as SafariService
from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.safari.safari import Safari, find_safaridriver
from crossbench.browsers.webdriver import DriverException, WebDriverBrowser
from crossbench.cli import ui
from crossbench.helper.wait import WaitRange
from crossbench.path import AnyPath, LocalPath

if TYPE_CHECKING:
  from crossbench.browsers.settings import Settings
  from crossbench.runner.groups.session import BrowserSessionRunGroup


class SafariWebDriver(WebDriverBrowser, Safari):

  MAX_STARTUP_TIMEOUT = dt.timedelta(seconds=10)

  def __init__(self,
               label: str,
               path: AnyPath,
               settings: Optional[Settings] = None) -> None:
    super().__init__(label, path, settings)
    assert self.platform.is_macos

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.SAFARI | BrowserAttributes.WEBDRIVER

  @override
  def _find_driver(self) -> AnyPath:
    # TODO: support remote platform
    assert self.platform.is_local, "Remote platform is not supported yet"
    return self.host_platform.local_path(
        find_safaridriver(self.path, self.platform))

  @override
  def _setup_driver_log_file(self) -> LocalPath:
    raise NotImplementedError("Cannot use custom driver log path for Safari")

  @override
  def _start_driver(self, session: BrowserSessionRunGroup,
                    driver_path: AnyPath) -> webdriver.Remote:
    return self._start_safari_driver(session, driver_path)

  def _start_safari_driver(self, session: BrowserSessionRunGroup,
                           driver_path: AnyPath) -> webdriver.Safari:
    assert not self._is_running
    logging.info("STARTING BROWSER: browser: %s driver: %s", self.path,
                 driver_path)

    options: SafariOptions = self._get_driver_options(session)
    session.setup_selenium_options(options)

    service = SafariService(executable_path=os.fspath(driver_path))
    driver_kwargs = {"service": service, "options": options}

    with ui.spinner():
      driver = self._start_driver_with_retries(driver_kwargs)
      self.platform.sleep(0.5)

    assert driver.session_id, "Could not start webdriver"
    logs: AnyPath = (
        self.platform.home() / "Library/Logs/com.apple.WebDriver" /
        driver.session_id)
    all_logs = list(self.platform.glob(logs, "safaridriver*"))
    if all_logs:
      self._driver_log_file = LocalPath(all_logs[0])
      assert self.platform.is_file(all_logs[0])
    return driver

  # TODO(cbruni): implement iOS platform
  def _start_driver_with_retries(
      self, driver_kwargs: dict[str, Any]) -> webdriver.Safari:
    # safaridriver for iOS / technology preview seems to be brittle.
    # Let's give it several chances to start up.
    seen_exceptions: Set[Type[Exception]] = set()
    retries = 0
    for _ in WaitRange(
        min=2, timeout=self.MAX_STARTUP_TIMEOUT).wait_with_backoff():
      try:
        return webdriver.Safari(**driver_kwargs)
      except Exception as e:  # pylint: disable=broad-except
        retries += 1
        exception_type = type(e)
        logging.warning("SafariWebDriver: startup failed (%s), retrying...",
                        exception_type)
        logging.debug("SafariWebDriver: startup error %s", e)
        # After 2 retries we don't accept the same error twice.
        if retries >= 2 and exception_type in seen_exceptions:
          raise DriverException("Could not start SafariWebDriver") from e
        seen_exceptions.add(type(e))
    raise DriverException("Could not start SafariWebDriver")

  def _get_driver_options(self,
                          session: BrowserSessionRunGroup) -> SafariOptions:
    options = SafariOptions()
    # Don't wait for document-ready.
    options.set_capability("pageLoadStrategy", "none")

    args = self._get_browser_flags_for_session(session)
    for arg in args:
      options.add_argument(arg)

    if self._settings.driver_logging:
      options.set_capability("safari:diagnose", "true")
    if "Technology Preview" in self.app_name:
      options.set_capability("browserName", "Safari Technology Preview")
      options.use_technology_preview = True
    return options

  @override
  def _validate_driver_version(self) -> None:
    # The bundled driver is always ok
    assert self._driver_path
    for parent in self._driver_path.parents:
      if parent == self.path.parent:
        return
    version = self.platform.sh_stdout(self._driver_path, "--version")
    assert str(self.version.major) in version, (
        f"safaridriver={self._driver_path} version='{version}' "
        f" doesn't match safari version={self.version.major}")

  @override
  def _setup_window(self) -> None:
    super()._setup_window()
    self.platform.exec_apple_script(f"""
        tell application "{self.app_name}"
          activate
        end tell""")

  @override
  def quit(self) -> None:
    super().quit()
    if self.platform.is_macos:
      # Safari needs some additional push to quit properly
      self.platform.exec_apple_script(f"""
          tell application "{self.app_name}"
            quit
          end tell""")

  @override
  def force_quit(self) -> None:
    try:
      super().force_quit()
    finally:
      # Certain safaridriver versions keep on lingering around when they fail.
      self.platform.sh("killall", "-9", "safaridriver", check=False)


class SafariWebdriverIOS(SafariWebDriver):
  MAX_STARTUP_TIMEOUT = dt.timedelta(seconds=15)

  @override
  def _get_driver_options(self,
                          session: BrowserSessionRunGroup) -> SafariOptions:
    options = super()._get_driver_options(session)
    desired_cap = {
        # "browserName": "Safari",
        # "browserVersion": "17.0.3", # iOS version
        # "safari:deviceType": "iPhone",
        # "safari:deviceName": "XXX's iPhone",
        # "safari:deviceUDID": "...",
        "platformName": "iOS",
        "safari:initialUrl": "about:blank",
        "safari:openLinksInBackground": True,
        "safari:allowPopups": True,
    }
    for key, value in desired_cap.items():
      options.set_capability(key, value)
    return options

  @override
  def _setup_window(self) -> None:
    pass
