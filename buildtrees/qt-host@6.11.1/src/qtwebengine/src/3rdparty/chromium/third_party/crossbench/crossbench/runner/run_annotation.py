# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
import enum
import logging
from typing import TYPE_CHECKING, Final, Iterable

from crossbench.helper import collection_helper

if TYPE_CHECKING:
  from crossbench.types import JsonDict


@enum.unique
class WarnLevel(enum.IntEnum):
  FATAL = 3
  ERROR = 2
  WARNING = 1
  INFO = 0


WARN_COLORS_LOOKUP: Final = {
    WarnLevel.FATAL: "❌",
    WarnLevel.ERROR: "❗",
    WarnLevel.WARNING: "🔶",
    WarnLevel.INFO: "🔵"
}

LOG_LEVEL_LOOKUP: Final = {
    WarnLevel.FATAL: logging.FATAL,
    WarnLevel.ERROR: logging.ERROR,
    WarnLevel.WARNING: logging.WARNING,
    WarnLevel.INFO: logging.INFO
}


@dataclasses.dataclass(frozen=True)
class RunAnnotation:
  message: str
  level: WarnLevel = WarnLevel.INFO

  @classmethod
  def fatal(cls, message: str) -> RunAnnotation:
    return cls(message, level=WarnLevel.FATAL)

  @classmethod
  def error(cls, message: str) -> RunAnnotation:
    return cls(message, level=WarnLevel.ERROR)

  @classmethod
  def warning(cls, message: str) -> RunAnnotation:
    return cls(message, level=WarnLevel.WARNING)

  @classmethod
  def info(cls, message: str) -> RunAnnotation:
    return cls(message, level=WarnLevel.INFO)

  def to_json(self) -> JsonDict:
    return {"message": self.message, "level": self.level.name}

  def log(self) -> None:
    logging.log(LOG_LEVEL_LOOKUP[self.level], "%s: %s",
                WARN_COLORS_LOOKUP[self.level], self.message)

  @classmethod
  def log_all(cls,
              annotations: Iterable[RunAnnotation],
              limit: int = 2) -> None:
    groups: dict[WarnLevel, list[RunAnnotation]] = collection_helper.group_by(
        annotations, lambda annotation: annotation.level)
    if not groups:
      return
    logging.info("RUN ANNOTATIONS:")
    for level in WarnLevel:
      if annotations_group := groups.get(level):
        for annotation in annotations_group[:limit]:
          annotation.log()
        skipped = len(annotations_group) - limit
        if skipped > 0:
          logging.info("   ... and %s more", skipped)
