# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing_extensions import override

from crossbench.network.traffic_shaping.base import TrafficShaper


class NoTrafficShaper(TrafficShaper):

  @property
  @override
  def is_live(self) -> bool:
    return True

  def __str__(self) -> str:
    return "full"
