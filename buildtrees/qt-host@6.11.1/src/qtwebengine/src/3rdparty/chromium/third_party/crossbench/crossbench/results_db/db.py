# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import contextlib
import logging
from typing import TYPE_CHECKING, Final, Iterable, Optional, Type

import sqlalchemy
import sqlalchemy.engine as orm_engine
import sqlalchemy.event as orm_event
from sqlalchemy import orm

from crossbench.results_db.records.browser import BrowserRecord
from crossbench.results_db.records.platform import PlatformRecord
from crossbench.results_db.records.run import RunRecord
from crossbench.results_db.records.unit import UnitRecord

if TYPE_CHECKING:
  from crossbench import path as pth
  from crossbench.browsers.browser import Browser
  from crossbench.plt.base import Platform
  from crossbench.results_db.records.base import BaseRecord
  from crossbench.runner.run import Run


DEFAULT_CLASSES: Final[tuple[Type[BaseRecord],
                             ...]] = (PlatformRecord, BrowserRecord, RunRecord,
                                      UnitRecord)


@orm_event.listens_for(orm_engine.Engine, "connect")
def set_sqlite_pragma(dbapi_connection, connection_record):
  """sqlite needs manual foreign key setup"""
  del connection_record
  cursor = dbapi_connection.cursor()
  cursor.execute("PRAGMA foreign_keys = ON;")
  cursor.execute("PRAGMA journal_mode = WAL;")
  cursor.execute("PRAGMA synchronous = NORMAL;")
  cursor.close()


class ResultsDB:

  def __init__(self, db_file: Optional[pth.LocalPath] = None):
    self._db_file: Optional[pth.LocalPath] = db_file
    init_tables: bool = True
    engine_url: str = "sqlite:///:memory:"
    if db_file:
      init_tables = not db_file.exists()
      engine_url = f"sqlite:///{self._db_file}"
    is_debug_logging = logging.getLogger().isEnabledFor(logging.DEBUG)
    self._engine = sqlalchemy.create_engine(engine_url, echo=is_debug_logging)
    if init_tables:
      self._init_tables()

  def _init_tables(self) -> None:
    # We don't use BaseRecord.metadata.create_all(self._engine) so every probe
    # can lazily initialize tables depending if they're used or not.
    for cls in DEFAULT_CLASSES:
      cls.__table__.create(self._engine)  # type: ignore
    with self.session() as session:
      UnitRecord.create_defaults(session)
      session.commit()

  @property
  def is_in_memory(self) -> bool:
    return not self._db_file

  @property
  def db_file(self) -> pth.LocalPath:
    if not self._db_file:
      raise RuntimeError("In-memory ResultDB has no DB file.")
    return self._db_file

  @property
  def engine(self) -> sqlalchemy.engine.Engine:
    return self._engine

  @contextlib.contextmanager
  def session(self):
    with orm.Session(self._engine) as session:
      yield session

  def setup_runs(self, runs: list[Run]) -> None:
    platforms = {run.browser_platform for run in runs}
    self.add_platforms(platforms)
    browsers = {run.browser for run in runs}
    self.add_browsers(browsers)
    self.add_runs(runs)

  def add_runs(self, runs: list[Run]) -> None:
    with self.session() as session:
      for run in runs:
        record = RunRecord.create(session, run)
        session.add(record)
      session.commit()

  def add_platforms(self, platforms: Iterable[Platform]) -> None:
    with self.session() as session:
      for platform in set(platforms):
        record = PlatformRecord.create(session, platform)
        session.add(record)
      session.commit()

  def add_browsers(self, browsers: Iterable[Browser]) -> None:
    with self.session() as session:
      for browser in set(browsers):
        record = BrowserRecord.create(session, browser)
        session.add(record)
      session.commit()

  def teardown_run(self, run: Run) -> None:
    """Update run entries"""
    with self.session() as session:
      run_record = session.get(RunRecord, run.index)
      assert run_record, f"Could not find run {run.index}"
      run_record.error_count = len(run.exceptions)
      run_record.errors = run.exceptions.to_json()
      run_record.start_datetime = run.start_datetime
      run_record.durations = run.durations.to_json()
      session.add(run_record)
      session.commit()
