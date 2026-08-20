# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import logging
from typing import TYPE_CHECKING, Any, Optional

from typing_extensions import override

from crossbench.benchmarks.base import Benchmark
from crossbench.cli.ui import timer
from crossbench.helper import input_helper
from crossbench.parse import DurationParser, ObjectParser
from crossbench.stories.story import Story

if TYPE_CHECKING:
  import argparse

  from crossbench.action_runner.config import ActionRunnerConfig
  from crossbench.cli.parser import CrossBenchArgumentParser
  from crossbench.runner.run import Run


class ManualStory(Story, metaclass=abc.ABCMeta):

  STORY_NAME = "manual"

  def __init__(self,
               start_after: Optional[dt.timedelta] = dt.timedelta(),
               run_for: Optional[dt.timedelta] = dt.timedelta(),
               url: Optional[str] = None) -> None:
    self._start_after = start_after
    self._run_for = run_for
    self._url = url
    duration = ((start_after or dt.timedelta()) +
                (run_for or dt.timedelta(seconds=30)))
    super().__init__(self.STORY_NAME, duration)

  @override
  def setup(self, run: Run) -> None:
    if self._start_after is None:
      logging.info("-" * 80)
      logging.critical("Press enter to start:")
      input()
    elif self._start_after.total_seconds():
      logging.critical("-" * 80)
      logging.critical(
          "The browser has launched. Measurement will start in %s" +
          " (or press enter to start immediately)", self._start_after)
      input_helper.input_with_timeout(timeout=self._start_after)
    logging.info("Starting Manual Benchmark...")

  def run(self, run: Run) -> None:
    with timer():
      logging.info("-" * 80)
      if url := self._url:
        with run.actions("Show URL") as actions:
          actions.show_url(url)
      self._wait_for_input()
    # Empty line to preserve timer output.
    print()
    logging.info("Stopping Manual Benchmark...")

  def _wait_for_input(self) -> None:
    if self._run_for is None:
      logging.critical("Press enter to stop:")
      try:
        input()
      except KeyboardInterrupt:
        pass
    else:
      logging.critical(
          "Measurement has started. The browser will close in %s" +
          " (or press enter to close immediately)", self._run_for)
      input_helper.input_with_timeout(timeout=self._run_for)

  @classmethod
  @override
  def all_story_names(cls) -> tuple[str, ...]:
    return (ManualStory.STORY_NAME,)


class ManualBenchmark(Benchmark, metaclass=abc.ABCMeta):
  """
  Full manual benchmark.

  Just launches the browser and lets the user perform the desired interactions.
  Optionally waits for |start_after| seconds, then runs measurements for
  |run_for| seconds, then closes the browser.
  """
  NAME = "manual"
  DEFAULT_STORY_CLS = ManualStory

  def __init__(self,
               action_runner_config: Optional[ActionRunnerConfig] = None,
               start_after: Optional[dt.timedelta] = None,
               run_for: Optional[dt.timedelta] = None,
               url: Optional[str] = None) -> None:
    manual_story = ManualStory(
        start_after=start_after, run_for=run_for, url=url)
    super().__init__([manual_story], action_runner_config)

  @classmethod
  @override
  def add_cli_parser(
      cls, subparsers: argparse.ArgumentParser) -> CrossBenchArgumentParser:
    parser = super().add_cli_parser(subparsers)
    parser.add_argument(
        "--start-after",
        type=DurationParser.positive_or_zero_duration,
        help="How long to wait until measurement starts")
    parser.add_argument(
        "--run-for",
        "--stop-after",
        "--duration",
        type=DurationParser.positive_duration,
        help="How long to run measurement for")
    parser.add_argument(
        "--url",
        "--test-url",
        type=ObjectParser.url_str,
        help="Navigate to URL right after the start-after timeout.")
    return parser

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["start_after"] = args.start_after
    kwargs["run_for"] = args.run_for
    kwargs["url"] = args.url
    return kwargs
