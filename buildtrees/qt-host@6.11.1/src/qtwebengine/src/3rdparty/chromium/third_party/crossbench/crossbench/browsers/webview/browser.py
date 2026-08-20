# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Command to run speedometer on webview_shell:
# `./cb.py speedometer --browser=adb:webview`

from __future__ import annotations

from typing import TYPE_CHECKING, Optional, Sequence

from typing_extensions import override

from crossbench.browsers.webview.webview import Webview

if TYPE_CHECKING:
  from selenium.webdriver.chromium.options import ChromiumOptions

  from crossbench import path as pth
  from crossbench.browsers.settings import Settings
  from crossbench.runner.groups.session import BrowserSessionRunGroup


class WebviewBrowser(Webview):

  def __init__(self,
               label: str,
               path: Optional[pth.AnyPath] = None,
               settings: Optional[Settings] = None) -> None:
    super().__init__(label, path, settings)
    # TODO: crbug.com/408236113 - Read the activity name from config
    # or command line to replace "WebViewBrowserActivity".
    self._android_activity: str = (
        f"{self._android_package}.WebViewBrowserActivity")

  @override
  def quit(self) -> None:
    # External code that started the driver is responsible for
    # shutting it down.
    self._is_running = False
    self._restore_chrome_flags()
    self._teardown_cache_dir()
    if self._stdout_log_file:
      self._stdout_log_file.close()
      self._stdout_log_file = None

  @override
  def _setup_binary_permissions(self) -> None:
    # Override to avoid trying to call grant_permissions.
    pass

  @override
  def _create_options(self, session: BrowserSessionRunGroup,
                      args: Sequence[str]) -> ChromiumOptions:
    options = super()._create_options(session, args)
    # Needed because WebView/WebLayer apps require activity name.
    options.add_experimental_option("androidActivity", self._android_activity)
    return options
