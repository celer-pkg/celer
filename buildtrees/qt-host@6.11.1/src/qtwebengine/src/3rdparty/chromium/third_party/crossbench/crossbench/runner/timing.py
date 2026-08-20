# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
import datetime as dt
from typing import TypeAlias

# Arbitrary very large number that doesn't break any browser driver protocol.
# chromedriver likely uses an uint32 ms internally, 2**30ms == 12days.
SAFE_MAX_TIMEOUT_TIMEDELTA = dt.timedelta(milliseconds=2**30)


AnyTime: TypeAlias = float | int | dt.timedelta
AnyTimeUnit: TypeAlias = float | int | dt.timedelta


@dataclasses.dataclass(frozen=True)
class Timing:
  cool_down_time: dt.timedelta = dt.timedelta(seconds=1)
  # General purpose time unit.
  unit: dt.timedelta = dt.timedelta(seconds=1)
  # Used for upper bound / timeout limits independently.
  timeout_unit: dt.timedelta = dt.timedelta()
  run_timeout: dt.timedelta = dt.timedelta()
  # Wait time after starting the browser and before running a workload.
  start_delay: dt.timedelta = dt.timedelta()
  # Wait time after running a workload and before stopping a browser.
  stop_delay: dt.timedelta = dt.timedelta()

  def __post_init__(self) -> None:
    if self.cool_down_time.total_seconds() < 0:
      raise ValueError(
          f"Timing.cool_down_time must be >= 0, but got: {self.cool_down_time}")
    if self.unit.total_seconds() <= 0:
      raise ValueError(f"Timing.unit must be > 0, but got {self.unit}")
    if self.timeout_unit:
      if self.timeout_unit.total_seconds() <= 0:
        raise ValueError(
            f"Timing.timeout_unit must be > 0, but got {self.timeout_unit}")
      if self.timeout_unit < self.unit:
        raise ValueError(f"Timing.unit must be <= Timing.timeout_unit: "
                         f"{self.unit} vs. {self.timeout_unit}")
    if self.run_timeout.total_seconds() < 0:
      raise ValueError(
          f"Timing.run_timeout, must be >= 0, but got {self.run_timeout}")

  def units(self, time: AnyTime, absolute_time: bool = False) -> int | float:
    """Convert absolute time (seconds, timedelta) to relative time units."""
    if isinstance(time, dt.timedelta):
      seconds = time.total_seconds()
    else:
      seconds = time
    if seconds < 0:
      raise ValueError(f"Unexpected negative time: {seconds}s")
    if absolute_time:
      return seconds
    return seconds / self.unit.total_seconds()

  def timedelta(self,
                time_units: AnyTimeUnit,
                absolute_time: bool = False) -> dt.timedelta:
    """Converts relative time units to absolute time."""
    return self._to_timedelta(time_units, self.unit, absolute_time)

  def timeout_timedelta(self,
                        time_units: AnyTimeUnit,
                        absolute_time: bool = False) -> dt.timedelta:
    """Converts relative time units to absolute time for timeouts.
    Note that timeouts can have a separate time unit."""
    if self.has_no_timeout:
      return SAFE_MAX_TIMEOUT_TIMEDELTA
    timeout_unit = self.timeout_unit or self.unit
    return self._to_timedelta(time_units, timeout_unit, absolute_time)

  def _to_timedelta(self,
                    time_units: AnyTimeUnit,
                    time_unit_duration: dt.timedelta,
                    absolute_time: bool = False) -> dt.timedelta:
    time_units_f: float | int = self._to_units_f(time_units)
    if absolute_time:
      absolute_time_f = dt.timedelta(seconds=time_units_f)
    else:
      absolute_time_f = time_units_f * time_unit_duration
    return self._to_safe_range(absolute_time_f)

  def _to_units_f(self, time_units: AnyTimeUnit) -> float | int:
    if isinstance(time_units, dt.timedelta):
      seconds = time_units.total_seconds()
    else:
      seconds = time_units
    assert isinstance(seconds, (float, int))
    if seconds < 0:
      raise ValueError(f"Time-units must be >= 0, but got {seconds}")
    return seconds

  def _to_safe_range(self, result: dt.timedelta) -> dt.timedelta:
    if result > SAFE_MAX_TIMEOUT_TIMEDELTA:
      return SAFE_MAX_TIMEOUT_TIMEDELTA
    return result

  @property
  def has_no_timeout(self) -> bool:
    return self.timeout_unit == dt.timedelta.max

  def to_json(self) -> dict[str, float]:
    return {
        "coolDownTime": self.cool_down_time.total_seconds(),
        "unit": self.unit.total_seconds(),
        "timeoutUnit": self.timeout_unit.total_seconds(),
        "runTimeout": self.run_timeout.total_seconds(),
    }
