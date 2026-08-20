# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Final, Iterable, Optional, TypeVar

from crossbench import plt

if TYPE_CHECKING:
  from crossbench.path import AnyPath
  PathT = TypeVar("PathT", bound=AnyPath)


def sort_by_file_size(files: Iterable[PathT],
                      platform: Optional[plt.Platform] = None) -> list[PathT]:
  real_platform = platform or plt.PLATFORM
  return sorted(files, key=lambda f: (real_platform.file_size(f), f.name))


SIZE_UNITS: Final[tuple[str, ...]] = ("B", "KiB", "MiB", "GiB", "TiB")


def get_file_size(file: AnyPath,
                  digits: int = 2,
                  platform: Optional[plt.Platform] = None) -> str:
  real_platform = platform or plt.PLATFORM
  size: float = float(real_platform.file_size(file))
  unit_index = 0
  divisor = 1024.0
  while (unit_index < len(SIZE_UNITS)) and size >= divisor:
    unit_index += 1
    size /= divisor
  return f"{size:.{digits}f} {SIZE_UNITS[unit_index]}"
