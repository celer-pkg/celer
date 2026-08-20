# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging


class DurationMeasureContext:

  def __init__(self, durations: Durations, name: str) -> None:
    self._start_time = dt.datetime.utcfromtimestamp(0)
    self._durations = durations
    self._name = name

  def __enter__(self) -> DurationMeasureContext:
    self._start_time = dt.datetime.now()
    return self

  def __exit__(self, exc_type, exc_value, traceback) -> None:
    assert self._start_time
    delta = dt.datetime.now() - self._start_time
    self._durations[self._name] = delta


class Durations:
  """
  Helper object to track durations.
  """

  def __init__(self) -> None:
    self._durations: dict[str, dt.timedelta] = {}

  def __getitem__(self, name: str) -> dt.timedelta:
    return self._durations[name]

  def __setitem__(self, name: str, duration: dt.timedelta) -> None:
    assert name not in self._durations, f"Cannot set '{name}' duration twice!"
    self._durations[name] = duration

  def __len__(self) -> int:
    return len(self._durations)

  def measure(self, name: str) -> DurationMeasureContext:
    assert name not in self._durations, (
        f"Cannot measure '{name}' duration twice!")
    return DurationMeasureContext(self, name)

  def to_json(self) -> dict[str, float]:
    return {
        name: self._durations[name].total_seconds()
        for name in sorted(self._durations.keys())
    }


class TimeScope:
  """
  Measures and logs the time spend during the lifetime of the TimeScope.
  """

  def __init__(self, message: str, level: int = 3) -> None:
    self._message = message
    self._level = level
    self._start: dt.datetime | None = None
    self._duration: dt.timedelta = dt.timedelta()

  @property
  def message(self) -> str:
    return self._message

  @property
  def duration(self) -> dt.timedelta:
    return self._duration

  def __enter__(self) -> TimeScope:
    self._start = dt.datetime.now()
    return self

  def __exit__(self, exc_type, exc_value, exc_traceback) -> None:
    assert self._start
    self._duration = dt.datetime.now() - self._start
    logging.log(self._level, "%s duration=%s", self._message, self._duration)
