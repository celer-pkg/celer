# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Iterable

from typing_extensions import override

from crossbench.runner.groups.base import RunGroup

if TYPE_CHECKING:
  from crossbench import exception
  from crossbench.browsers.browser import Browser
  from crossbench.probes.probe import Probe
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run


class BrowsersRunGroup(RunGroup):

  def __init__(self, story_groups: Iterable[StoriesRunGroup],
               throw: bool) -> None:
    super().__init__(throw)
    self._story_groups = tuple(story_groups)
    if not story_groups:
      raise ValueError("No story groups provided")
    self._set_path(self._story_groups[0].path.parents[1])

  @property
  def story_groups(self) -> Iterable[StoriesRunGroup]:
    return self._story_groups

  @property
  def browsers(self) -> Iterable[Browser]:
    for story_group in self._story_groups:
      yield story_group.browser

  @property
  def repetitions_groups(self) -> Iterable[RepetitionsRunGroup]:
    for story_group in self._story_groups:
      yield from story_group.repetitions_groups

  @property
  @override
  def runs(self) -> Iterable[Run]:
    for group in self._story_groups:
      yield from group.runs

  @property
  @override
  def info_stack(self) -> exception.TInfoStack:
    return ("Merging results from multiple browsers",)

  @override
  def _merge_probe_results(self, probe: Probe) -> ProbeResult:
    return probe.merge_browsers(self)
