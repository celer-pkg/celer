# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import json
import logging
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.probes.internal.base import (InternalJsonResultProbe,
                                             InternalJsonResultProbeContext)
from crossbench.probes.results import EmptyProbeResult

if TYPE_CHECKING:
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.cache_temperatures import \
      CacheTemperaturesRunGroup
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.types import Json, JsonDict


class ResultsSummaryProbe(InternalJsonResultProbe):
  """
  Runner-internal meta-probe: Collects a summary results.json with all the Run
  information, including all paths to the results of all attached Probes.
  """
  NAME = "cb.results"
  # Given that this is  a meta-Probe that summarizes the data from other
  # probes we exclude it from the default results lists.
  PRODUCES_DATA = False

  @property
  @override
  def is_attached(self) -> bool:
    return True

  @override
  def merge_cache_temperatures(self,
                               group: CacheTemperaturesRunGroup) -> ProbeResult:
    # If session setup failed, the results will not have been initialized.
    return group.first_run.results.get(self, EmptyProbeResult())

  @override
  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    repetitions: list[JsonDict] = []
    browser: JsonDict | None = None

    has_empty_results = False
    for run in group.runs:
      if run.results[self].is_empty:
        has_empty_results = True
        continue
      source_file = run.results[self].json
      assert source_file.is_file()
      with source_file.open(encoding="utf-8") as f:
        repetition_data = json.load(f)
      if browser is None:
        browser = repetition_data["browser"]
        del browser["log"]
      repetition_summary: JsonDict = {
          "cwd": repetition_data["cwd"],
          "probes": repetition_data["probes"],
          "success": repetition_data["success"],
          "errors": repetition_data["errors"],
      }
      repetitions.append(repetition_summary)

    merged_data: JsonDict = {
        "cwd": str(group.path),
        "story": group.story.details_json(),
        "browser": browser,
        "group": group.info,
        "repetitions": repetitions,
        "probes": group.results.to_json(),
        "success": group.is_success,
        "errors": group.exceptions.error_messages(),
    }
    if has_empty_results:
      logging.error("Probe %s produced empty results for some runs.", self.NAME)
    return self.write_group_result(group, merged_data, csv_formatter=None)

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    stories: JsonDict = {}
    browser = None

    for repetitions_group in group.repetitions_groups:
      source_file = repetitions_group.results[self].json
      assert source_file.is_file()
      with source_file.open(encoding="utf-8") as f:
        merged_story_data = json.load(f)
      if browser is None:
        browser = merged_story_data["browser"]
      story_info = merged_story_data["story"]
      stories[story_info["name"]] = {
          "cwd": merged_story_data["cwd"],
          "duration": story_info["duration"],
          "probes": merged_story_data["probes"],
          "errors": merged_story_data["errors"],
      }

    merged_data: JsonDict = {
        "cwd": str(group.path),
        "browser": browser,
        "stories": stories,
        "group": group.info,
        "probes": group.results.to_json(),
        "success": group.is_success,
        "errors": group.exceptions.error_messages(),
    }
    return self.write_group_result(group, merged_data, csv_formatter=None)

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    browsers: JsonDict = {}
    for story_group in group.story_groups:
      source_file = story_group.results[self].json
      assert source_file.is_file()
      with source_file.open(encoding="utf-8") as f:
        merged_browser_data = json.load(f)
      browser_info = merged_browser_data["browser"]
      browsers[browser_info["unique_name"]] = {
          "cwd": merged_browser_data["cwd"],
          "probes": merged_browser_data["probes"],
          "errors": merged_browser_data["errors"],
      }

    merged_data: JsonDict = {
        "cwd": str(group.path),
        "browsers": browsers,
        "probes": group.results.to_json(),
        "success": group.is_success,
        "errors": group.exceptions.error_messages(),
    }
    return self.write_group_result(group, merged_data, csv_formatter=None)

  @override
  def get_context_cls(self) -> Type[InternalJsonResultProbeContext]:
    return ResultsSummaryProbeContext


class ResultsSummaryProbeContext(InternalJsonResultProbeContext):

  @override
  def to_json(self, actions: Actions) -> Json:
    return self.run.details_json()
