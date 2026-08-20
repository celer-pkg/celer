# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
import math
import time
from typing import TYPE_CHECKING, Iterator, Optional

if TYPE_CHECKING:
  from crossbench.runner.timing import AnyTime, AnyTimeUnit


def as_timedelta(value: int | float | dt.timedelta) -> dt.timedelta:
  if isinstance(value, dt.timedelta):
    return value
  return dt.timedelta(seconds=value)


class WaitRange:
  """
  Create wait/sleep ranges with the given parameters:

  If present we start with the initial delay, and then exponentially
  increase the sleep/wait time by the given factor, until we reach the max
  sleep time.

  | delay | min | min * factor | ... | min * factor ** N |  ... | max |
  | --------------------------- timeout ------------------------------|
  | i=0   | i=1 | i=2          | ............... | i=max_iterations-1 |

  The timeout puts an upper bound to the total sleep time when using
  wait_with_backoff().
  """

  def __init__(
      self,
      min: AnyTime = 0.1,  # pylint: disable=redefined-builtin
      timeout: AnyTime = 10,
      factor: float = 1.01,
      max: Optional[AnyTime] = None,  # pylint: disable=redefined-builtin
      max_iterations: int | float = math.inf,
      delay: AnyTime = 0) -> None:
    self._min: dt.timedelta = as_timedelta(min)
    assert self._min.total_seconds() > 0
    if not max:
      self._max: dt.timedelta = self._min * 10
    else:
      self._max = as_timedelta(max)
    assert self._min <= self._max
    assert 1.0 < factor
    self._factor: float = factor
    self._timeout: dt.timedelta = as_timedelta(timeout)
    assert 0 < self._timeout.total_seconds()
    self._delay = as_timedelta(delay)
    assert self._delay <= self._timeout
    assert max_iterations > 0
    self._max_iterations: int | float = max_iterations

  @property
  def timeout(self) -> dt.timedelta:
    return self._timeout

  def __iter__(self) -> Iterator[tuple[int, dt.timedelta]]:
    i = 0
    if self._delay:
      yield i, self._delay
      i += 1

    current_sleep = self._min
    while True:
      if self._max_iterations <= i:
        break
      yield i, current_sleep
      current_sleep = min(current_sleep * self._factor, self._max)
      i += 1

  def wait_with_backoff(
      self,) -> Iterator[tuple[int, dt.timedelta, dt.timedelta]]:
    start = dt.datetime.now()
    timeout = self._timeout
    for i, sleep_for in self:
      duration = dt.datetime.now() - start
      if duration > self._timeout:
        raise TimeoutError(f"Waited for {duration}")
      time_left = timeout - duration
      yield i, duration, time_left
      sleep_f(sleep_for.total_seconds())


def sleep(seconds: AnyTimeUnit) -> None:
  if isinstance(seconds, dt.timedelta):
    seconds = seconds.total_seconds()
  sleep_f(seconds)


def sleep_f(seconds: float) -> None:
  if seconds == 0:
    return
  logging.debug("WAIT %ss", seconds)
  time.sleep(seconds)


def wait_with_backoff(
    wait_range: AnyTime | WaitRange,
) -> Iterator[tuple[int, dt.timedelta, dt.timedelta]]:
  if not isinstance(wait_range, WaitRange):
    wait_range = WaitRange(timeout=wait_range)
  return wait_range.wait_with_backoff()
