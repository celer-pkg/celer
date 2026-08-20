# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import json
import logging
from typing import (TYPE_CHECKING, Any, Final, Mapping, MutableMapping,
                    Optional, Sequence, Type)

from immutabledict import immutabledict
from typing_extensions import override

from crossbench.benchmarks.base import (PressBenchmark,
                                        PressBenchmarkStoryFilter)
from crossbench.benchmarks.benchmark_probe import BenchmarkProbeMixin
from crossbench.helper import url_helper
from crossbench.parse import NumberParser, ObjectParser
from crossbench.probes.helper import Flatten
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.metric import Metric, MetricsMerger
from crossbench.stories.press_benchmark import PressBenchmarkStory

if TYPE_CHECKING:
  import argparse

  from crossbench.path import LocalPath
  from crossbench.probes.results import ProbeResult, ProbeResultDict
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run
  from crossbench.types import Json


def _probe_remove_tests_segments(path: tuple[str, ...]) -> str:
  return "/".join(segment for segment in path if segment != "tests")


class SpeedometerProbe(
    BenchmarkProbeMixin, JsonResultProbe, metaclass=abc.ABCMeta):
  """
  Speedometer-specific probe (compatible with v2.X and v3.X).
  Extracts all speedometer times and scores.
  """
  SORT_KEYS: bool = False
  SCORE_METRIC_KEY: Final[str] = "Score"

  @abc.abstractmethod
  @override
  def get_context_cls(self) -> Type[SpeedometerProbeContext]:
    pass

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        repetitions_group.results[self].json
        for repetitions_group in group.repetitions_groups)
    if self.SCORE_METRIC_KEY not in merged.data:
      merged.data[self.SCORE_METRIC_KEY] = self._compute_total_score(merged)
    return self.write_group_result(group, merged)

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

  def _is_valid_metric_key(self, metric_key: str) -> bool:
    parts = metric_key.split("/")
    if len(parts) == 2:
      return True
    if len(parts) == 1:
      return parts[0] in ("Geomean", "Score")
    return parts[-1] == "total"

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
    logging.critical("Speedometer results:")
    if not single_result:
      logging.critical("  %s", result_dict[self].csv)
      logging.critical("  %s", result_dict[self].get("xlsx"))
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
      if not self._is_valid_metric_key(metric_key):
        continue
      table[metric_key].append(
          Metric.format(metric["average"], metric["stddev"]))


class SpeedometerProbeContext(JsonResultProbeContext):
  JS = "return JSON.stringify(window.suiteValues);"

  @override
  def to_json(self, actions: Actions) -> Json:
    # Use serialized json as transport format to preserve object key order.
    json_payload = actions.js(self.JS)
    return json.loads(json_payload)

  @override
  def flatten_json_data(self, json_data: Any) -> Json:
    # json_data may contain multiple iterations, merge those first
    json_data = ObjectParser.non_empty_sequence(json_data,
                                                f"{self.probe.name} metrics")
    merged = MetricsMerger(
        json_data, key_fn=_probe_remove_tests_segments).to_json(
            value_fn=lambda values: values.geomean, sort=self.probe.SORT_KEYS)
    return Flatten(merged, sort=self.probe.SORT_KEYS).data


class SpeedometerStory(PressBenchmarkStory, metaclass=abc.ABCMeta):
  URL_LOCAL: str = "http://localhost:8000/"
  DEFAULT_ITERATIONS: int = 10

  def __init__(self,
               substories: Sequence[str] = (),
               iterations: Optional[int] = None,
               url_params: Optional[Mapping[str, str]] = None,
               url: Optional[str] = None) -> None:
    self._iterations: int = NumberParser.positive_int(
        iterations or self.DEFAULT_ITERATIONS,
        "iteration count",
        parse_str=False)
    self._url_params: Mapping[str, str] = immutabledict(url_params or {})
    super().__init__(substories=substories, url=url)

  @property
  def iterations(self) -> int:
    return self._iterations

  @property
  @override
  def substory_duration(self) -> dt.timedelta:
    return self.iterations * self.single_substory_duration

  @property
  def single_substory_duration(self) -> dt.timedelta:
    return dt.timedelta(seconds=0.4)

  @property
  @override
  def slow_duration(self) -> dt.timedelta:
    """Max duration that covers run-times on slow machines and/or
    debug-mode browsers.
    Making this number too large might cause needless wait times on broken
    browsers/benchmarks.
    """
    return dt.timedelta(seconds=60 * 20) + self.duration * 10

  @property
  def url_params(self) -> MutableMapping[str, str]:
    params: MutableMapping[str, str] = dict(self._url_params)
    if self.iterations != self.DEFAULT_ITERATIONS:
      params["iterationCount"] = str(self.iterations)
    return params


  @override
  def setup(self, run: Run) -> None:
    updated_url = self.get_run_url(run)
    with run.actions("Setup") as actions:
      actions.show_url(updated_url)
      actions.wait_js_condition(
          "return window.Suites !== undefined;", 0.5, timeout=10)
      self._setup_substories(actions)
      self._setup_benchmark_client(actions)
      actions.wait(0.5)

  @override
  def get_run_url(self, run: Run) -> str:
    url = super().get_run_url(run)
    url = url_helper.update_url_query(url, self.url_params)
    if url != self.url:
      logging.info("CUSTOM URL: %s", url)
    return url

  def _setup_substories(self, actions: Actions) -> None:
    if self._substories == self.SUBSTORIES:
      return
    actions.js(
        """
        let substories = arguments[0];
        Suites.forEach((suite) => {
          suite.disabled = substories.indexOf(suite.name) === -1;
        });""",
        arguments=[self._substories])

  def _setup_benchmark_client(self, actions: Actions) -> None:
    actions.js("""
      window.testDone = false;
      window.suiteValues = [];
      const client = window.benchmarkClient;
      const clientCopy = {
        didRunSuites: client.didRunSuites,
        didFinishLastIteration: client.didFinishLastIteration,
      };
      client.didRunSuites = function(measuredValues, ...arguments) {
          clientCopy.didRunSuites.call(this, measuredValues, ...arguments);
          window.suiteValues.push(measuredValues);
      };
      client.didFinishLastIteration = function(...arguments) {
          clientCopy.didFinishLastIteration.call(this, ...arguments);
          window.testDone = true;
      };""")

  def run(self, run: Run) -> None:
    with run.actions("Running") as actions:
      actions.js("""
          if (window.startTest) {
            window.startTest();
          } else {
            // Interactive Runner fallback / old 3.0 fallback.
            let startButton = document.getElementById("runSuites") ||
                document.querySelector("start-tests-button") ||
                document.querySelector(".buttons button");
            startButton.click();
          }
          """)
      actions.wait(self.fast_duration)
    with run.actions("Waiting for completion") as actions:
      actions.wait_js_condition(
          "return window.testDone",
          0.5,
          timeout=self.slow_duration,
          delay=self.substory_duration)


ProbeClsTupleT = tuple[Type[SpeedometerProbe], ...]


class SpeedometerBenchmarkStoryFilter(PressBenchmarkStoryFilter):
  __doc__ = PressBenchmarkStoryFilter.__doc__

  @classmethod
  @override
  def add_cli_arguments(
      cls, parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser = super().add_cli_arguments(parser)
    parser.add_argument(
        "--iterations",
        "--iteration-count",
        default=SpeedometerStory.DEFAULT_ITERATIONS,
        type=NumberParser.positive_int,
        help="Number of iterations each Speedometer subtest is run "
        "within the same session. \n"
        "Note: --repetitions restarts the whole benchmark, --iterations runs "
        "the same test tests n-times within the same session without the setup "
        "overhead of starting up a whole new browser.")
    return parser

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["iterations"] = args.iterations
    kwargs["url_params"] = cls.url_params_from_cli(args)
    return kwargs

  @classmethod
  def url_params_from_cli(cls,
                          args: argparse.Namespace) -> MutableMapping[str, str]:
    del args
    return {}

  def __init__(self,
               story_cls: Type[SpeedometerStory],
               patterns: Sequence[str],
               args: Optional[argparse.Namespace] = None,
               separate: bool = False,
               url: Optional[str] = None,
               iterations: Optional[int] = None,
               url_params: Optional[Mapping[str, str]] = None) -> None:
    self._iterations = iterations
    self._url_params = url_params
    assert issubclass(story_cls, SpeedometerStory)
    super().__init__(story_cls, patterns, args, separate, url)

  @override
  def create_stories_from_names(self, names: list[str],
                                separate: bool) -> Sequence[SpeedometerStory]:
    return self.story_cls.from_names(
        names,
        separate=separate,
        url=self.url,
        iterations=self._iterations,
        url_params=self._url_params)


class SpeedometerBenchmark(PressBenchmark, metaclass=abc.ABCMeta):

  DEFAULT_STORY_CLS = SpeedometerStory
  STORY_FILTER_CLS = SpeedometerBenchmarkStoryFilter

  @classmethod
  @override
  def short_base_name(cls) -> str:
    return "sp"

  @classmethod
  @override
  def base_name(cls) -> str:
    return "speedometer"
