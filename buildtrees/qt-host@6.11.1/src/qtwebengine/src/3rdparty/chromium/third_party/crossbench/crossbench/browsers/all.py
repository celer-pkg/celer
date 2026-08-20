# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import annotations

from crossbench.browsers.chrome.applescript import ChromeAppleScript
from crossbench.browsers.chrome.chrome import Chrome
from crossbench.browsers.chrome.webdriver import (ChromeWebDriver,
                                                  ChromeWebDriverAndroid,
                                                  ChromeWebDriverChromeOsSsh,
                                                  ChromeWebDriverSsh,
                                                  LocalChromeWebDriverAndroid)
from crossbench.browsers.chromium.applescript import ChromiumAppleScript
from crossbench.browsers.chromium.chromium import Chromium
from crossbench.browsers.chromium.webdriver import (
    ChromiumBasedWebDriver, ChromiumWebDriver, ChromiumWebDriverAndroid,
    ChromiumWebDriverChromeOsSsh, ChromiumWebDriverSsh,
    LocalChromiumWebDriverAndroid)
from crossbench.browsers.chromium_based.chromium_based import ChromiumBased
from crossbench.browsers.d8.d8 import D8
from crossbench.browsers.edge.edge import Edge
from crossbench.browsers.edge.webdriver import EdgeWebDriver
from crossbench.browsers.firefox.firefox import Firefox
from crossbench.browsers.firefox.webdriver import FirefoxWebDriver
from crossbench.browsers.safari.applescript import SafariAppleScript
from crossbench.browsers.safari.safari import Safari
from crossbench.browsers.safari.webdriver import (SafariWebDriver,
                                                  SafariWebdriverIOS)
from crossbench.browsers.webview.browser import WebviewBrowser
from crossbench.browsers.webview.embedder import WebviewEmbedder

__all__ = [
    "Chrome",
    "ChromeAppleScript",
    "ChromeWebDriver",
    "ChromeWebDriverAndroid",
    "ChromeWebDriverChromeOsSsh",
    "ChromeWebDriverSsh",
    "Chromium",
    "ChromiumAppleScript",
    "ChromiumBased",
    "ChromiumBasedWebDriver",
    "ChromiumWebDriver",
    "ChromiumWebDriverAndroid",
    "ChromiumWebDriverChromeOsSsh",
    "ChromiumWebDriverSsh",
    "D8",
    "Edge",
    "EdgeWebDriver",
    "Firefox",
    "FirefoxWebDriver",
    "LocalChromeWebDriverAndroid",
    "LocalChromiumWebDriverAndroid",
    "Safari",
    "SafariAppleScript",
    "SafariWebDriver",
    "SafariWebdriverIOS",
    "WebviewBrowser",
    "WebviewEmbedder",
]
