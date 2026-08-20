# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import itertools
import json
import logging
from typing import TYPE_CHECKING, Any, MutableMapping, Optional, Type

from typing_extensions import override

from crossbench.action_runner.action.enums import ReadyState
from crossbench.benchmarks.benchmark_probe import BenchmarkProbeMixin
from crossbench.benchmarks.motionmark.base import MotionMarkBenchmark
from crossbench.helper import url_helper
from crossbench.probes.helper import Flatten
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.metric import Metric, MetricsMerger
from crossbench.stories.press_benchmark import PressBenchmarkStory

if TYPE_CHECKING:
  from crossbench.path import LocalPath
  from crossbench.probes.results import ProbeResult, ProbeResultDict
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run
  from crossbench.types import Json


def _clean_up_path_segments(path: tuple[str, ...]) -> Optional[str]:
  name = path[-1]
  if name.startswith("segment") or name == "data":
    return None
  if path[:2] == ("testsResults", "MotionMark"):
    path = path[2:]
  return "/".join(path)


class MotionMark1Probe(BenchmarkProbeMixin, JsonResultProbe, abc.ABC):
  """
  MotionMark-specific Probe.
  Extracts all MotionMark times and scores.
  """

  @abc.abstractmethod
  @override
  def get_context_cls(self) -> Type[MotionMark1ProbeContext]:
    pass

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        story_group.results[self].json
        for story_group in group.repetitions_groups)
    return self.write_group_result(group, merged)

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self.merge_browsers_json_list(group).merge(
        self.merge_browsers_csv_list(group))

  @override
  def log_run_result(self, run: Run) -> None:
    self._log_result(run.results, single_result=True)

  @override
  def log_browsers_result(self, group: BrowsersRunGroup) -> None:
    self._log_result(group.results, single_result=False)

  def _log_result(self, result_dict: ProbeResultDict,
                  single_result: bool) -> None:
    if self not in result_dict:
      return
    results_json: LocalPath = result_dict[self].json
    logging.info("-" * 80)
    logging.critical("Motionmark results:")
    if not single_result:
      logging.critical("  %s", result_dict[self].csv)
    logging.info("- " * 40)

    with results_json.open(encoding="utf-8") as f:
      data = json.load(f)
      if single_result:
        score = data.get("score") or data["Score"]
        logging.critical("Score %s", score)
      else:
        self._log_result_metrics(data)

  @override
  def _extract_result_metrics_table(self, metrics: dict[str, Any],
                                    table: dict[str, list[str]]) -> None:
    for metric_key, metric in metrics.items():
      if not self._valid_metric_key(metric_key):
        continue
      table[metric_key].append(
          Metric.format(metric["average"], metric["stddev"]))
    # Separate runs don't produce a score
    if total_metric := metrics.get("score") or metrics.get("Score"):
      table["Score"].append(
          Metric.format(total_metric["average"], total_metric["stddev"]))

  def _valid_metric_key(self, metric_key: str) -> bool:
    parts = metric_key.split("/")
    return len(parts) == 2 or parts[-1] == "score"


class MotionMark1ProbeContext(JsonResultProbeContext):
  JS = """
    return window.benchmarkRunnerClient.results.results;
  """

  @override
  def to_json(self, actions: Actions) -> Json:
    return actions.js(self.JS)

  @override
  def flatten_json_data(self, json_data: list) -> Json:
    assert isinstance(json_data, list) and len(json_data) == 1, (
        "Motion12MarkProbe requires a results list.")
    return Flatten(json_data[0], key_fn=_clean_up_path_segments).data


class MotionMark1Story(PressBenchmarkStory):
  URL_LOCAL: str = "http://localhost:8000/"
  ALL_STORIES = {
      "MotionMark": (
          "Multiply",
          "Canvas Arcs",
          "Leaves",
          "Paths",
          "Canvas Lines",
          "Images",
          "Design",
          "Suits",
      ),
      "HTML suite": (
          "CSS bouncing circles",
          "CSS bouncing clipped rects",
          "CSS bouncing gradient circles",
          "CSS bouncing blend circles",
          "CSS bouncing filter circles",
          # "CSS bouncing SVG images",
          "CSS bouncing tagged images",
          "Focus 2.0",
          "DOM particles, SVG masks",
          # "Composited Transforms",
      ),
      "Canvas suite": (
          "canvas bouncing clipped rects",
          "canvas bouncing gradient circles",
          # "canvas bouncing SVG images",
          # "canvas bouncing PNG images",
          "Stroke shapes",
          "Fill shapes",
          "Canvas put/get image data",
      ),
      "SVG suite": (
          "SVG bouncing circles",
          "SVG bouncing clipped rects",
          "SVG bouncing gradient circles",
          # "SVG bouncing SVG images",
          # "SVG bouncing PNG images",
      ),
      "Leaves suite": (
          "Translate-only Leaves",
          "Translate + Scale Leaves",
          "Translate + Opacity Leaves",
      ),
      "Multiply suite": (
          "Multiply: CSS opacity only",
          "Multiply: CSS display only",
          "Multiply: CSS visibility only",
      ),
      "Text suite": (
          "Design: Latin only (12 items)",
          "Design: CJK only (12 items)",
          "Design: RTL and complex scripts only (12 items)",
          "Design: Latin only (6 items)",
          "Design: CJK only (6 items)",
          "Design: RTL and complex scripts only (6 items)",
      ),
      "Suits suite": (
          "Suits: clip only",
          "Suits: shape only",
          "Suits: clip, shape, rotation",
          "Suits: clip, shape, gradient",
          "Suits: static",
      ),
      "3D Graphics": (
          "Triangles (WebGL)",
          # "Triangles (WebGPU)",
      ),
      "Basic canvas path suite": (
          "Canvas line segments, butt caps",
          "Canvas line segments, round caps",
          "Canvas line segments, square caps",
          "Canvas line path, bevel join",
          "Canvas line path, round join",
          "Canvas line path, miter join",
          "Canvas line path with dash pattern",
          "Canvas quadratic segments",
          "Canvas quadratic path",
          "Canvas bezier segments",
          "Canvas bezier path",
          "Canvas arcTo segments",
          "Canvas arc segments",
          "Canvas rects",
          "Canvas ellipses",
          "Canvas line path, fill",
          "Canvas quadratic path, fill",
          "Canvas bezier path, fill",
          "Canvas arcTo segments, fill",
          "Canvas arc segments, fill",
          "Canvas rects, fill",
          "Canvas ellipses, fill",
      )
  }
  SUBSTORIES = tuple(itertools.chain.from_iterable(ALL_STORIES.values()))
  READY_TIMEOUT: dt.timedelta = dt.timedelta(seconds=10)
  DEVELOPER_READY_JS: str = (
      "return document.querySelector('tree > li') !== undefined;")
  # The default page is ready immediately.
  READY_JS: str = "return true;"

  @classmethod
  @override
  def default_story_names(cls) -> tuple[str, ...]:
    return cls.ALL_STORIES["MotionMark"]

  @property
  @override
  def substory_duration(self) -> dt.timedelta:
    return dt.timedelta(seconds=35)

  @property
  def url_params(self) -> MutableMapping[str, str]:
    return {}

  @override
  def get_run_url(self, run: Run) -> str:
    url = super().get_run_url(run)
    if (url_params := self.url_params) or not self.has_default_substories:
      dev_url: str = f"{url}/developer.html"
      url = url_helper.update_url_query(dev_url, url_params)
    if url != self.url:
      logging.info("CUSTOM URL: %s", url)
    return url

  @override
  def setup(self, run: Run) -> None:
    test_url = self.get_run_url(run)
    use_developer_url = "/developer.html" in test_url
    with run.actions("Setup") as actions:
      actions.show_url(
          url=test_url,
          ready_state=ReadyState.COMPLETE,
          timeout=dt.timedelta(seconds=10))
      self._setup_wait_until_ready(actions, use_developer_url)
      if use_developer_url:
        self._setup_filter_stories(actions)

  def _setup_wait_until_ready(self, actions: Actions,
                              use_developer_url: bool) -> None:
    if use_developer_url:
      wait_js = self.DEVELOPER_READY_JS
    else:
      wait_js = self.READY_JS
    actions.wait_js_condition(wait_js, 0.2, self.READY_TIMEOUT)

  def _setup_filter_stories(self, actions: Actions) -> None:
    num_enabled = actions.js(
        """
      let benchmarks = arguments[0];
      const list = document.querySelectorAll(".tree li");
      let counter = 0;
      for (const row of list) {
        const name = row.querySelector("label.tree-label").textContent.trim();
        let checked = benchmarks.includes(name);
        const labels = row.querySelectorAll("input[type=checkbox]");
        for (const label of labels) {
          if (checked) {
            label.click()
            counter++;
          }
        }
      }
      return counter
      """,
        arguments=[self._substories])
    assert num_enabled > 0, "No tests were enabled"
    actions.wait(0.1)

  def run(self, run: Run) -> None:
    with run.actions("Running") as actions:
      actions.js("window.benchmarkController.startBenchmark()")
      actions.wait(self.fast_duration)
    with run.actions("Waiting for completion") as actions:
      actions.wait_js_condition(
          """
          return window.benchmarkRunnerClient.results._results != undefined
          """,
          0.5,
          timeout=self.slow_duration,
          delay=self.substory_duration / 4)


class MotionMark1Benchmark(MotionMarkBenchmark):
  pass
