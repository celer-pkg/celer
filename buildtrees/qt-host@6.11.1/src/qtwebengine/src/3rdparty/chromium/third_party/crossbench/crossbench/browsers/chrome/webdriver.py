# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
from typing import TYPE_CHECKING

import selenium.common.exceptions
from selenium import webdriver
from selenium.webdriver.chrome.options import Options as ChromeOptions
from selenium.webdriver.chrome.service import Service as ChromeService
from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.chrome.base import ChromeBaseMixin
from crossbench.browsers.chromium.webdriver import (
    ChromiumBasedWebDriver, ChromiumWebDriverAndroid,
    ChromiumWebDriverChromeOsSsh, ChromiumWebDriverSsh,
    LocalChromiumWebDriverAndroid)
from crossbench.browsers.chromium_based import helper
from crossbench.browsers.webdriver import DriverException

if TYPE_CHECKING:
  from selenium.webdriver.chromium.options import ChromiumOptions
  from selenium.webdriver.chromium.service import ChromiumService
  from selenium.webdriver.chromium.webdriver import ChromiumDriver


class ChromeWebDriver(ChromeBaseMixin, ChromiumBasedWebDriver):

  WEB_DRIVER_OPTIONS = ChromeOptions
  WEB_DRIVER_SERVICE = ChromeService

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return (BrowserAttributes.CHROME | BrowserAttributes.CHROMIUM_BASED
            | BrowserAttributes.WEBDRIVER)

  @override
  def _create_driver(self,  # pytype: disable=override-error
                     options: ChromiumOptions,
                     service: ChromiumService) -> ChromiumDriver:
    assert isinstance(options, ChromeOptions)
    assert isinstance(service, ChromeService)
    try:
      return webdriver.Chrome(options=options, service=service)
    except selenium.common.exceptions.WebDriverException as e:
      msg: list[str] = [f"Could not start WebDriver: {e.msg}"]
      if self.is_locally_compiled():
        msg.append(helper.build_chromedriver_instructions(self.app_path.parent))
      msg_str = "\n".join(msg)
      logging.error(msg_str)
      raise DriverException(msg_str) from e


class ChromeWebDriverAndroid(ChromiumWebDriverAndroid, ChromeWebDriver):
  pass


class LocalChromeWebDriverAndroid(LocalChromiumWebDriverAndroid,
                                  ChromeWebDriver):
  pass


class ChromeWebDriverSsh(ChromiumWebDriverSsh, ChromeWebDriver):
  pass


class ChromeWebDriverChromeOsSsh(ChromiumWebDriverChromeOsSsh, ChromeWebDriver):
  pass
