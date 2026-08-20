# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
from typing import TYPE_CHECKING, Iterable

from typing_extensions import override

from crossbench.benchmarks.loading.page.base import Page
from crossbench.benchmarks.loading.playback_controller import \
    PlaybackController
from crossbench.benchmarks.loading.tab_controller import TabController

if TYPE_CHECKING:
  from crossbench.action_runner.base import ActionRunner
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict


class CombinedPage(Page):

  def __init__(
      self,
      pages: Iterable[Page],
      name: str = "combined",
      playback: PlaybackController = PlaybackController.default(),
      tabs: TabController = TabController.default(),
      about_blank_duration: dt.timedelta = dt.timedelta()
  ) -> None:
    self._pages = tuple(pages)
    assert self._pages, "No sub-pages provided for CombinedPage"
    assert len(self._pages) >= 1, "Combined Page needs at least one page"
    self._tabs = tabs

    duration = dt.timedelta()
    for page in self._pages:
      page.set_parent(self)
      duration += page.duration
    super().__init__(name, duration, playback, tabs, about_blank_duration)
    self.url = None

  @property
  @override
  def tabs(self) -> TabController:
    return self._tabs

  @property
  def pages(self) -> Iterable[Page]:
    return self._pages

  @property
  @override
  def substories(self) -> tuple[str, ...]:
    return tuple(
        substory for page in self._pages for substory in page.substories)

  @property
  @override
  def first_url(self) -> str:
    return self._pages[0].first_url

  @override
  def details_json(self) -> JsonDict:
    result = super().details_json()
    result["pages"] = list(page.details_json() for page in self._pages)
    return result

  @override
  def teardown(self, run: Run) -> None:
    for page in self._pages:
      page.teardown(run)

  @override
  def setup(self, run: Run) -> None:
    for page in self.pages:
      page.setup(run)

  def run_once(self, run: Run) -> None:
    action_runner = run.action_runner
    multiple_tabs = self.tabs.multiple_tabs
    action_runner.run_combined_page(run, self, multiple_tabs)

  @override
  def run_with(self, run: Run, action_runner: ActionRunner,
               multiple_tabs: bool) -> None:
    action_runner.run_combined_page(run, self, multiple_tabs)

  def __str__(self) -> str:
    combined_name = ",".join(page.name for page in self._pages)
    return f"CombinedPage({combined_name})"
