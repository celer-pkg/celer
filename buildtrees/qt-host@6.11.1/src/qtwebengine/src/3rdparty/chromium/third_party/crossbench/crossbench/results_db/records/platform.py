# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Self

from sqlalchemy import orm
from sqlalchemy import types as orm_types
from sqlalchemy.orm import Mapped

from crossbench.results_db.records.base import BaseRecord

if TYPE_CHECKING:
  from crossbench.plt.base import Platform


class PlatformRecord(BaseRecord):
  __tablename__ = "platform"

  @classmethod
  def create(cls, session, platform: Platform) -> Self:
    del session
    os_details = platform.os_details()
    cpu_details = platform.cpu_details()
    python_details = platform.python_details()

    return cls(
        label=str(platform),
        name=platform.name,
        os_name=os_details["system"],
        os_version=os_details["release"],
        os_version_name=os_details["version"],
        cpu_architecture=str(platform.machine),
        cpu_physical_cores=cpu_details["physical cores"],
        cpu_logical_cores=cpu_details["logical cores"],
        cpu_max_frequency=cpu_details.get("max frequency", "N/A"),
        cpu_min_frequency=cpu_details.get("min frequency", "N/A"),
        hw_model=platform.device,
        python_version=python_details["version"])

  label: Mapped[str] = orm.mapped_column(orm_types.String(), primary_key=True)
  name: Mapped[str] = orm.mapped_column(orm_types.String())

  os_name: Mapped[str] = orm.mapped_column(orm_types.String())
  os_version: Mapped[str] = orm.mapped_column(orm_types.String())
  os_version_name: Mapped[str] = orm.mapped_column(orm_types.String())

  cpu_architecture: Mapped[str] = orm.mapped_column(orm_types.String())
  cpu_physical_cores: Mapped[int] = orm.mapped_column(orm_types.Integer())
  cpu_logical_cores: Mapped[int] = orm.mapped_column(orm_types.Integer())
  cpu_max_frequency: Mapped[str] = orm.mapped_column(orm_types.String())
  cpu_min_frequency: Mapped[str] = orm.mapped_column(orm_types.String())

  hw_model: Mapped[str] = orm.mapped_column(orm_types.String())

  python_version: Mapped[str] = orm.mapped_column(orm_types.String())
