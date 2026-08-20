# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import argparse
import dataclasses
import datetime as dt
from typing import Iterator

from crossbench.parse import DurationParser, NumberParser


class PlaybackController(abc.ABC):

  @classmethod
  def parse(cls, value: str) -> PlaybackController:
    if not value or value == "once":
      return cls.once()
    if value in ("inf", "infinity", "forever"):
      return cls.forever()
    if value[-1].isnumeric():
      raise argparse.ArgumentTypeError(
          f"Missing unit suffix: '{value}'\n"
          "Use 'x' for repetitions or time unit 's', 'm', 'h'")
    if value[-1] == "x":
      loops = NumberParser.positive_int(value[:-1], "Repeat-count")
      return cls.repeat(loops)
    duration = DurationParser.positive_duration(value)
    return cls.timeout(duration)

  @classmethod
  def default(cls) -> PlaybackController:
    return cls.once()

  @classmethod
  def once(cls) -> RepeatPlaybackController:
    return RepeatPlaybackController(1)

  @classmethod
  def repeat(cls, count: int) -> RepeatPlaybackController:
    return RepeatPlaybackController(count)

  @classmethod
  def forever(cls) -> PlaybackController:
    return ForeverPlaybackController()

  @classmethod
  def timeout(cls, duration: dt.timedelta) -> TimeoutPlaybackController:
    return TimeoutPlaybackController(duration)

  @abc.abstractmethod
  def __iter__(self) -> Iterator[int]:
    pass


@dataclasses.dataclass(frozen=True)
class ForeverPlaybackController(PlaybackController):

  def __iter__(self) -> Iterator[int]:
    i = 0
    while True:
      yield i
      i += 1


@dataclasses.dataclass(frozen=True)
class TimeoutPlaybackController(PlaybackController):
  duration : dt.timedelta

  def __iter__(self) -> Iterator[int]:
    end = dt.datetime.now() + self.duration
    yield 0
    if not self.duration:
      return
    i = 1
    while dt.datetime.now() <= end:
      yield i
      i += 1


@dataclasses.dataclass(frozen=True)
class RepeatPlaybackController(PlaybackController):
  count : int

  def __post_init__(self) -> None:
    NumberParser.positive_int(self.count, " page playback count")

  def __iter__(self) -> Iterator[int]:
    yield from range(self.count)
