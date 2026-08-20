# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import enum
from typing import Any

from crossbench.parse import ObjectParser
from crossbench.str_enum_with_help import StrEnumWithHelp


@enum.unique
class BrowserDriverType(StrEnumWithHelp):
  WEB_DRIVER = ("WebDriver", "Use Selenium with webdriver, for local runs.")
  APPLE_SCRIPT = ("AppleScript", "Use AppleScript, for local macOS runs only")
  ANDROID = ("Android",
             "Use Webdriver for android. Allows to specify additional settings")
  IOS = ("iOS", "Placeholder, unsupported at the moment")
  LINUX_SSH = ("Remote Linux",
               "Use remote webdriver and execute commands via SSH")
  CHROMEOS_SSH = ("Remote ChromeOS",
                  "Use remote ChromeDriver and execute commands via SSH")

  @classmethod
  def default(cls) -> BrowserDriverType:
    return cls.WEB_DRIVER

  @classmethod
  def parse(cls, value: Any) -> BrowserDriverType:
    if isinstance(value, cls):
      return value
    if value == "":
      return BrowserDriverType.default()
    value = ObjectParser.non_empty_str(value, "driver_type")
    identifier = value.lower()
    if identifier in ("selenium", "webdriver"):
      return BrowserDriverType.WEB_DRIVER
    if identifier in ("applescript", "osa"):
      return BrowserDriverType.APPLE_SCRIPT
    if identifier in ("android", "adb"):
      return BrowserDriverType.ANDROID
    if identifier in ("iphone", "ios"):
      return BrowserDriverType.IOS
    if identifier == "ssh":
      return BrowserDriverType.LINUX_SSH
    if identifier == "chromeos-ssh":
      return BrowserDriverType.CHROMEOS_SSH
    raise argparse.ArgumentTypeError(f"Unknown driver type: {repr(value)}")

  @property
  def is_remote_driver(self) -> bool:
    if self in (BrowserDriverType.CHROMEOS_SSH, BrowserDriverType.LINUX_SSH):
      return True
    return False

  @property
  def is_local_driver(self) -> bool:
    return not self.is_remote_driver

  @property
  def is_remote_browser(self) -> bool:
    if self in (BrowserDriverType.ANDROID, BrowserDriverType.CHROMEOS_SSH,
                BrowserDriverType.LINUX_SSH):
      return True
    return False

  @property
  def is_local_browser(self) -> bool:
    return not self.is_remote_browser
