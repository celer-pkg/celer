# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Self

from sqlalchemy import orm
from sqlalchemy import types as orm_types
from sqlalchemy.orm import Mapped
from sqlalchemy.sql import schema as orm_schema

from crossbench.results_db.records.base import BaseRecord
from crossbench.results_db.records.platform import PlatformRecord

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser


class BrowserRecord(BaseRecord):
  __tablename__ = "browser"

  @classmethod
  def create(cls, session, browser: Browser) -> Self:
    js_flags = ""
    if browser.attributes().is_chromium_based:
      js_flags = str(browser.js_flags)

    return cls(
        label=browser.label,
        name=browser.type_name(),
        path=str(browser.path),
        version=str(browser.version),
        channel=browser.version.channel_name,
        flags=str(browser.flags),
        js_flags=str(js_flags),
        driver="N/A",
        driver_version="N/A",
        driver_path=str(browser.driver_path),
        platform=session.query(PlatformRecord).filter(
            PlatformRecord.name == browser.platform.name).first(),
    )

  label: Mapped[str] = orm.mapped_column(orm_types.String(), primary_key=True)
  name: Mapped[str] = orm.mapped_column(orm_types.String())
  path: Mapped[str] = orm.mapped_column(orm_types.String())

  version: Mapped[str] = orm.mapped_column(orm_types.String())
  channel: Mapped[str] = orm.mapped_column(orm_types.String())

  flags: Mapped[str] = orm.mapped_column(orm_types.String())
  js_flags: Mapped[str] = orm.mapped_column(orm_types.String())

  driver: Mapped[str] = orm.mapped_column(orm_types.String())
  driver_version: Mapped[str] = orm.mapped_column(orm_types.String())
  driver_path: Mapped[str] = orm.mapped_column(orm_types.String())

  platform_label: Mapped[int] = orm.mapped_column(
      orm_schema.ForeignKey("platform.label"))
  platform: Mapped[PlatformRecord] = orm.relationship()

  # TODO: more settings
