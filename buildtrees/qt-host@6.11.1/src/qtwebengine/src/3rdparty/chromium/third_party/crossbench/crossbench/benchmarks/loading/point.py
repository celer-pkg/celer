# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses


@dataclasses.dataclass(frozen=True)
# Represents a single point on a screen.
class Point:
  # The offset in pixels from the left edge of the screen.
  x: int
  # The offset in pixels from the top edge of the screen.
  y: int
