# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import TYPE_CHECKING, Final, Optional

from typing_extensions import override

from crossbench.browsers.chrome.webdriver import ChromeWebDriverAndroid
from crossbench.browsers.chromium.webdriver import FLAGS_WEBVIEW

if TYPE_CHECKING:
  from crossbench import path as pth
  from crossbench.browsers.settings import Settings
  from crossbench.browsers.version import BrowserVersion

WEBVIEW_PROVIDER : Final[str] = "com.google.android.webview.debug"


# TODO: crbug.com/393058910 - Replace this Webview class stub placeholder
# once Webview class is properly created.
class Webview(ChromeWebDriverAndroid, metaclass=abc.ABCMeta):

  def __init__(self,
               label: str,
               path: Optional[pth.AnyPath] = None,
               settings: Optional[Settings] = None) -> None:
    super().__init__(label, path, settings)
    self._chrome_command_line_path: pth.AnyPath = FLAGS_WEBVIEW

  @override
  def _extract_version(self) -> BrowserVersion:
    return self.version_cls().parse(self.platform.app_version(WEBVIEW_PROVIDER))
