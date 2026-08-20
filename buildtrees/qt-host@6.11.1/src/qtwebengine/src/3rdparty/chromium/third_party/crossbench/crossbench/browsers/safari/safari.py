# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
from typing import TYPE_CHECKING, Optional

from typing_extensions import override

from crossbench import path as pth
from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.browser import Browser
from crossbench.browsers.safari.version import SafariVersion

if TYPE_CHECKING:
  from crossbench import plt
  from crossbench.browsers.settings import Settings


SAFARIDRIVER_PATH = pth.AnyPosixPath("/usr/bin/safaridriver")


def find_safaridriver(bin_path: pth.AnyPath,
                      platform: plt.Platform) -> pth.AnyPath:
  assert platform.is_file(bin_path), f"Invalid binary path: {bin_path}"
  driver_path = bin_path.parent / "safaridriver"
  if platform.exists(driver_path):
    return driver_path
  # The system-default Safari version doesn't come with the driver
  assert bin_path.is_relative_to(Safari.default_path(platform)), (
      f"Expected default Safari.app binary but got {bin_path}")
  return SAFARIDRIVER_PATH


class Safari(Browser):

  @classmethod
  def default_path(cls, platform: plt.Platform) -> pth.AnyPath:
    return platform.path("/Applications/Safari.app")

  @classmethod
  def technology_preview_path(cls, platform: plt.Platform) -> pth.AnyPath:
    return platform.path("/Applications/Safari Technology Preview.app")

  @classmethod
  @override
  def type_name(cls) -> str:
    return "safari"

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.SAFARI

  def __init__(self,
               label: str,
               path: pth.AnyPath,
               settings: Optional[Settings] = None) -> None:
    self.bundle_name: str = ""
    super().__init__(label, path, settings=settings)
    assert self.platform.is_macos, "Safari only works on MacOS"

  def _init_path_and_version(self, path: Optional[pth.AnyPath] = None) -> None:
    super()._init_path_and_version(path)
    assert self.path
    self.bundle_name = self.path.stem.replace(" ", "")
    assert self.bundle_name

  @override
  def _extract_version(self) -> SafariVersion:
    assert self.path
    app_version: str = self.platform.app_version(self.path)
    driver_version = self.platform.app_version(
        find_safaridriver(self.path, self.platform))
    return SafariVersion.parse(f"{app_version} {driver_version}")

  @override
  def _setup_cache_dir(self) -> Optional[pth.AnyPath]:
    assert self.settings.cache_dir is None, (
        "Cannot set custom cache dir for Safari")
    assert self.bundle_name, "Missing bundle_name"
    cache_dir = self.platform.home() / (
        f"Library/Containers/com.apple.{self.bundle_name}/Data/Library/Caches")
    if not self.platform.exists(cache_dir.parent):
      logging.warning("Could not find existing config dir for %s.", self)
      return None
    self._clear_cache(cache_dir)
    return cache_dir

  @override
  def _clear_cache(self, cache_dir: Optional[pth.AnyPath]) -> None:
    super()._clear_cache(cache_dir)
    # This magic wait lowers safaridriver startup failures.
    self.platform.sleep(0.5)
