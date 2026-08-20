# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import dataclasses
import enum
import re

from typing import Optional

from crossbench import path as pth
from crossbench.config import ConfigEnum


@enum.unique
class TargetPlatform(ConfigEnum):
  ANDROID = ("adb", "Android via adb")
  CHROME_OS = ("cros", "ChromeOS via ssh")
  LOCAL = ("local", "local browser")


@dataclasses.dataclass(frozen=True)
class WebTestsRunConfig:
  platform: TargetPlatform
  device_id: str
  browser: str
  secrets_file: Optional[pth.AnyPath]
  playback: Optional[str]
  tests_regex: re.Pattern
  variants_regex: re.Pattern
  results_path: pth.AnyPath
  web_tests_root: pth.AnyPath
  debug: bool
  dry_run: bool
