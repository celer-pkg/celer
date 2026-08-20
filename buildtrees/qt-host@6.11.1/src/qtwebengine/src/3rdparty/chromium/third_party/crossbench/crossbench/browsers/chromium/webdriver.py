# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import logging
import re
import subprocess
import sys
from typing import TYPE_CHECKING, Any, Optional, Sequence, cast

from immutabledict import immutabledict
from selenium.webdriver.chromium import webdriver as chromium_webdriver
from selenium.webdriver.remote.webdriver import WebDriver as RemoteWebDriver
from typing_extensions import override

from crossbench import exception
from crossbench import hjson as cb_hjson
from crossbench import path as pth
from crossbench.browsers.chromium.base import ChromiumBaseMixin
from crossbench.browsers.chromium_based.webdriver import ChromiumBasedWebDriver
from crossbench.cli import ui
from crossbench.cli.config.secrets import GoogleUsernamePassword
from crossbench.helper import wait
from crossbench.helper.path_finder import ChromiumBuildBinaryFinder
from crossbench.parse import NumberParser
from crossbench.plt.android_adb import AndroidAdbPlatform
from crossbench.plt.base import SubprocessError
from crossbench.plt.bin import Binaries
from crossbench.plt.chromeos_ssh import ChromeOsSshPlatform
from crossbench.plt.linux_ssh import LinuxSshPlatform

if TYPE_CHECKING:
  import datetime as dt

  from selenium import webdriver
  from selenium.webdriver.chromium.options import ChromiumOptions
  from selenium.webdriver.chromium.service import ChromiumService

  from crossbench.browsers.settings import Settings
  from crossbench.browsers.version import BrowserVersion
  from crossbench.cli.config.secrets import UsernamePassword
  from crossbench.plt.base import Platform
  from crossbench.plt.process_meminfo import ProcessMeminfo
  from crossbench.runner.groups.session import BrowserSessionRunGroup


# Android is high-tech and reads chrome flags from an app-specific file.
# TODO: extend support to more than just chrome.
_FLAG_ROOT: pth.AnyPosixPath = pth.AnyPosixPath("/data/local/tmp/")
FLAGS_WEBLAYER: pth.AnyPosixPath = _FLAG_ROOT / "weblayer-command-line"
FLAGS_WEBVIEW: pth.AnyPosixPath = _FLAG_ROOT / "webview-command-line"
FLAGS_CONTENT_SHELL: pth.AnyPosixPath = (
    _FLAG_ROOT / "content-shell-command-line")
FLAGS_CHROME: pth.AnyPosixPath = _FLAG_ROOT / "chrome-command-line"


class ChromiumWebDriver(ChromiumBaseMixin, ChromiumBasedWebDriver):

  @override
  def _create_driver(
      self, options: ChromiumOptions,
      service: ChromiumService) -> chromium_webdriver.ChromiumDriver:
    return chromium_webdriver.ChromiumDriver(
        browser_name="chromium",
        vendor_prefix="goog",
        options=options,
        service=service)


class ChromiumWebDriverAndroid(ChromiumBasedWebDriver):

  def __init__(self,
               label: str,
               path: Optional[pth.AnyPath] = None,
               settings: Optional[Settings] = None) -> None:
    assert settings, "Android browser needs custom settings and platform"
    self._chrome_command_line_path: pth.AnyPath = FLAGS_CHROME
    self._previous_command_line_contents: str | None = None
    self._needs_restore_chrome_flags: bool = False
    super().__init__(label, path, settings)
    self._android_package: str = self._lookup_android_package(self.path)
    if not self._android_package:
      raise RuntimeError("Could not find matching adb package for "
                         f"{self.path} on {self.platform}")

  def _lookup_android_package(self, path: pth.AnyPath) -> str:
    return self.platform.app_path_to_package(path)

  @property
  def android_package(self) -> str:
    return self._android_package

  @property
  @override
  def platform(self) -> AndroidAdbPlatform:
    assert isinstance(
        self._platform,
        AndroidAdbPlatform), (f"Invalid platform: {self._platform}")
    return cast(AndroidAdbPlatform, self._platform)

  def _init_resolve_binary(self, path: pth.AnyPath) -> pth.AnyPath:
    return path

  UNSUPPORTED_FLAGS: tuple[str, ...] = (
      "--disable-sync",
      "--window-size",
      "--window-position",
  )

  @override
  def _start_driver(self, session: BrowserSessionRunGroup,
                    driver_path: pth.AnyPath) -> webdriver.Remote:
    self.adb_force_stop()
    if session.browser.wipe_system_user_data:
      self.adb_force_clear()
      self._setup_binary_permissions()
    self._backup_chrome_flags()
    return self._start_chromedriver(session, driver_path)

  def _backup_chrome_flags(self) -> None:
    assert self._previous_command_line_contents is None
    self._previous_command_line_contents = self._read_device_flags()
    assert not self._needs_restore_chrome_flags, "Invalid flag restore state."
    self._needs_restore_chrome_flags = True
    atexit.register(self._restore_chrome_flags)

  def _read_device_flags(self) -> Optional[str]:
    if not self.platform.exists(self._chrome_command_line_path):
      return None
    return self.platform.cat(self._chrome_command_line_path)

  def adb_force_stop(self) -> None:
    self.platform.adb.force_stop(self.android_package)

  def adb_force_clear(self) -> None:
    self.platform.adb.force_clear(self.android_package)

  def force_quit(self) -> None:
    try:
      try:
        super().force_quit()
      finally:
        self.adb_force_stop()
    finally:
      self._restore_chrome_flags()

  @override
  def meminfo(self, timeout: dt.timedelta) -> list[ProcessMeminfo]:
    return self.platform.process_meminfo(self.android_package, timeout)

  def _restore_chrome_flags(self) -> None:
    atexit.unregister(self._restore_chrome_flags)
    if not self._needs_restore_chrome_flags:
      return
    current_flags = self._read_device_flags()
    if current_flags != self._previous_command_line_contents:
      logging.warning("%s: flags file changed during run", self)
      logging.debug("before: %s", self._previous_command_line_contents)
      logging.debug("current: %s", current_flags)
    if self._previous_command_line_contents is None:
      logging.debug("%s: deleting chrome flags file: %s", self,
                    self._chrome_command_line_path)
      self.platform.rm(self._chrome_command_line_path, missing_ok=True)
    else:
      logging.debug("%s: restoring previous flags file contents in %s", self,
                    self._chrome_command_line_path)
      self.platform.write_text(self._chrome_command_line_path,
                               self._previous_command_line_contents)
    self._needs_restore_chrome_flags = False
    self._previous_command_line_contents = None

  @override
  def _create_options(self, session: BrowserSessionRunGroup,
                      args: Sequence[str]) -> ChromiumOptions:
    options: ChromiumOptions = super()._create_options(session, args)
    options.binary_location = ""
    options.add_experimental_option("androidPackage", self.android_package)
    options.add_experimental_option("androidDeviceSerial",
                                    self.platform.adb.serial_id)
    if not self.clear_cache_dir:
      options.add_experimental_option("androidKeepAppDataDir", True)
    return options

  @override
  def _setup_binary(self) -> None:  # pytype: disable=override-error
    super()._setup_binary()
    self._setup_binary_permissions()

  def _setup_binary_permissions(self) -> None:
    try:
      self.platform.adb.grant_permissions(self.android_package)
    except SubprocessError as e:
      logging.warning("Error setting app permissions: %s", e)

  @override
  def _setup_window(self) -> None:  # pytype: disable=override-error
    logging.debug("%s: Skipping viewport settings %s on %s",
                  type(self).__name__, self.viewport, self)


class LocalChromiumWebDriverAndroid(ChromiumWebDriverAndroid):
  """
  Custom version that uses a locally built bundle wrapper.
  https://chromium.googlesource.com/chromium/src/+/HEAD/docs/android_build_instructions.md
  """

  @classmethod
  def is_apk_helper(cls, path: Optional[pth.AnyPath]) -> bool:
    if not path or len(path.parts) == 1:
      return False
    return path.name.endswith("_apk")

  def __init__(self,
               label: str,
               path: Optional[pth.AnyPath] = None,
               settings: Optional[Settings] = None) -> None:
    if not self.is_apk_helper(path):
      raise ValueError(
          "Locally built chrome version does not work with packaged apks.")
    assert settings, "Android browser needs custom settings and platform"
    assert path, "Got invalid path"
    self._package_info: immutabledict[str, Any] = self._parse_package_info(
        settings.platform, path)
    super().__init__(label, path, settings)

  @override
  def _lookup_android_package(self, path: pth.AnyPath) -> str:
    return self._package_info["Package name"]

  # TODO: enable override again.
  # @override
  def _extract_version(self) -> BrowserVersion:
    return self.version_cls().parse(self._package_info["versionName"])

  def _parse_package_info(self, platform: Platform,
                          path: pth.AnyPath) -> immutabledict[str, Any]:
    output = platform.host_platform.sh_stdout(
        path, "package-info").rstrip().splitlines()
    package_info = {}
    for line in output:
      key, value = line.split(": ")
      package_info[key] = cb_hjson.loads_unique_keys(value)
    return immutabledict(package_info)

  @override
  def _setup_binary(self) -> None:
    super()._setup_binary()
    with ui.spinner():
      sys.stdout.write(f"   Installing {self.path.name} on {self.platform}\r")
      self.host_platform.sh_stdout(self.path, "install",
                                   f"--device={self.platform.serial_id}")

  @override
  def _find_driver(self) -> pth.AnyPath:
    if self._driver_path:
      return self._driver_path
    assert self.app_path
    if build_dir := self.local_build_dir():
      logging.info("Looking for local chromedriver in %s", build_dir.parent)
      finder = ChromiumBuildBinaryFinder(self.host_platform, "chromedriver",
                                         (build_dir.parent,))
      if driver_path := finder.path:
        return driver_path
    raise ValueError("Chrome APK helper needs an explicit chrome driver. "
                     "Use --driver-path or a custom browser config.")


class AutoForwardingRemoteWebDriver(RemoteWebDriver):
  """
  Wraps RemoteWebDriver, but starts, stops, and forwards ports for chromedriver.
  """

  # Example ss output line (with whitespace shortened):
  # LISTEN 0 5 127.0.0.1:34595 0.0.0.0:* users:(("chromedriver",pid=80388,fd=8))
  SS_CHROMEDRIVER_LINE_RE = re.compile(
      r"^LISTEN\s+"
      # Recv-Q
      r"\d+\s+"
      # Send-Q
      r"\d+\s+"
      # Local Address:Port
      r"127.0.0.1:(?P<port>\d+)\s+"
      # Peer Address:Port
      r"\S+\s+"
      # Process
      r"users:\(\("
      r"\"chromedriver\",pid=\d+,fd=\d+"
      r"\)\)\s*$",
      re.MULTILINE)

  _platform: LinuxSshPlatform
  _forward_port: int
  _chromedriver: subprocess.Popen | None

  def __init__(
      self,
      platform: LinuxSshPlatform,
      chromedriver_path: Optional[pth.AnyPath],
      options: ChromiumOptions,
  ) -> None:
    with exception.annotate("Starting chromedriver"):
      self._platform = platform
      self._killall_chromedriver()
      self._chromedriver = platform.popen(
          chromedriver_path or Binaries.CHROMEDRIVER.resolve(platform),
          stdin=subprocess.PIPE)
      atexit.register(self._stop_remote_driver)
      driver_port = self._wait_for_driver_port()
      self._forward_port = platform.ports.forward(0, driver_port)
      logging.info(
          "Chromedriver listening on %d forwarded through local port %d",
          driver_port, self._forward_port)
    super().__init__(f"http://127.0.0.1:{self._forward_port}", options=options)

  def quit(self) -> None:
    try:
      super().quit()
    finally:
      self._stop_remote_driver()

  def _stop_remote_driver(self) -> None:
    if not self._chromedriver:
      return
    try:
      self._chromedriver.terminate()
      self._chromedriver = None
    finally:
      try:
        # Closing the ssh connection doesn't terminate chromedriver, so kill it.
        self._killall_chromedriver()
      finally:
        if forward_port := self._forward_port:
          self._platform.ports.stop_forward(forward_port)
          self._forward_port = 0

  def _killall_chromedriver(self) -> None:
    self._platform.sh("killall", "chromedriver", check=False)

  def _wait_for_driver_port(self) -> int:
    for _ in wait.wait_with_backoff(10):
      listening = self._platform.sh_stdout("ss", "-HOlntp")
      if m := self.SS_CHROMEDRIVER_LINE_RE.search(listening):
        return NumberParser.port_number(m[1], "driver port")
    raise RuntimeError("not reached")


class ChromiumWebDriverSsh(ChromiumBasedWebDriver):

  @property
  @override
  def platform(self) -> LinuxSshPlatform:
    assert isinstance(self._platform,
                      LinuxSshPlatform), (f"Invalid platform: {self._platform}")
    return cast(LinuxSshPlatform, self._platform)

  @override
  def _start_driver(self, session: BrowserSessionRunGroup,
                    driver_path: pth.AnyPath) -> RemoteWebDriver:
    del driver_path
    args = self._get_browser_flags_for_session(session)
    options = self._create_options(session, args)
    platform = self.platform
    host = platform.host
    port = platform.port
    if port == 0:
      return AutoForwardingRemoteWebDriver(
          platform, self._settings.driver_path, options=options)
    driver = RemoteWebDriver(f"http://{host}:{port}", options=options)
    return driver


class ChromiumWebDriverChromeOsSsh(ChromiumBasedWebDriver):

  @property
  @override
  def platform(self) -> ChromeOsSshPlatform:
    assert isinstance(
        self._platform,
        ChromeOsSshPlatform), (f"Invalid platform: {self._platform}")
    return cast(ChromeOsSshPlatform, self._platform)

  UNSUPPORTED_FLAGS: tuple[str, ...] = (
      "--user-data-dir",
      "--window-size",
      "--window-position",
  )

  @override
  def _start_driver(self, session: BrowserSessionRunGroup,
                    driver_path: pth.AnyPath) -> RemoteWebDriver:
    del driver_path
    platform = self.platform
    host = platform.host
    port = platform.port
    args: tuple[str, ...] = self._get_browser_flags_for_session(session)
    # TODO(spadhi): correctly handle flags:
    #   1. decide which flags to pass to chrome vs chromedriver
    #   2. investigate irrelevant / unsupported flags on ChromeOS
    #   3. filter out and pass the chrome flags to the debugging session below
    #   4. pass the remaining flags to RemoteWebDriver options
    google_login = session.browser.secrets.google
    if google_login:
      dbg_port = platform.create_debugging_session(
          username=google_login.username,
          password=google_login.password,
          browser_flags=args)
    else:
      dbg_port = platform.create_debugging_session(browser_flags=args)
    options = self._create_options(session, args)
    options.add_experimental_option("debuggerAddress", f"127.0.0.1:{dbg_port}")

    if port == 0:
      return AutoForwardingRemoteWebDriver(
          platform, self._settings.driver_path, options=options)
    return RemoteWebDriver(f"http://{host}:{port}", options=options)

  # On ChromeOS, the system profile is the same as the browser profile.
  def is_logged_in(self,
                   secret: UsernamePassword,
                   strict: bool = False) -> bool:
    if secret.username == self.platform.username and isinstance(
        secret, GoogleUsernamePassword):
      return True
    if not strict:
      return False
    raise RuntimeError("Login of non-primary Google accounts not supported")
