# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import typing
from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from crossbench import path as pth
  from crossbench.plt.base import Platform


class ProbeResultOrigin(typing.Protocol):

  @property
  def browser_platform(self) -> Platform:
    raise NotImplementedError()

  @property
  def host_platform(self) -> Platform:
    raise NotImplementedError()

  @property
  def browser_tmp_dir(self) -> pth.AnyPath:
    raise NotImplementedError()

  @property
  def is_remote(self) -> bool:
    raise NotImplementedError()

  @property
  def out_dir(self) -> pth.LocalPath:
    raise NotImplementedError()
