# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Optional, Self, Type

from typing_extensions import override

from crossbench.parse import ObjectParser
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.metric import MetricsMerger
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.probes.probe import ProbeConfigParser, ProbeKeyT
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.types import Json


def parse_javascript(value: str) -> str:
  # TODO: maybe add more sanity checks
  return ObjectParser.non_empty_str(value, name="javascript")


class JSProbe(JsonResultProbe):
  """
  Probe for extracting arbitrary metrics using custom javascript code.
  """
  NAME = "js"
  RESULT_LOCATION = ResultLocation.LOCAL
  IS_GENERAL_PURPOSE = True

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "setup",
        type=parse_javascript,
        help=(
            "Optional JavaScript code that is run immediately before a story. "
            "This can be used for setting up some JS tracking code or patch "
            "existing code for custom metric tracking."))
    parser.add_argument(
        "js",
        type=parse_javascript,
        required=True,
        help=("Required JavaScript code that is run immediately after "
              "a story has finished. The code must return a JS object with "
              "(nested) metric values (numbers)."))
    return parser

  def __init__(self, js: str, setup: Optional[str] = None) -> None:
    super().__init__()
    self._setup_js = setup
    self._metric_js = js

  @property
  def setup_js(self) -> Optional[str]:
    return self._setup_js

  @property
  def metric_js(self) -> str:
    return self._metric_js

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (
        ("setup_js", self._setup_js),
        ("metric_js", self._metric_js),
    )

  def get_context_cls(self) -> Type[JSProbeContext]:
    return JSProbeContext

  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        story_group.results[self].json
        for story_group in group.repetitions_groups)
    return self.write_group_result(group, merged)

  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self.merge_browsers_json_list(group).merge(
        self.merge_browsers_csv_list(group))


class JSProbeContext(JsonResultProbeContext[JSProbe]):

  @override
  def to_json(self, actions: Actions) -> Json:
    data = actions.js(self.probe.metric_js)
    return ObjectParser.non_empty_dict(data, "JS metric data")

  def start(self) -> None:
    if setup_js := self.probe.setup_js:
      with self.run.actions(f"Probe({self.probe.name}) setup") as actions:
        actions.js(setup_js)
