# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import atexit
import logging
import os
import time
import traceback
from typing import TYPE_CHECKING, Any, Optional, Sequence, cast

import selenium.common.exceptions
import urllib3
from selenium import webdriver
from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.browser import Browser
from crossbench.browsers.version import BrowserVersion, UnknownBrowserVersion
from crossbench.probes.internal.browser.driver_log import BrowserDriverLogProbe
from crossbench.types import JsonDict

if TYPE_CHECKING:
  import datetime as dt

  from selenium.webdriver.common.timeouts import Timeouts
  from selenium.webdriver.remote.remote_connection import RemoteConnection

  from crossbench.browsers.settings import Settings
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.path import AnyPath, LocalPath
  from crossbench.runner.groups.session import BrowserSessionRunGroup


def _get_http_timeout(driver: webdriver.Remote) -> int:
  executor = cast("RemoteConnection", driver.command_executor)
  return executor.client_config.timeout


def _set_http_timeout(driver: webdriver.Remote, timeout: float):
  logging.debug("Setting http request timeout to %s", timeout)
  executor = cast("RemoteConnection", driver.command_executor)
  executor.client_config.timeout = timeout


class DriverException(RuntimeError):
  """Wrapper for more readable error messages than the default
  WebDriver exceptions."""

  def __init__(self, msg: str, browser: Optional[Browser] = None) -> None:
    self._browser = browser
    self._msg = msg
    super().__init__(msg)

  def __str__(self) -> str:
    browser_prefix = ""
    if self._browser:
      browser_prefix = f"browser={self._browser}: "
    return f"{browser_prefix}{self._msg}"


class JsTimeoutContext:
  """
    A context manager to temporarily adjust Selenium WebDriver and JS timeouts
    and restore them afterwards.
    """

  def __init__(self, driver: webdriver.Remote, timeout: Optional[dt.timedelta]):
    if timeout is not None and timeout.total_seconds() <= 0:
      raise ValueError("Timeout must be a positive duration.")

    self._driver = driver
    self._new_timeout = timeout

  def __enter__(self):
    if self._new_timeout is None:
      return

    self._original_command_executor_timeout: float = _get_http_timeout(
        self._driver)
    self._original_script_timeout: float = self._driver.timeouts.script

    _set_http_timeout(self._driver, self._new_timeout.total_seconds())
    self._driver.set_script_timeout(self._new_timeout.total_seconds())
    return

  def __exit__(self, exc_type, exc_val, exc_tb):
    if self._new_timeout is None:
      return

    if self._original_command_executor_timeout is not None:
      _set_http_timeout(self._driver, self._original_command_executor_timeout)
      self._driver.set_script_timeout(self._original_script_timeout)

class WebDriverBrowser(Browser, metaclass=abc.ABCMeta):
  # TODO: properly annotate this lazily initialized instance variable.
  _private_driver: webdriver.Remote

  def __init__(self,
               label: str,
               path: Optional[AnyPath] = None,
               settings: Optional[Settings] = None) -> None:
    super().__init__(label, path, settings)
    self._driver_path: AnyPath | None = self._settings.driver_path
    self._driver_log_file: LocalPath | None = None
    self._driver_pid: int = 0
    self._pid: int = 0
    self.log_file: LocalPath | None = None

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.WEBDRIVER

  @property
  def driver_log_file(self) -> Optional[LocalPath]:
    return self._driver_log_file

  @override
  def validate_binary(self) -> None:
    super().validate_binary()
    self._driver_path = self.host_platform.absolute(self._find_driver())
    # TODO: support remote chromedriver as well
    assert self.host_platform.exists(self._driver_path), (
        f"Webdriver path '{self._driver_path}' does not exist")

  @abc.abstractmethod
  def _find_driver(self) -> AnyPath:
    pass

  @abc.abstractmethod
  def _validate_driver_version(self) -> None:
    pass

  @override
  def validate_env(self, env: RunnerEnv) -> None:
    super().validate_env(env)
    self._validate_driver_version()

  @override
  def start(self, session: BrowserSessionRunGroup) -> None:
    super().start(session)
    assert self._driver_path
    try:
      self._private_driver = self._start_driver(session, self._driver_path)
    except selenium.common.exceptions.WebDriverException as e:
      msg = e.msg or "Could not create Webdriver session."
      raise DriverException(msg, self) from e
    self._is_running = True
    atexit.register(self.force_quit)
    self._find_driver_pid()
    self._set_driver_timeouts(session)
    self._setup_window()

  def _find_driver_pid(self) -> None:
    service = getattr(self._private_driver, "service", None)
    if not service:
      return
    self._driver_pid = service.process.pid
    candidates: list[int] = []
    for child in self.platform.process_children(self._driver_pid):
      if str(child["exe"]) == str(self.path):
        candidates.append(child["pid"])
    if len(candidates) == 1:
      self._pid = candidates[0]
    else:
      logging.debug(
          "Could not find unique browser process for webdriver: %s, got %s",
          self, candidates)

  def _set_driver_timeouts(self, session: BrowserSessionRunGroup) -> None:
    """Adjust the global webdriver timeouts if the runner has custom timeout
    unit values.
    If timing.has_no_timeout each value is set to SAFE_MAX_TIMEOUT_TIMEDELTA."""
    if http_timeout := self.http_request_timeout:
      _set_http_timeout(self._private_driver, http_timeout.total_seconds())
    timing = session.timing
    if not timing.timeout_unit:
      return
    if timing.has_no_timeout:
      logging.info("Disabling webdriver timeouts")
    else:
      factor = timing.timeout_unit.total_seconds()
      if factor != 1.0:
        logging.info("Increasing webdriver timeouts by %fx", factor)
    timeouts: Timeouts = self._private_driver.timeouts
    if implicit_wait := getattr(timeouts, "implicit_wait", None):
      timeouts.implicit_wait = timing.timeout_timedelta(
          implicit_wait).total_seconds()
    if script := getattr(timeouts, "script", None):
      timeouts.script = timing.timeout_timedelta(script).total_seconds()
    if page_load := getattr(timeouts, "page_load", None):
      timeouts.page_load = timing.timeout_timedelta(page_load).total_seconds()
    self._private_driver.timeouts = timeouts

  def _setup_driver_log_file(self) -> LocalPath:
    log_file = self.log_file
    assert log_file, "Missing browser log file"
    self._driver_log_file = log_file.with_suffix(".driver.log")
    assert self._driver_log_file.name == BrowserDriverLogProbe.NAME, (
        f"Expected driver log file name {BrowserDriverLogProbe.NAME}, "
        f"but got: {self._driver_log_file}")
    return self._driver_log_file

  def _setup_window(self) -> None:
    # Force main window to foreground.
    self._private_driver.switch_to.window(
        self._private_driver.current_window_handle)
    if (self.viewport.is_headless or
        not self._private_driver.capabilities["setWindowRect"]):
      return
    if self.viewport.is_fullscreen:
      self._private_driver.fullscreen_window()
    elif self.viewport.is_maximized:
      self._private_driver.maximize_window()
    else:
      self._private_driver.set_window_position(self.viewport.x, self.viewport.y)
      self._private_driver.set_window_size(self.viewport.width,
                                           self.viewport.height)

  @abc.abstractmethod
  def _start_driver(self, session: BrowserSessionRunGroup,
                    driver_path: AnyPath) -> webdriver.Remote:
    pass

  @override
  def details_json(self) -> JsonDict:
    details: JsonDict = super().details_json()
    log = cast(JsonDict, details["log"])
    if self.driver_log_file:
      log["driver"] = os.fspath(self.driver_log_file)
    return details

  @override
  def show_url(self, url: str, target: Optional[str] = None) -> None:
    logging.debug("WebDriverBrowser.show_url(%s, %s)", url, target)
    try:
      if target in ("_self", None):
        # Do the navigation in the active tab.
        pass
      elif target == "_new_tab":
        self._private_driver.switch_to.new_window("tab")
      elif target == "_new_window":
        self._private_driver.switch_to.new_window("window")
      else:
        raise RuntimeError(f"unexpected target {target}")
      self._private_driver.get(url)
    except selenium.common.exceptions.WebDriverException as e:
      if msg := e.msg:
        self._wrap_webdriver_exception(e, msg, url)
      raise

  @override
  def switch_to_new_tab(self) -> None:
    self._private_driver.switch_to.new_window("tab")

  @override
  def screenshot(self, path: LocalPath) -> None:
    if not self._private_driver.get_screenshot_as_file(path.as_posix()):
      raise DriverException(
          f"Browser failed to get_screenshot_as_file to file '{path}'", self)

  def _wrap_webdriver_exception(
      self, e: selenium.common.exceptions.WebDriverException, msg: str,
      url: str) -> None:
    if "net::ERR_CONNECTION_REFUSED" in msg:
      raise DriverException(
          f"Browser failed to load URL={url}. The URL is likely unreachable.",
          self) from e
    if "net::ERR_INTERNET_DISCONNECTED" in msg:
      raise DriverException(
          f"Browser failed to load URL={url}. "
          f"The device is not connected to the internet.", self) from e

  @override
  def js(
      self,
      script: str,
      timeout: Optional[dt.timedelta] = None,
      arguments: Sequence[object] = ()
  ) -> Any:
    logging.debug("WebDriverBrowser.js() timeout=%s, script: %s", timeout,
                  script)
    assert self._is_running
    try:
      with JsTimeoutContext(self._private_driver, timeout):
        return self._private_driver.execute_script(script, *arguments)
    except selenium.common.exceptions.WebDriverException as e:
      # pylint: disable=raise-missing-from
      raise ValueError(f"Could not execute JS: {e.msg}")

  def close_all_tabs(self) -> None:
    try:
      all_handles = self._private_driver.window_handles
      for handle in all_handles:
        self._private_driver.switch_to.window(handle)
        self._private_driver.close()
    except (selenium.common.exceptions.InvalidSessionIdException,
            urllib3.exceptions.MaxRetryError) as e:
      logging.debug("%s: Got errors while closing all tabs: {%s}", self, e)

  @override
  def quit(self) -> None:
    try:
      assert self._is_running
      self.close_all_tabs()
    finally:
      super().quit()

  @override
  def force_quit(self) -> None:
    if getattr(self, "_private_driver", None) is None or not self._is_running:
      return
    atexit.unregister(self.force_quit)
    logging.debug("WebDriverBrowser.force_quit()")
    try:
      try:
        # Close the current window.
        self._private_driver.close()
        time.sleep(0.1)
      except selenium.common.exceptions.NoSuchWindowException:
        # No window is good.
        pass
      except selenium.common.exceptions.InvalidSessionIdException:
        # Closing the last tab will close the session as well.
        return
      try:
        self._private_driver.quit()
      except selenium.common.exceptions.InvalidSessionIdException:
        return
      # Sometimes a second quit is needed, ignore any warnings there
      try:
        self._private_driver.quit()
      except Exception as e:  # pylint: disable=broad-except
        logging.debug("Driver raised exception on quit: %s\n%s", e,
                      traceback.format_exc())
      return
    except Exception as e:  # pylint: disable=broad-except
      logging.debug("Could not quit browser: %s\n%s", e, traceback.format_exc())
    finally:
      self._is_running = False


class RemoteWebDriver(WebDriverBrowser, Browser):
  """Represent a remote WebDriver that has already been started"""

  @classmethod
  @override
  def type_name(cls) -> str:
    return "remote"

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.WEBDRIVER | BrowserAttributes.REMOTE

  def __init__(self, label: str, driver: webdriver.Remote) -> None:
    self._private_driver = driver
    super().__init__(label=label, path=None)

  @override
  def _extract_version(self) -> BrowserVersion:
    raw_version: str = self._private_driver.capabilities["browserVersion"]
    parts: tuple[int, ...] = tuple(map(int, raw_version.split(".")))
    return UnknownBrowserVersion(parts, version_str=raw_version)

  @override
  def _validate_driver_version(self) -> None:
    pass

  @override
  def _find_driver(self) -> LocalPath:
    raise NotImplementedError()

  @override
  def _start_driver(self, session: BrowserSessionRunGroup,
                    driver_path: AnyPath) -> webdriver.Remote:
    raise NotImplementedError()

  @override
  def _setup_binary(self) -> None:
    pass

  @override
  def _setup_cache_dir(self):
    pass

  def validate_binary(self) -> None:
    pass

  @override
  def start(self, session: BrowserSessionRunGroup) -> None:
    # Driver has already been started. We just need to mark it as running.
    self._is_running = True
    self._setup_window()

  @override
  def quit(self) -> None:
    # External code that started the driver is responsible for shutting it down.
    self._is_running = False
