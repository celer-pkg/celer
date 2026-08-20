# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum


class BrowserAttributes(enum.IntFlag):
  SAFARI = enum.auto()
  FIREFOX = enum.auto()
  CHROMIUM = enum.auto()
  CHROME = enum.auto()
  EDGE = enum.auto()

  CHROMIUM_BASED = enum.auto()

  D8 = enum.auto()

  WEBDRIVER = enum.auto()
  APPLESCRIPT = enum.auto()

  MOBILE = enum.auto()
  DESKTOP = enum.auto()

  REMOTE = enum.auto()

  @property
  def is_chromium_based(self) -> bool:
    return bool(self.CHROMIUM_BASED & self)

  @property
  def is_chrome(self) -> bool:
    return bool(self & self.CHROME)

  @property
  def is_safari(self) -> bool:
    return bool(self & self.SAFARI)

  @property
  def is_edge(self) -> bool:
    return bool(self & self.EDGE)

  @property
  def is_firefox(self) -> bool:
    return bool(self & self.FIREFOX)

  @property
  def is_remote(self) -> bool:
    return bool(self & self.REMOTE)

  @property
  def is_local(self) -> bool:
    return not self.is_remote
