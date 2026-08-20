# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from crossbench import path as pth
  from crossbench.plt.base import Platform


def build_chromedriver_instructions(build_dir: pth.AnyPath) -> str:
  return ("Please build 'chromedriver' manually for local builds:\n"
          f"    autoninja -C {build_dir} chromedriver")


def find_build_dir(path: pth.AnyPath,
                   platform: Platform,
                   limit: int = 3) -> pth.AnyPath | None:
  for parent in path.parents[:limit]:
    if platform.exists(parent / "args.gn"):
      return parent
  return None


def is_build_dir(path: pth.AnyPath, platform: Platform) -> bool:
  return platform.is_file(path / "args.gn")


def is_in_build_dir(path: pth.AnyPath, platform: Platform) -> bool:
  # bypass potentially expensive checks
  if "src" not in path.parts:
    return False
  return any(is_build_dir(parent, platform) for parent in path.parents)
