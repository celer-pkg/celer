# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import json
import logging
import statistics
from collections import defaultdict
from typing import TYPE_CHECKING, Any, Final, Optional, Sequence, Type, cast

from typing_extensions import override

from crossbench.benchmarks.base import PressBenchmark
from crossbench.benchmarks.benchmark_probe import BenchmarkProbeMixin
from crossbench.parse import ObjectParser
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.metric import CSVFormatter, Metric, MetricsMerger
from crossbench.stories.press_benchmark import PressBenchmarkStory

if TYPE_CHECKING:
  import argparse

  from crossbench.action_runner.config import ActionRunnerConfig
  from crossbench.cli.parser import CrossBenchArgumentParser
  from crossbench.path import LocalPath
  from crossbench.probes.results import ProbeResult, ProbeResultDict
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run
  from crossbench.stories.story import Story
  from crossbench.types import Json


class JetStreamProbe(
    BenchmarkProbeMixin, JsonResultProbe, metaclass=abc.ABCMeta):
  """
  JetStream-specific Probe.
  Extracts all JetStream times and scores.
  """

  TOTAL_METRIC_KEY: Final[str] = "Total/score"
  SORT_KEYS: bool = False

  @property
  def jetstream(self) -> JetStreamBenchmark:
    return cast(JetStreamBenchmark, self.benchmark)

  @abc.abstractmethod
  @override
  def get_context_cls(self) -> Type[JetStreamProbeContext]:
    pass

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
    logging.critical("JetStream results:")
    if not single_result:
      logging.critical("  %s", result_dict[self].csv)
      logging.critical("  %s", result_dict[self].get("xlsx"))
    logging.info("- " * 40)

    with results_json.open(encoding="utf-8") as f:
      data = json.load(f)
      if single_result:
        logging.critical("Score %s", data[self.TOTAL_METRIC_KEY])
      else:
        self._log_result_metrics(data)

  @override
  def _extract_result_metrics_table(self, metrics: dict[str, Any],
                                    table: dict[str, list[str]]) -> None:
    for metric_key, metric_value in metrics.items():
      if not self._is_valid_metric_key(metric_key):
        continue
      table[metric_key].append(
          Metric.format(metric_value["average"], metric_value["stddev"]))
      # Separate runs don't produce a score
    if self.TOTAL_METRIC_KEY in metrics:
      metric_value = metrics[self.TOTAL_METRIC_KEY]
      table["Score"].append(
          Metric.format(metric_value["average"], metric_value["stddev"]))

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        story_group.results[self].json
        for story_group in group.repetitions_groups)
    # We discard the score when merging separate line item runs, recompute it!
    if self.TOTAL_METRIC_KEY not in merged.data:
      merged.data[self.TOTAL_METRIC_KEY] = self._compute_total_score(merged)
    return self.write_group_result(group, merged, JetStreamCSVFormatter)

  def _compute_total_score(self, merged: MetricsMerger) -> Metric:
    line_item_scores: list[list[float]] = []
    for key, metric in merged.data.items():
      if self._is_valid_metric_key(key):
        line_item_scores.append(metric.values)
    total_score = Metric()
    for iteration_line_items_score_values in zip(*line_item_scores):
      iteration_score = Metric(iteration_line_items_score_values).geomean
      total_score.append(iteration_score)
    return total_score

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self.merge_browsers_json_list(group).merge(
        self.merge_browsers_csv_list(group))

  def _is_valid_metric_key(self, metric_key: str) -> bool:
    parts = metric_key.split("/")
    if len(parts) != 2:
      return False
    if self.jetstream.detailed_metrics:
      return True
    return parts[0] != "Total" and parts[1] == "score"



class JetStreamProbeContext(JsonResultProbeContext):
  JS: str = """
  let results = Object.create(null);
  let benchmarks = []
  for (let benchmark of JetStream.benchmarks) {
    const data = { score: benchmark.score };
    if ("worst4" in benchmark) {
      data.firstIteration = benchmark.firstIteration;
      data.average = benchmark.average;
      data.worst4 = benchmark.worst4;
    } else if ("runTime" in benchmark) {
      data.runTime = benchmark.runTime;
      data.startupTime = benchmark.startupTime;
    } else if ("mainRun" in benchmark) {
      data.mainRun = benchmark.mainRun;
      data.stdlib = benchmark.stdlib;
    }
    results[benchmark.plan.name] = data;
    benchmarks.push(benchmark);
  };
  return JSON.stringify(results);
"""

  @override
  def to_json(self, actions: Actions) -> dict[str, float]:
    # Use serialized json as transport format to preserve object key order.
    json_payload = actions.js(self.JS)
    json_data = json.loads(json_payload)
    ObjectParser.non_empty_dict(json_data, f"{self.probe.name} metrics")
    return json_data

  @override
  def process_json_data(self, json_data: Json) -> Json:
    assert isinstance(json_data, dict)
    assert "Total" not in json_data, (
        "JSON result data already contains a ['Total'] entry.")
    json_data["Total"] = self._compute_total_metrics(json_data)
    return json_data

  def _compute_total_metrics(self, json_data: dict[str,
                                                   Any]) -> dict[str, float]:
    # Manually add all total scores
    accumulated_metrics = defaultdict(list)
    for _, metrics in json_data.items():
      for metric, value in metrics.items():
        accumulated_metrics[metric].append(value)
    total: dict[str, float] = {}
    for metric, values in accumulated_metrics.items():
      total[metric] = statistics.geometric_mean(values)
    return total


class JetStreamCSVFormatter(CSVFormatter):
  TOTAL_METRIC_KEY: Final[str] = JetStreamProbe.TOTAL_METRIC_KEY

  @override
  def format_items(self, data: dict[str, Json],
                   sort: bool) -> Sequence[tuple[str, Json]]:
    items = list(data.items())
    if sort:
      items.sort()
    # Copy all /score items to the top:
    score_items = []
    for key, value in items:
      if key != self.TOTAL_METRIC_KEY and key.endswith("/score"):
        score_items.append((key, value))
    total_item = [(self.TOTAL_METRIC_KEY, data[self.TOTAL_METRIC_KEY])]
    return total_item + score_items + items


class JetStreamStory(PressBenchmarkStory, metaclass=abc.ABCMeta):
  URL_LOCAL: str = "http://localhost:8000/"

  @property
  @override
  def substory_duration(self) -> dt.timedelta:
    return dt.timedelta(seconds=2)

  def run(self, run: Run) -> None:
    with run.actions("Running") as actions:
      # This might be run in d8, where JetStream.start() is blocking
      with actions.wait_until(self.fast_duration):
        actions.js("JetStream.start()", timeout=self.slow_duration)
    self.run_wait_until_done(run)

  def run_wait_until_done(self, run: Run) -> None:
    with run.actions("Waiting for completion") as actions:
      actions.wait_js_condition(
          """
        let summaryElement = document.getElementById("result-summary");
        return (summaryElement.classList.contains("done"));
        """,
          0.5,
          self.slow_duration,
          delay=self.substory_duration)


class JetStreamBenchmark(PressBenchmark, metaclass=abc.ABCMeta):

  @classmethod
  @override
  def short_base_name(cls) -> str:
    return "js"

  @classmethod
  @override
  def base_name(cls) -> str:
    return "jetstream"

  @classmethod
  @override
  def add_cli_parser(
      cls, subparsers: argparse.ArgumentParser) -> CrossBenchArgumentParser:
    parser = super().add_cli_parser(subparsers)
    parser.add_argument(
        "--detailed-metrics",
        "--details",
        default=False,
        action="store_true",
        help="Report more detailed internal metrics.")
    return parser

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["detailed_metrics"] = args.detailed_metrics
    return kwargs

  def __init__(self,
               stories: Sequence[Story],
               action_runner_config: Optional[ActionRunnerConfig] = None,
               custom_url: Optional[str] = None,
               detailed_metrics: bool = False) -> None:
    self._detailed_metrics = detailed_metrics
    super().__init__(stories, action_runner_config, custom_url)

  @property
  def detailed_metrics(self) -> bool:
    return self._detailed_metrics
