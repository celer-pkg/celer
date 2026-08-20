# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import collections
import logging
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.probes.chromium_probe import ChromiumProbe
from crossbench.probes.probe_context import ProbeContext
from crossbench.probes.probe_error import ProbeMissingDataError
from crossbench.probes.results import LocalProbeResult, ProbeResult

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.path import LocalPath
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.repetitions import (
      CacheTemperatureRepetitionsRunGroup, RepetitionsRunGroup)
  from crossbench.runner.groups.stories import StoriesRunGroup


class V8RCSProbe(ChromiumProbe):
  """
  Chromium-only Probe to extract runtime-call-stats data that can be used
  to analyze precise counters and time spent in various VM components in V8:
  https://v8.dev/tools/head/callstats.html
  """
  NAME = "v8.rcs"

  @override
  def attach(self, browser: Browser) -> None:
    super().attach(browser)
    browser.js_flags.update(("--runtime-call-stats", "--allow-natives-syntax"))

  @override
  def get_context_cls(self) -> Type[V8RCSProbeContext]:
    return V8RCSProbeContext

  def concat_group_files(self, group: RepetitionsRunGroup
                         | CacheTemperatureRepetitionsRunGroup,
                         file_name: str) -> LocalPath:
    result_dir = group.get_local_probe_result_dir(self)
    result_files = (run.results[self].file for run in group.runs)
    result_file = self.host_platform.concat_files(
        inputs=result_files,
        output=result_dir / file_name,
        prefix=f"\n== Page: {group.story.name}\n")
    return result_file

  @override
  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    all_file = self.concat_group_files(group, "all.rcs.txt")
    result_files = [all_file]
    for temperature_group in group.cache_temperature_repetitions_groups:
      temperature_file_name = f"{temperature_group.cache_temperature}.rcs.txt"
      group_file = self.concat_group_files(temperature_group,
                                           temperature_file_name)
      result_files.append(group_file)
    result_dir = group.get_local_probe_result_dir(self)
    self.host_platform.symlink_or_copy(all_file,
                                       result_dir.with_suffix(".rcs.txt"))
    return LocalProbeResult(file=tuple(result_files))

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    name_groups = collections.defaultdict(list)
    for repetition_group in group.repetitions_groups:
      for result_file in repetition_group.results[self].file_list:
        name_groups[result_file.name].append(result_file)

    result_dir = group.get_local_probe_result_dir(self)
    result_files = []
    for name, files in name_groups.items():
      result_files.append(
          self.host_platform.concat_files(
              inputs=files, output=result_dir / name))
    src_file = result_dir / "all.rcs.txt"
    self.host_platform.symlink_or_copy(src_file,
                                       result_dir.with_suffix(".rcs.txt"))
    return LocalProbeResult(file=(src_file,))

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    # We put all the fils by in a toplevel v8.rcs folder
    result_dir = group.get_local_probe_result_dir(self)
    files = []
    for story_group in group.story_groups:
      story_group_file = story_group.results[self].file
      # Be permissive and skip failed probes
      if not story_group_file.exists():
        logging.info("Probe %s: skipping non-existing results file: %s",
                     self.NAME, story_group_file)
        continue
      dest_file = result_dir / f"{story_group.browser.unique_name}.rcs.txt"
      self.host_platform.symlink_or_copy(story_group_file, dest_file)
      files.append(dest_file)
    return LocalProbeResult(file=files)

  @override
  def log_browsers_result(self, group: BrowsersRunGroup) -> None:
    if self not in group.results:
      return
    logging.info("-" * 80)
    logging.critical(
        "V8 RCS results: open on  http://v8.dev/tools/head/callstats.html")
    for file in group.results[self].get_all("txt"):
      logging.critical("    %s", file)
    logging.info("- " * 40)


class V8RCSProbeContext(ProbeContext[V8RCSProbe]):
  _rcs_table: str | None = None

  @override
  def setup(self) -> None:
    pass

  def start(self) -> None:
    pass

  def stop(self) -> None:
    with self.run.actions("Extract RCS") as actions:
      self._rcs_table = actions.js("return %GetAndResetRuntimeCallStats();")

  def teardown(self) -> ProbeResult:
    if not self._rcs_table:
      raise ProbeMissingDataError(
          "Chrome didn't produce any RCS data. "
          "Use Chrome Canary or make sure to enable the "
          "v8_enable_runtime_call_stats compile-time flag.")
    rcs_file = self.local_result_path.with_suffix(".rcs.txt")
    with rcs_file.open("a") as f:
      f.write(self._rcs_table)
    return LocalProbeResult(file=(rcs_file,))
