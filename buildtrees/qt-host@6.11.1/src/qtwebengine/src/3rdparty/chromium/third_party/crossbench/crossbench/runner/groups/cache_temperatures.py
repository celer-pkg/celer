# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Iterable

from typing_extensions import override

from crossbench.helper import collection_helper
from crossbench.runner.groups.base import RunGroup

if TYPE_CHECKING:
  from crossbench import exception
  from crossbench.browsers.browser import Browser
  from crossbench.probes.probe import Probe
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run
  from crossbench.stories.story import Story
  from crossbench.types import JsonDict, JsonMapping


class CacheTemperaturesRunGroup(RunGroup):
  """
  A group of Run objects with different cache temperatures for the same Story
  with same browser and same repetition.
  """

  @classmethod
  def groups(cls,
             runs: Iterable[Run],
             throw: bool = False) -> tuple[CacheTemperaturesRunGroup, ...]:
    return tuple(
        collection_helper.group_by(
            runs,
            key=lambda run: (run.story, run.browser, run.repetition),
            group=lambda _: cls(throw),
            sort_key=None).values())

  def __init__(self, throw: bool = False) -> None:
    super().__init__(throw)
    self._runs: list[Run] = []
    self._story: Story | None = None
    self._browser: Browser | None = None
    self._repetition = -1
    self._cache_temperature = ""

  def append(self, run: Run) -> None:
    if self._path is None:
      self._set_path(run.group_dir)
      self._story = run.story
      self._browser = run.browser
      self._repetition = run.repetition
    assert self._story == run.story
    assert self._path == run.group_dir
    assert self._browser == run.browser
    assert self._repetition == run.repetition
    self._runs.append(run)

  @property
  @override
  def runs(self) -> Iterable[Run]:
    return iter(self._runs)

  @property
  def first_run(self) -> Run:
    return self._runs[0]

  @property
  def repetition(self) -> int:
    return self._repetition

  @property
  def story(self) -> Story:
    assert self._story
    return self._story

  @property
  def browser(self) -> Browser:
    assert self._browser
    return self._browser

  @property
  @override
  def info_stack(self) -> exception.TInfoStack:
    return (
        "Merging results from multiple cache temperatures",
        f"browser={self.browser.unique_name}",
        f"story={self.story}",
        f"repetition={self.repetition}",
    )

  @property
  @override
  def info(self) -> JsonMapping:
    info: JsonDict = {
        "story": str(self.story),
        "repetition": self.repetition,
    }
    info.update(super().info)
    return info

  @override
  def _merge_probe_results(self, probe: Probe) -> ProbeResult:
    return probe.merge_cache_temperatures(self)
