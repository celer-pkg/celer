# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import TYPE_CHECKING, Type

from crossbench.browsers.chrome.version import ChromeVersion

if TYPE_CHECKING:
  from crossbench import plt
  from crossbench.browsers.chromium.version import ChromiumVersion
  from crossbench.path import AnyPath


class ChromeBaseMixin(abc.ABC):

  @classmethod
  def version_cls(cls) -> Type[ChromiumVersion]:
    return ChromeVersion

  @classmethod
  def default_path(cls, platform: plt.Platform) -> AnyPath:
    return cls.stable_path(platform)

  @classmethod
  def stable_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Chrome Stable",
        macos=["Google Chrome.app"],
        linux=["google-chrome", "chrome"],
        win=["Google/Chrome/Application/chrome.exe"])

  @classmethod
  def beta_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Chrome Beta",
        macos=["Google Chrome Beta.app"],
        linux=["google-chrome-beta"],
        win=["Google/Chrome Beta/Application/chrome.exe"])

  @classmethod
  def dev_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Chrome Dev",
        macos=["Google Chrome Dev.app"],
        linux=["google-chrome-unstable"],
        win=["Google/Chrome Dev/Application/chrome.exe"])

  @classmethod
  def canary_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Chrome Canary",
        macos=["Google Chrome Canary.app"],
        linux=["google-chrome-canary"],
        win=["Google/Chrome SxS/Application/chrome.exe"])

  @classmethod
  def type_name(cls) -> str:
    return "chrome"
