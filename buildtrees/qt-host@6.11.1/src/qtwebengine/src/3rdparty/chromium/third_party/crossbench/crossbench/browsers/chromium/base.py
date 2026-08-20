# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from crossbench.browsers.chromium.version import ChromiumVersion

if TYPE_CHECKING:
  from crossbench.path import AnyPath
  from crossbench.plt.base import Platform


class ChromiumBaseMixin:

  @classmethod
  def version_cls(cls) -> Type[ChromiumVersion]:
    return ChromiumVersion

  @classmethod
  def default_path(cls, platform: Platform) -> AnyPath:
    return cls.canary_path(platform)

  @classmethod
  def canary_path(cls, platform: Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Chromium",
        macos=["Chromium.app"],
        linux=["google-chromium", "chromium"],
        win=["Google/Chromium/Application/chromium.exe"])

  @classmethod
  def type_name(cls) -> str:
    return "chromium"
