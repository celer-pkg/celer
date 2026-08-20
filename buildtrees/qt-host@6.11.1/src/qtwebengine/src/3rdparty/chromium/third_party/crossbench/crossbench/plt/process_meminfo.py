# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses


@dataclasses.dataclass(frozen=True)
class ProcessMeminfo:
  pid: int
  name: str
  pss_total: int
  rss_total: int
  swap_total: int
