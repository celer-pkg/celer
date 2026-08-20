# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import logging
import os
from typing import (TYPE_CHECKING, Any, Iterable, Optional, Sequence, TextIO,
                    Type, cast)

from selenium.webdriver.chromium.options import ChromiumOptions
from selenium.webdriver.chromium.service import ChromiumService
from selenium.webdriver.chromium.webdriver import ChromiumDriver
from typing_extensions import override

from crossbench import path as pth
from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.chromium.driver_finder import (ChromeDriverFinder,
                                                        DriverNotFoundError)
from crossbench.browsers.chromium.version import (ChromeDriverVersion,
                                                  ChromiumVersion)
from crossbench.browsers.chromium_based import helper
from crossbench.browsers.chromium_based.chromium_based import ChromiumBased
from crossbench.browsers.chromium_based.devtools_tracer import DevToolsTracer
from crossbench.browsers.webdriver import WebDriverBrowser
from crossbench.flags.chrome import ChromeFlags
from crossbench.helper import wait

if TYPE_CHECKING:
  import re

  from selenium import webdriver

  from crossbench.browsers.version import BrowserVersion
  from crossbench.flags.base import FlagsT
  from crossbench.runner.groups.session import BrowserSessionRunGroup


class ChromiumBasedWebDriver(
    WebDriverBrowser, ChromiumBased, metaclass=abc.ABCMeta):

  WEB_DRIVER_OPTIONS: Type[ChromiumOptions] = ChromiumOptions
  WEB_DRIVER_SERVICE: Type[ChromiumService] = ChromiumService
  UNSUPPORTED_FLAGS: tuple[str, ...] = ()

  def __init__(self, *args, **kwargs) -> None:
    super().__init__(*args, **kwargs)
    self._script_identifier_kwargs: dict[Any, Any] | None = None
    self._tracer: DevToolsTracer | None = None
    self._stdout_log_file: TextIO | None = None

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return (BrowserAttributes.CHROMIUM | BrowserAttributes.CHROMIUM_BASED
            | BrowserAttributes.WEBDRIVER)

  def use_local_chromedriver(self) -> bool:
    return self.version.major == 0 or self.is_locally_compiled()

  def is_locally_compiled(self) -> bool:
    return bool(self.local_build_dir())

  def local_build_dir(self) -> pth.LocalPath | None:
    if path := helper.find_build_dir(self.path, self.host_platform):
      return self.host_platform.local_path(path)
    return None

  def _execute_cdp_cmd(self, driver: webdriver.Remote, cmd: str,
                       cmd_args: dict):
    return driver.execute("executeCdpCommand", {
        "cmd": cmd,
        "params": cmd_args
    })["value"]

  @override
  def _filter_flags_for_run(self, flags: FlagsT) -> FlagsT:
    assert isinstance(flags, ChromeFlags)
    chrome_flags: ChromeFlags = cast(ChromeFlags, flags)
    for flag in self.UNSUPPORTED_FLAGS:
      if flag not in chrome_flags:
        continue
      flag_value = chrome_flags.pop(flag, None)
      logging.debug("Chromium: Removed unsupported flag: %s=%s", flag,
                    flag_value)
    return chrome_flags  # type: ignore

  @override
  def _find_driver(self) -> pth.AnyPath:
    if self._driver_path:
      return self._driver_path
    finder = ChromeDriverFinder(self)
    assert self.app_path
    if self.use_local_chromedriver():
      return finder.find_local_build()
    try:
      return finder.download()
    except DriverNotFoundError as original_download_error:
      logging.debug(
          "Could not download chromedriver, "
          "falling back to finding local build: %s", original_download_error)
      try:
        return finder.find_local_build()
      except DriverNotFoundError as e:
        logging.debug("Could not find fallback chromedriver: %s", e)
        raise original_download_error from e
      # to make an old pytype version happy
      return pth.LocalPath()

  @override
  def _start_driver(self, session: BrowserSessionRunGroup,
                    driver_path: pth.AnyPath) -> webdriver.Remote:
    return self._start_chromedriver(session, driver_path)

  def _start_chromedriver(self, session: BrowserSessionRunGroup,
                          driver_path: pth.AnyPath) -> ChromiumDriver:
    assert not self._is_running
    assert self.log_file
    args = self._get_browser_flags_for_session(session)
    options = self._create_options(session, args)

    self._log_browser_start(args, driver_path)
    service_args: list[str] = []
    if self._settings.driver_logging:
      service_args += [
          "--verbose", f"--log-path={os.fspath(self._setup_driver_log_file())}"
      ]

    adb_port = os.environ.get("ANDROID_ADB_SERVER_PORT")
    if adb_port and adb_port.isdigit():
      service_args += ["--adb-port=" + adb_port]

    # pytype: disable=wrong-keyword-args
    assert self._stdout_log_file is None
    # On desktop platforms service logs contain browser stdout, hence the name.
    self._stdout_log_file = self.log_file.with_stem("browser.stdout").open("w+")
    service = self.WEB_DRIVER_SERVICE(
        executable_path=os.fspath(driver_path),
        service_args=service_args,
        log_output=self._stdout_log_file,
    )
    if hasattr(service, "log_file"):
      # TODO: remove once we upgrade the min selenium version
      # Workaround for older selenium versions which ignore the log_file kwarg.
      setattr(service, "log_file", self._stdout_log_file)

    # TODO: support remote platforms
    driver = self._create_driver(options, service)
    # pytype: enable=wrong-keyword-args
    # Prevent debugging overhead.
    self._execute_cdp_cmd(driver, "Runtime.setMaxCallStackSizeToCapture",
                          {"size": 0})
    return driver

  def _create_options(self, session: BrowserSessionRunGroup,
                      args: Sequence[str]) -> ChromiumOptions:
    assert not self._is_running
    options: ChromiumOptions = self.WEB_DRIVER_OPTIONS()
    options.set_capability("browserVersion", str(self.version.major))
    # Don't wait for document-ready.
    options.set_capability("pageLoadStrategy", "none")
    for arg in args:
      options.add_argument(arg)
    options.binary_location = os.fspath(self.path)
    session.setup_selenium_options(options)
    return options

  @abc.abstractmethod
  def _create_driver(self, options: ChromiumOptions,
                     service: ChromiumService) -> ChromiumDriver:
    pass

  @override
  def _validate_driver_version(self) -> None:
    assert self._driver_path, "No driver available"
    error_message = None
    if self.is_local and helper.is_build_dir(
        self.platform.local_path(self.app_path.parent), self.platform):
      error_message = self._validate_locally_built_driver(
          self.platform.local_path(self._driver_path))
    else:
      error_message = self._validate_any_driver_version(self._driver_path)
    if error_message:
      raise RuntimeError("\n".join(error_message))

  def _validate_locally_built_driver(
      self, driver_path: pth.LocalPath) -> Optional[Iterable[str]]:
    # TODO: migrate to version object on the browser
    browser_version: BrowserVersion = self.version
    assert isinstance(browser_version, ChromiumVersion)
    driver_version = ChromeDriverVersion.parse(
        self.platform.app_version(driver_path))
    if browser_version.parts == driver_version.parts:
      return None
    return (f"Chromedriver version mismatch: driver={driver_version.parts_str} "
            f"browser={browser_version.parts_str} ({self}).",
            helper.build_chromedriver_instructions(driver_path.parent))

  def _validate_any_driver_version(
      self, driver_path: pth.AnyPath) -> Optional[Iterable[str]]:
    raw_version_str = self.host_platform.sh_stdout(driver_path, "--version")
    driver_version = ChromeDriverVersion.parse(raw_version_str)
    if driver_version.major == self.version.major:
      return None
    return (f"Chromedriver version mismatch: driver={driver_version} "
            f"browser={self.version} ({self})",)

  @override
  def run_script_on_new_document(self, script: str) -> None:
    if self._script_identifier_kwargs is not None:
      self._execute_cdp_cmd(self._private_driver,
                            "Page.removeScriptToEvaluateOnNewDocument",
                            self._script_identifier_kwargs)
    self._script_identifier_kwargs = self._execute_cdp_cmd(
        self._private_driver, "Page.addScriptToEvaluateOnNewDocument",
        {"source": script})

  @override
  def quit(self) -> None:
    self._script_identifier_kwargs = None
    super().quit()

  @override
  def current_window_id(self) -> str:
    return str(self._private_driver.current_window_handle)

  @override
  def switch_window(self, window_id: str) -> None:
    self._private_driver.switch_to.window(window_id)

  @override
  def switch_tab(
      self,
      title: Optional[re.Pattern] = None,
      url: Optional[re.Pattern] = None,
      tab_index: Optional[int] = None,
      relative_tab_index: Optional[int] = None,
      timeout: dt.timedelta = dt.timedelta(seconds=0)
  ) -> str:
    assert not (tab_index is not None and relative_tab_index is not None)
    driver = self._private_driver
    original_handle = driver.current_window_handle
    for _ in wait.wait_with_backoff(timeout):
      # Search through other handles starting from current_window_handle + 1
      try:
        i = driver.window_handles.index(original_handle)
      except ValueError as e:
        raise RuntimeError("Original starting tab no longer exists") from e

      if relative_tab_index is not None:
        tab_index = (i + relative_tab_index) % len(driver.window_handles)
      if tab_index is not None:
        handles = [driver.window_handles[tab_index]]
      else:
        # Start searching with the tab after the current tab.
        handles = driver.window_handles[i + 1:] + driver.window_handles[:i + 1]

      for handle in handles:
        driver.switch_to.window(handle)
        if title is not None:
          if title.search(driver.title) is None:
            continue
        if url is not None:
          if url.search(driver.current_url) is None:
            continue
        return handle
    error = "No new tab found"
    if title is not None:
      error += f" with title matching {repr(title.pattern)}"
    if url is not None:
      error += f" with url matching {repr(url.pattern)}"
    if tab_index is not None:
      error += f" with tab_index matching {tab_index}"
    if relative_tab_index is not None:
      error += f" with relative_tab_index matching {tab_index}"
    raise RuntimeError(error)

  @override
  def close_tab(
      self,
      title: Optional[re.Pattern] = None,
      url: Optional[re.Pattern] = None,
      tab_index: Optional[int] = None,
      relative_tab_index: Optional[int] = None,
      timeout: dt.timedelta = dt.timedelta(seconds=0)
  ) -> None:
    driver = self._private_driver
    original_handle = driver.current_window_handle
    tab_to_close = original_handle

    if title or url or (tab_index is not None):
      tab_to_close = self.switch_tab(title, url, tab_index, relative_tab_index,
                                     timeout)

    driver.close()

    if tab_to_close != original_handle:
      driver.switch_to.window(original_handle)
    else:
      # When a tab closes itself, arbitrarily default
      # to switching to the first tab.
      driver.switch_to.window(driver.window_handles[0])

  @override
  def close_all_tabs(self) -> None:
    driver = self._private_driver
    current_handle = driver.current_window_handle

    for handle in driver.window_handles:
      driver.switch_to.window(handle)
      if handle != current_handle:
        driver.close()

    # Closing every tab will cause the browser to exit.
    # As a workaround navigate the final tab to about:blank.
    driver.switch_to.window(current_handle)
    self.show_url("about:blank")

  @property
  def current_url(self) -> str:
    return self._private_driver.current_url

  # TODO(crbug.com/428953697): Consider unifying BrowserProfilingProbe with
  # other similar ones.
  def start_profiling(self) -> None:
    assert isinstance(self._private_driver, ChromiumDriver)
    self._tracer = DevToolsTracer(self._private_driver)
    self._tracer.start()

  def stop_profiling(self) -> Any:
    assert isinstance(self._private_driver, ChromiumDriver)
    assert self._tracer is not None
    output = self._tracer.end()
    self._tracer = None
    return output

  @override
  def force_quit(self) -> None:
    try:
      super().force_quit()
    finally:
      if self._stdout_log_file:
        self._stdout_log_file.close()
        self._stdout_log_file = None
