# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TypedDict


class DisplayInfo(TypedDict):
  resolution: tuple[int, int]
  refresh_rate: float
