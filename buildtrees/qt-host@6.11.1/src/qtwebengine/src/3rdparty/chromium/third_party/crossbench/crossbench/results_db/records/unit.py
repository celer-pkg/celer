# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum

from sqlalchemy import orm
from sqlalchemy import types as orm_types
from sqlalchemy.orm import Mapped

from crossbench.results_db.records.base import BaseRecord


class ImprovementDirection(enum.IntEnum):
  UP = 1
  DOWN = -1


_UNIT_PRESETS = (
    # (NAME, UNIT, DIRECTION, HELP)
    ("ms", "ms", ImprovementDirection.DOWN, "Duration in milliseconds."),
    ("ms+", "ms", ImprovementDirection.UP, "Duration in milliseconds."),
    ("score", None, ImprovementDirection.UP, "Unitless score."),
    ("KiB", "KiB", ImprovementDirection.DOWN, "Size in kilobytes."),
)


class UnitRecord(BaseRecord):
  __tablename__ = "unit"

  name: Mapped[str] = orm.mapped_column(orm_types.String(), primary_key=True)
  unit: Mapped[str] = orm.mapped_column(orm_types.String(), nullable=True)
  improvement_direction: Mapped[ImprovementDirection] = orm.mapped_column(
      orm_types.Enum(ImprovementDirection))
  help: Mapped[str | None] = orm.mapped_column(
      orm_types.String(), nullable=True)

  @classmethod
  def create_defaults(cls, session):
    for name, unit, direction, help_str in _UNIT_PRESETS:
      if direction is ImprovementDirection.UP:
        help_str = f"{help_str} Bigger is better."
      elif direction is ImprovementDirection.DOWN:
        help_str = f"{help_str} Less is better."
      else:
        raise ValueError("Unknown improvement direction")
      help_str = help_str.strip()

      session.add(
          cls(name=name,
              unit=unit,
              improvement_direction=direction,
              help=help_str))
