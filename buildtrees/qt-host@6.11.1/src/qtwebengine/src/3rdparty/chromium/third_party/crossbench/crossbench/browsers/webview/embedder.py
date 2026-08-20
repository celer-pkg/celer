# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
import os
import shlex
from typing import TYPE_CHECKING, Sequence

from selenium import webdriver
from selenium.webdriver.chrome.options import Options as ChromeOptions
from typing_extensions import override

from crossbench.browsers.webview.webview import Webview

if TYPE_CHECKING:
  from selenium.webdriver.chromium.webdriver import ChromiumDriver

  from crossbench import path as pth
  from crossbench.runner.groups.session import BrowserSessionRunGroup


class WebviewEmbedder(Webview):
  @override
  def start(self,  # pytype: disable=override-error
            session: BrowserSessionRunGroup) -> None:
    # Start is a no-op. Embedder activity will be started by the Benchmark.
    # Webview will be started by the Embedder. Driver will be started
    # by the ProbeContext. We do, however, need to set up browser flags
    # and kill any currently running Embedder app instances to make sure
    # it picks up the new flags when started by the Benchmark.
    self._backup_chrome_flags()
    args = self._get_browser_flags_for_session(session)
    logging.debug("%s: setting flags file contents in %s", self,
                  self._chrome_command_line_path)
    self.platform.write_text(self._chrome_command_line_path,
                             shlex.join(("webview", *args)))
    self.platform.sh("pkill", "-f", self.android_package)
    self._log_browser_start(args)
    self._is_running = True

  @override
  def quit(self) -> None:  # pytype: disable=override-error
    # External code that started the driver is responsible for shutting it down.
    self._is_running = False
    self._restore_chrome_flags()
    self._teardown_cache_dir()

  @override
  def _start_driver(self,  # pytype: disable=override-error
                    session: BrowserSessionRunGroup,
                    driver_path: pth.AnyPath) -> ChromiumDriver:
    options = self._create_options(session, [])
    service = webdriver.ChromeService(executable_path=os.fspath(driver_path))
    driver = webdriver.Chrome(options=options, service=service)
    return driver

  def start_driver(self, session: BrowserSessionRunGroup) -> ChromiumDriver:
    assert self._driver_path
    self._private_driver = self._start_driver(session, self._driver_path)
    self._set_driver_timeouts(session)
    return self._private_driver

  @override
  def _create_options(self,  # pytype: disable=override-error
                      session: BrowserSessionRunGroup,
                      args: Sequence[str]) -> ChromeOptions:
    options = ChromeOptions()
    # TODO(zbikowski): process name should come from config
    options.add_experimental_option("androidPackage", self.android_package)
    options.add_experimental_option(
      "androidProcess", f"{self.android_package}:search")
    options.add_experimental_option("androidUseRunningApp", True)
    return options

  @override
  def _log_browser_start(self,
                         args: tuple[str, ...],
                         driver_path: pth.AnyPath | None = None) -> None:
    super()._log_browser_start(args, driver_path)
    logging.info("📱 STARTING BROWSER Embedder: %s",
                 self.platform.app_version(self.android_package))
