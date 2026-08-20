# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import json
from typing import TYPE_CHECKING, Iterable, Self, Type

from typing_extensions import override

from crossbench.probes.internal.base import (InternalJsonResultProbe,
                                             InternalJsonResultProbeContext)
from crossbench.probes.probe_context import ProbeSessionContext
from crossbench.probes.results import EmptyProbeResult

if TYPE_CHECKING:
  from crossbench.probes.results import ProbeResult, ProbeResultDict
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.base import RunGroup
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.groups.session import BrowserSessionRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.types import Json, JsonList


class ErrorsProbe(InternalJsonResultProbe):
  """
  Runner-internal meta-probe: Collects all errors from running stories and/or
  from merging probe data.
  """
  NAME = "cb.errors"

  @override
  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    return self._merge_group(group, (run.results for run in group.runs))

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    return self._merge_group(
        group, (rep_group.results for rep_group in group.repetitions_groups))

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self._merge_group(
        group, (story_group.results for story_group in group.story_groups))

  def _merge_group(self, group: RunGroup,
                   results_iter: Iterable[ProbeResultDict]) -> ProbeResult:
    merged_errors: JsonList = []

    for results in results_iter:
      result = results[self]
      if not result:
        continue
      source_file = result.json
      assert source_file.is_file()
      with source_file.open(encoding="utf-8") as f:
        repetition_errors = json.load(f)
        assert isinstance(repetition_errors, list)
        merged_errors.extend(repetition_errors)

    group_errors = group.exceptions.to_json()
    assert isinstance(group_errors, list)
    merged_errors.extend(group_errors)

    if not merged_errors:
      return EmptyProbeResult()
    return self.write_group_result(group, merged_errors, csv_formatter=None)

  @override
  def get_context_cls(self) -> Type[ErrorsProbeContext]:
    return ErrorsProbeContext

  @override
  def get_session_context(self: Self, session: BrowserSessionRunGroup):
    return ErrorProbeSessionContext(self, session)


class ErrorsProbeContext(InternalJsonResultProbeContext):

  @override
  def to_json(self, actions: Actions) -> Json:
    return self.run.exceptions.to_json()


class ErrorProbeSessionContext(ProbeSessionContext):

  @override
  def start(self) -> None:
    # Only extract data in the late teardown phase.
    pass

  @override
  def stop(self) -> None:
    # Only extract data in the late teardown phase.
    pass

  @override
  def teardown(self) -> ProbeResult:
    if self.session.is_success:
      return self.empty_result()
    # Use custom name for the session to not clash with the run errors.
    result_path = self.local_result_path.parent / "cb.session.errors.json"
    with result_path.open("w") as f:
      json.dump(self.session.exceptions.to_json(), f)
    return self.local_result(json=(result_path,))
