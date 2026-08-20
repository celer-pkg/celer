# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import dataclasses
from typing import Any


@dataclasses.dataclass(frozen=True)
class DeviceInfo(abc.ABC):
  device_id: str
  name: str

  def asdict(self) -> dict[str, Any]:
    return dataclasses.asdict(self)

  def __str__(self) -> str:
    return f"{self.name} ({self.device_id})"
