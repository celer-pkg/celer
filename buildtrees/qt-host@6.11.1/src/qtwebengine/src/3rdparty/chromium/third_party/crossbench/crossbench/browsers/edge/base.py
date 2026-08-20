# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from crossbench.browsers.edge.version import EdgeVersion

if TYPE_CHECKING:
  from crossbench import plt
  from crossbench.browsers.chromium.version import ChromiumVersion
  from crossbench.path import AnyPath


class EdgeBaseMixin:

  @classmethod
  def version_cls(cls) -> Type[ChromiumVersion]:
    return EdgeVersion

  @classmethod
  def default_path(cls, platform: plt.Platform) -> AnyPath:
    return cls.stable_path(platform)

  @classmethod
  def stable_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Edge Stable",
        macos=["Microsoft Edge.app"],
        linux=["microsoft-edge"],
        win=["Microsoft/Edge/Application/msedge.exe"])

  @classmethod
  def beta_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Edge Beta",
        macos=["Microsoft Edge Beta.app"],
        linux=["microsoft-edge-beta"],
        win=["Microsoft/Edge Beta/Application/msedge.exe"])

  @classmethod
  def dev_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Edge Dev",
        macos=["Microsoft Edge Dev.app"],
        linux=["microsoft-edge-dev"],
        win=["Microsoft/Edge Dev/Application/msedge.exe"])

  @classmethod
  def canary_path(cls, platform: plt.Platform) -> AnyPath:
    return platform.search_app_or_executable(
        "Edge Canary",
        macos=["Microsoft Edge Canary.app"],
        linux=[],
        win=["Microsoft/Edge SxS/Application/msedge.exe"])

  @classmethod
  def type_name(cls) -> str:
    return "edge"
