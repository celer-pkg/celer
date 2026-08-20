# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Optional

from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.browser import Browser
from crossbench.browsers.firefox.version import FirefoxVersion
from crossbench.browsers.viewport import Viewport
from crossbench.browsers.webdriver import WebDriverBrowser

if TYPE_CHECKING:
  from crossbench import plt
  from crossbench.flags.base import Flags
  from crossbench.path import AnyPath
  from crossbench.runner.groups.session import BrowserSessionRunGroup


class Firefox(Browser):

  @classmethod
  def default_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Firefox",
        macos=["Firefox.app"],
        linux=["firefox"],
        win=["Mozilla Firefox/firefox.exe"])

  @classmethod
  def developer_edition_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Firefox Developer Edition",
        macos=["Firefox Developer Edition.app"],
        linux=["firefox-developer-edition"],
        win=["Firefox Developer Edition/firefox.exe"])

  @classmethod
  def nightly_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Firefox Nightly",
        macos=["Firefox Nightly.app"],
        linux=["firefox-nightly", "firefox-trunk"],
        win=["Firefox Nightly/firefox.exe"])

  @classmethod
  @override
  def type_name(cls) -> str:
    return "firefox"

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.FIREFOX

  @override
  def _setup_cache_dir(self) -> Optional[AnyPath]:
    if cache_dir := self.settings.cache_dir:
      return cache_dir
    return self.platform.mkdtemp(prefix="firefox")

  @override
  def _extract_version(self) -> FirefoxVersion:
    return FirefoxVersion.parse(self.platform.app_version(self.path))

  @override
  def _get_browser_flags_for_session(
      self, session: BrowserSessionRunGroup) -> tuple[str, ...]:
    flags_copy = self.flags.copy()
    flags_copy.update(session.extra_flags)
    flags_copy.update(self.network.extra_flags(self.attributes()))
    self._handle_viewport_flags(flags_copy)
    if self.log_file:
      flags_copy["--MOZ_LOG_FILE"] = str(self.log_file)
    return tuple(flags_copy)

  def _handle_viewport_flags(self, flags: Flags) -> None:
    new_width, new_height = 0, 0
    if self.viewport.has_size:
      new_width, new_height = self.viewport.size
    update_size = False
    if "--width" in flags:
      if self.viewport.is_default:
        new_width = int(flags["--width"])
        update_size = True
      else:
        assert self.viewport.width == int(flags["--width"])
    if "--height" in flags:
      if self.viewport.is_default:
        new_height = int(flags["--height"])
        update_size = True
      else:
        assert self.viewport.height == int(flags["--height"])
    if update_size:
      assert self.viewport.is_default
      self.viewport = Viewport(new_width, new_height)
    elif self.viewport.has_size:
      flags["--width"] = str(self.viewport.width)
      flags["--height"] = str(self.viewport.height)

    self._sync_viewport_flag(flags, "--kiosk", self.viewport.is_fullscreen,
                             Viewport.FULLSCREEN)
    self._sync_viewport_flag(flags, "--headless", self.viewport.is_headless,
                             Viewport.HEADLESS)

    if self.viewport.has_size and not self.viewport.is_default:
      if not isinstance(self,
                        WebDriverBrowser) and self.viewport.size != (0, 0):
        raise ValueError(f"Browser {self} cannot handle viewport position: "
                         f"{self.viewport.position}")
    elif not isinstance(self, WebDriverBrowser):
      raise ValueError(
          f"Browser {self} cannot handle viewport mode: {self.viewport}")
