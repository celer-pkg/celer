# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
An adapter class for CBB to interact with the crossbench runner.
The goal is to abstract out the crossbench runner interface details and
provide an integration point for CBB.

Any breaking changes in the function definitions here need to be coordinated
with corresponding changes in CBB in google3
"""

from __future__ import annotations

from typing import TYPE_CHECKING, Optional, Type

from typing_extensions import override

import crossbench.benchmarks.all as benchmarks
import crossbench.browsers.browser
import crossbench.browsers.webdriver as cb_webdriver
import crossbench.runner.runner
from crossbench import path as pth
from crossbench.cli.config.env import ValidationMode
from crossbench.runner.run import Run

if TYPE_CHECKING:
  import datetime as dt

  from selenium import webdriver

  from crossbench.action_runner.base import ActionRunner
  from crossbench.benchmarks.base import PressBenchmark
  from crossbench.runner.groups.session import BrowserSessionRunGroup
  from crossbench.stories.press_benchmark import PressBenchmarkStory
  from crossbench.stories.story import Story

press_benchmarks: list[Type[PressBenchmark]] = [
    # Speedometer:
    benchmarks.Speedometer20Benchmark,
    benchmarks.Speedometer21Benchmark,
    benchmarks.Speedometer30Benchmark,
    benchmarks.Speedometer31Benchmark,
    benchmarks.SpeedometerMainBenchmark,
    # MotionMark:
    benchmarks.MotionMark12Benchmark,
    benchmarks.MotionMark13Benchmark,
    benchmarks.MotionMark131Benchmark,
    benchmarks.MotionMarkMainBenchmark,
    # JetStream:
    benchmarks.JetStream11Benchmark,
    benchmarks.JetStream20Benchmark,
    benchmarks.JetStream21Benchmark,
    benchmarks.JetStream22Benchmark,
    benchmarks.JetStreamMainBenchmark,
]

press_benchmarks_dict: dict[str, Type[PressBenchmark]] = {
    cls.NAME: cls for cls in press_benchmarks
}


def get_pressbenchmark_cls(
    benchmark_name: str) -> Optional[Type[PressBenchmark]]:
  """Returns the class of the specified pressbenchmark.

  Args:
    benchmark_name: Name of the benchmark.

  Returns:
    An instance of the benchmark implementation
  """
  return press_benchmarks_dict.get(benchmark_name)


def get_pressbenchmark_story_cls(
    benchmark_name: str) -> Optional[Type[PressBenchmarkStory]]:
  """Returns the class of the specified pressbenchmark story.

  Args:
    benchmark_name: Name of the benchmark.

  Returns:
    An instance of the benchmark default story
  """

  benchmark_cls = get_pressbenchmark_cls(benchmark_name)
  if benchmark_cls is not None:
    return benchmark_cls.DEFAULT_STORY_CLS

  return None


def create_remote_webdriver(driver: webdriver.Remote
                           ) -> cb_webdriver.RemoteWebDriver:
  """Creates a remote webdriver instance for the driver.

  Args:
    driver: Remote web driver instance.
  """

  browser = cb_webdriver.RemoteWebDriver("default", driver)
  return browser


def get_probe_result_file(benchmark_name: str,
                          browser: crossbench.browsers.browser.Browser,
                          output_dir: pth.LocalPathLike,
                          probe_name: Optional[str] = None) -> Optional[str]:
  """Returns the path to the probe result file.

  Args:
    benchmark_name: Name of the benchmark.
    browser: Browser instance.
    output_dir: Path to the directory where the output of the benchmark
                execution was written.
    probe_name: Optional name of the probe for the result file. If not
                specified, the first probe from the default benchmark story
                will be used."""
  output_dir_path = pth.LocalPath(output_dir)
  if probe_name is None:
    if benchmark_name not in press_benchmarks_dict:
      return None
    benchmark_cls = press_benchmarks_dict[benchmark_name]
    probe_cls = benchmark_cls.PROBES[0]
    probe_name = probe_cls.NAME

  result_file: pth.LocalPath = (
      output_dir_path / browser.unique_name / "stories" / f"{probe_name}.json")
  return str(result_file)


class CbbRunner(crossbench.runner.runner.Runner):

  @override
  def create_run(self, browser_session: BrowserSessionRunGroup, story: Story,
                 action_runner: ActionRunner, repetition: int, is_warmup: bool,
                 temperature: str, index: int, name: str, timeout: dt.timedelta,
                 throw: bool, env_validation_mode: ValidationMode) -> Run:
    return CbbRun(self, browser_session, story, action_runner, repetition,
                  is_warmup, temperature, index, name, timeout, throw,
                  env_validation_mode)


class CbbRun(Run):

  @override
  def _setup_session_dir(self) -> None:
    # Don't create symlink loops and skip this step
    pass


def run_benchmark(output_folder: pth.LocalPathLike,
                  browser_list: list[crossbench.browsers.browser.Browser],
                  benchmark: PressBenchmark) -> None:
  """Runs the benchmark using crossbench runner.

  Args:
    output_folder: Path to the directory where the output of the benchmark
                  execution will be written.
    browser_list: List of browsers to run the benchmark on.
    benchmark: The Benchmark instance to run.
  """
  runner = CbbRunner(
      out_dir=pth.LocalPath(output_folder),
      browsers=browser_list,
      benchmark=benchmark,
      env_validation_mode=ValidationMode.SKIP)

  runner.run()
