# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.probes.internal.base import (InternalJsonResultProbe,
                                             InternalJsonResultProbeContext)
from crossbench.probes.metric import MetricsMerger

if TYPE_CHECKING:
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.types import Json


class DurationsProbe(InternalJsonResultProbe):
  """
  Runner-internal meta-probe: Collects timing information for various components
  of the runner (and the times spent in individual stories as well).
  """
  NAME = "cb.durations"

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        (repetitions_group.results[self].json
         for repetitions_group in group.repetitions_groups),
        merge_duplicate_paths=True)
    return self.write_group_result(group, merged, csv_formatter=None)

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        (story_group.results[self].json for story_group in group.story_groups),
        merge_duplicate_paths=True)
    return self.write_group_result(group, merged, csv_formatter=None)

  @override
  def get_context_cls(self) -> Type[InternalJsonResultProbeContext]:
    return DurationsProbeContext


class DurationsProbeContext(InternalJsonResultProbeContext):

  @override
  def to_json(self, actions: Actions) -> Json:
    return self.run.durations.to_json()
