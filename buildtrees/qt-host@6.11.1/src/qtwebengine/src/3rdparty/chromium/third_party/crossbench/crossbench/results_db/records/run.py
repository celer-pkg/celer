# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
from typing import TYPE_CHECKING, Self

from sqlalchemy import orm
from sqlalchemy import types as orm_types
from sqlalchemy.orm import Mapped
from sqlalchemy.sql import schema as orm_schema

from crossbench.results_db.records.base import BaseRecord
from crossbench.results_db.records.browser import BrowserRecord

if TYPE_CHECKING:
  from crossbench.runner.run import Run


class RunRecord(BaseRecord):
  __tablename__ = "run"

  @classmethod
  def create(cls, session, run: Run) -> Self:
    return cls(
        index=run.index,
        repetition=run.repetition,
        cache_temperature=str(run.temperature),
        name=run.name,
        browser=session.query(BrowserRecord).filter(
            BrowserRecord.label == run.browser.label).first(),
        story=run.story.name)

  index: Mapped[int] = orm.mapped_column(orm_types.Integer(), primary_key=True)
  repetition: Mapped[int] = orm.mapped_column(orm_types.Integer())
  cache_temperature: Mapped[str] = orm.mapped_column(orm_types.String())
  name: Mapped[str] = orm.mapped_column(orm_types.String())

  browser_label: Mapped[int] = orm.mapped_column(
      orm_schema.ForeignKey("browser.label"))
  browser: Mapped[BrowserRecord] = orm.relationship()

  story: Mapped[str] = orm.mapped_column(orm_types.String())

  error_count: Mapped[int] = orm.mapped_column(
      orm_types.Integer(), nullable=True)
  errors: Mapped[orm_types.JSON] = orm.mapped_column(
      orm_types.JSON(), nullable=True)

  start_datetime: Mapped[dt.datetime] = orm.mapped_column(
      orm_types.DateTime(), nullable=True)
  durations: Mapped[orm_types.JSON] = orm.mapped_column(
      orm_types.JSON(), nullable=True)
