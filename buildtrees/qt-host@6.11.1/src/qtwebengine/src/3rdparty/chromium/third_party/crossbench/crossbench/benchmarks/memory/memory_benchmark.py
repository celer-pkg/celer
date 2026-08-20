# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import json
import logging
from typing import TYPE_CHECKING, Any, MutableMapping, Optional, Sequence, Type

import selenium.common.exceptions
import urllib3.exceptions
from typing_extensions import override

from crossbench.action_runner.action_runner_listener import \
    ActionRunnerListener
from crossbench.benchmarks.base import StoryFilter, SubStoryBenchmark
from crossbench.benchmarks.benchmark_probe import BenchmarkProbeMixin
from crossbench.benchmarks.loading.page.base import Page
from crossbench.benchmarks.loading.page.live import LivePage
from crossbench.benchmarks.loading.tab_controller import TabController
from crossbench.helper import url_helper
from crossbench.parse import NumberParser
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.metric import MetricsMerger
from crossbench.runner.exception import StopStoryException

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
  from crossbench.types import JsonDict


class MemoryProbe(BenchmarkProbeMixin, JsonResultProbe):
  """
  Memory-specific Probe.
  Extracts the number of alive tabs.
  """
  NAME: str = "memory_probe"

  @override
  def get_context_cls(self) -> Type[MemoryProbeContext]:
    return MemoryProbeContext

  def to_json(self, actions: Actions) -> JsonDict:
    raise NotImplementedError(
        "should not be called, data comes from memory probe context")

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
    logging.critical("Memory results (num of alive tabs):")
    if not single_result:
      logging.critical("  %s", result_dict[self].csv)
    logging.info("- " * 40)

    with results_json.open(encoding="utf-8") as f:
      data = json.load(f)
      if single_result:
        logging.critical("Score %s", data["alive_tab_count"])
      else:
        self._log_result_metrics(data)

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        repetitions_group.results[self].json
        for repetitions_group in group.repetitions_groups)
    return self.write_group_result(group, merged)

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self.merge_browsers_json_list(group).merge(
        self.merge_browsers_csv_list(group))


class MemoryProbeContext(ActionRunnerListener,
                         JsonResultProbeContext[MemoryProbe]):

  def __init__(self, probe: MemoryProbe, run: Run) -> None:
    super().__init__(probe, run)
    cur_benchmark = probe.benchmark
    if not isinstance(cur_benchmark, MemoryBenchmark):
      raise TypeError("The probe only works for MemoryBenchmark")
    run.action_runner.set_listener(self)
    self._skippable_tab_count = cur_benchmark._skippable_tab_count
    self._target_tab_count = cur_benchmark.get_target_tab_count()
    self._intensive_tab_switch_count = \
      cur_benchmark.get_intensive_tab_switch_count()
    # Records the navigation_start_time time for each window handle.
    self._navigation_time_ms: dict[str, float] = {}
    self._tab_count: int = 1

  def start(self) -> None:
    pass

  @override
  def to_json(self, actions: Actions) -> JsonDict:
    return {"alive_tab_count": self._tab_count - 1}

  def _increment_tab_count(self) -> None:
    self._tab_count += 1

  def _record_navigation_time(self, run: Run) -> None:
    """
    Record NavigationStart time for each handle.
    """
    with run.actions("_record_navigation_time", measure=False) as action:
      cur_handle: str = action.current_window_id()
      navigation_start_time = action.js(
          "return window.performance.timing.navigationStart")
      logging.debug("Browser: %s. Navigation starttime for handle %s is %s.",
                    run.browser.unique_name, cur_handle, navigation_start_time)
      self._navigation_time_ms[cur_handle] = navigation_start_time

  def _check_liveness(self, run: Run) -> None:
    """
    Navigate each opened tab, and check if the navigation start time
    has changed. If so, then it means that page has been discarded
    and reloaded.
    """
    with run.actions("_check_liveness", measure=False) as action:
      for handle, handle_navigation_time_ms in self._navigation_time_ms.items():
        logging.debug("Browser: %s. Liveness checking for handle: %s",
                      run.browser, handle)
        action.switch_window(handle)
        action.wait(1)
        navigation_start_time = action.js(
            "return window.performance.timing.navigationStart")
        if navigation_start_time != handle_navigation_time_ms:
          logging.info(
              "Browser: %s. The max num of tabs we can keep alive concurrently "
              "is: %s ", run.browser, self._tab_count - 1)
          raise StopStoryException("Found a page that has been reloaded.")

  def _check_error_msg(self, e: Exception) -> bool:
    if isinstance(e, selenium.common.exceptions.WebDriverException
                 ) and "page crash" in str(e):
      return True
    if isinstance(e, selenium.common.exceptions.TimeoutException):
      return True
    if isinstance(e, urllib3.exceptions.ReadTimeoutError):
      return True
    # Error msg from `Could not execute JS` due to page crash.
    if isinstance(e, ValueError) and "page crash" in str(e):
      return True
    return False

  @override
  def handle_error(self, run: Run, e: Exception) -> None:
    """
    If there is a page crash error or a http request time out
    for the stress liveness test, directly exit the benchmark
    and report the max alive tab count.
    """
    if self._check_error_msg(e):
      logging.info(
          "Browser: %s. The max num of tabs we can keep alive concurrently "
          "is: %s ", run.browser, self._tab_count - 1)
      raise StopStoryException(f"Found a Tab Crash/Timeout: {e}")

  @override
  def handle_page_run(self, run: Run) -> None:
    self._record_navigation_time(run)
    if self._tab_count > self._skippable_tab_count:
      self._check_liveness(run)
    # Conduct intensive tab switch between the target num of tabs.
    if self._intensive_tab_switch_count > 0 \
      and self._tab_count == self._target_tab_count:
      self._intensive_tab_switch(run)
      self._collect_tab_switch_metric(run)

  @override
  def handle_new_tab(self, run: Run) -> None:
    self._increment_tab_count()

  def _intensive_tab_switch(self, run: Run) -> None:
    cur_tab_switch_count = 0
    with run.actions("Intensive Tab Switching", measure=False) as action:
      while cur_tab_switch_count < self._intensive_tab_switch_count:
        for handle, _ in self._navigation_time_ms.items():
          cur_tab_switch_count += 1
          logging.debug(
              "Browser: %s. Switching to handle: %s. "
              "Current tab switch count: %s", run.browser, handle,
              cur_tab_switch_count)
          action.switch_window(handle)
          action.wait(2)

  def _collect_tab_switch_metric(self, run: Run) -> None:
    with run.actions("Collect Tab Switch Metric", measure=False) as action:
      browser = run.browser
      browser.switch_to_new_tab()
      switch_duration_histogram = \
        "chrome://histograms/#Browser.Tabs.TotalSwitchDuration3"
      browser.show_url(switch_duration_histogram)
      content = action.js(
          "let content = document.documentElement.innerText; return content;")
      logging.info("TabSwitchDuration Metrics: %s", content)


class MemoryBenchmarkStoryFilter(StoryFilter[Page]):
  """
  Create memory story
  Specify alloc-count, block-size, compressiblity,
  prefill-constant, random style to decide the
  memory workload.
  """
  stories: Sequence[Page]
  URL = "https://chromium-workloads.web.app/web-tests/main/synthetic/memory"

  @classmethod
  @override
  def add_cli_arguments(
      cls, parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser = super().add_cli_arguments(parser)
    parser.add_argument(
        "--alloc-count",
        type=NumberParser.positive_int,
        default=1,
        help="The number of block to allocate.")
    parser.add_argument(
        "--block-size",
        type=NumberParser.positive_int,
        default=128,
        help="The size of each block (MB).")
    parser.add_argument(
        "--compressibility",
        type=NumberParser.positive_zero_int,
        default=0,
        help="The compressibility (0-100)")
    parser.add_argument(
        "--prefill-constant",
        type=NumberParser.any_int,
        default=1,
        help="Prefill memory buffer with given constant (-1-127)."
        "Default is 1."
        "-1 represents no prefilling.")
    parser.add_argument(
        "--random-per-buffer",
        dest="random_per_page",
        action="store_false",
        help="With the flag, it will generate the memory workload "
        "with random per buffer level. Without the flag,"
        "it will generate the memory workload with random"
        "per page level.")

    tab_group = parser.add_mutually_exclusive_group()
    tab_group.add_argument(
        "--tabs",
        type=TabController.parse,
        default=TabController.default(),
        help="Open memory workload in single/multiple/infinity tabs. "
        "Default is single."
        "Valid values are: 'single', 'inf', 'infinity', number")
    tab_group.add_argument(
        "--single-tab",
        dest="tabs",
        const=TabController.single(),
        default=TabController.default(),
        action="store_const",
        help="Open memory workload in a single tab."
        "Equivalent to --tabs=single")
    tab_group.add_argument(
        "--infinite-tab",
        dest="tabs",
        const=TabController.forever(),
        action="store_const",
        help="Open memory workload in separate tabs infinitely."
        "Equivalent to --tabs=infinity")
    return parser

  @override
  def process_all(self, patterns: Sequence[str]) -> None:
    self.stories = self.stories_from_cli_args(self.args)

  @classmethod
  def stories_from_cli_args(cls, args: argparse.Namespace) -> Sequence[Page]:
    url_params: MutableMapping[str, str] = {
        "alloc": str(args.alloc_count),
        "blocksize": str(args.block_size),
        "compress": str(args.compressibility),
        "prefill": str(args.prefill_constant),
    }
    if not args.random_per_page:
      url_params["randomperpage"] = "false"
    url = url_helper.update_url_query(cls.URL, url_params)
    stories: Sequence[Page] = []
    page = LivePage("memory", url, dt.timedelta(seconds=2), tabs=args.tabs)
    stories = [page]
    return stories

  @override
  def create_stories(self, separate: bool) -> Sequence[Page]:
    return self.stories


class MemoryBenchmark(SubStoryBenchmark):
  """
  Benchmark runner for memory stress test.
  """

  NAME = "memory"
  DEFAULT_STORY_CLS = Page
  STORY_FILTER_CLS = MemoryBenchmarkStoryFilter
  PROBES: tuple[Type[MemoryProbe], ...] = (MemoryProbe,)

  @classmethod
  @override
  def add_cli_parser(
      cls, subparsers: argparse.ArgumentParser) -> CrossBenchArgumentParser:
    parser = super().add_cli_parser(subparsers)
    cls.STORY_FILTER_CLS.add_cli_arguments(parser)
    parser.add_argument(
        "--skippable-tab-count",
        type=NumberParser.positive_int,
        default=0,
        help="The number of tabs that can be skipped for liveness checking.")
    parser.add_argument(
        "--intensive-tab-switch-count",
        type=NumberParser.positive_int,
        default=0,
        help="The num of tab switch for stress testing.")
    return parser

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["skippable_tab_count"] = args.skippable_tab_count
    kwargs["target_tab_count"] = args.tabs.count
    kwargs["intensive_tab_switch_count"] = args.intensive_tab_switch_count
    return kwargs

  @classmethod
  @override
  def stories_from_cli_args(cls, args: argparse.Namespace) -> Sequence[Page]:
    super().stories_from_cli_args(args)
    stories = MemoryBenchmarkStoryFilter.stories_from_cli_args(args)
    return stories

  @classmethod
  @override
  def all_story_names(cls) -> tuple[str, ...]:
    return ()

  def __init__(self,
               stories: Sequence[Page],
               action_runner_config: Optional[ActionRunnerConfig] = None,
               skippable_tab_count: int = 0,
               target_tab_count: int = 0,
               intensive_tab_switch_count: int = 0) -> None:
    for story in stories:
      assert isinstance(story, Page)
    super().__init__(stories, action_runner_config)
    self._skippable_tab_count = skippable_tab_count
    self._target_tab_count = target_tab_count
    self._intensive_tab_switch_count = intensive_tab_switch_count

  def get_target_tab_count(self) -> int:
    return self._target_tab_count

  def get_intensive_tab_switch_count(self) -> int:
    return self._intensive_tab_switch_count

  @classmethod
  @override
  def describe(cls) -> dict[str, Any]:
    data = super().describe()
    data["url"] = cls.STORY_FILTER_CLS.URL
    return data
