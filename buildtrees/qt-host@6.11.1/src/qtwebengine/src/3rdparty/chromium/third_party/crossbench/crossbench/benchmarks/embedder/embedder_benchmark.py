# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import argparse
import datetime as dt
import logging
from typing import TYPE_CHECKING, Optional, Sequence, cast

from typing_extensions import override

from crossbench.benchmarks.base import SubStoryBenchmark
from crossbench.benchmarks.embedder.config.cujs import CUJsConfig
from crossbench.cli.ui import timer
from crossbench.parse import ObjectParser
from crossbench.stories.story import Story

if TYPE_CHECKING:
  from crossbench.action_runner.config import ActionRunnerConfig
  from crossbench.benchmarks.loading.config.blocks import ActionBlock
  from crossbench.browsers.webview.embedder import WebviewEmbedder
  from crossbench.cli.parser import CrossBenchArgumentParser
  from crossbench.runner.run import Run


class EmbedderStory(Story, metaclass=abc.ABCMeta):

  def __init__(self, name: str, actions: Sequence[ActionBlock]):
    duration = dt.timedelta(seconds=30)
    self._actions = actions
    super().__init__(name, duration)

  def setup(self, run: Run) -> None:
    # TODO(zbikowski): Add a way to ensure embedder is installed.
    # Launching the Google Quick Search app
    run_browser = cast("WebviewEmbedder", run.browser)
    run.browser_platform.sh("am", "start", "-S", "-W", "-n",
                            f"{run_browser.android_package}/.SearchActivity")
    logging.info("Starting Embedder Benchmark...")

  def run(self, run: Run) -> None:
    with timer():
      logging.info("-" * 80)
      action_runner = run.action_runner
      for block in self._actions:
        for action in block:
          action.run_with(run, action_runner)
    # Empty line to preserve timer output.
    print()
    logging.info("Stopping Embedder Benchmark...")

  def teardown(self, run: Run) -> None:
    # Return to home screen after the story has run
    run.browser_platform.sh("am", "start", "-a", "android.intent.action.MAIN",
                            "-c", "android.intent.category.HOME")

  @classmethod
  def all_story_names(cls) -> tuple[str, ...]:
    return ()


class EmbedderBenchmark(SubStoryBenchmark):
  """
  Benchmark runner for a WV embedder app mode.
  """
  NAME = "embedder"
  DEFAULT_STORY_CLS = EmbedderStory

  def __init__(
      self,
      stories: Sequence[Story],
      action_runner_config: Optional[ActionRunnerConfig] = None) -> None:
    for story in stories:
      assert isinstance(story, self.DEFAULT_STORY_CLS)
    super().__init__(stories, action_runner_config)

  @classmethod
  @override
  def cli_description(cls) -> str:
    assert cls.__doc__
    return cls.__doc__.strip()

  @classmethod
  @override
  def add_cli_parser(
      cls, subparsers: argparse.ArgumentParser) -> CrossBenchArgumentParser:
    parser = super().add_cli_parser(subparsers)
    parser.add_argument(
        "--cujs-config",
        "--cuj-config",
        type=CUJsConfig.parse,
        help="Stories we want to perform in the benchmark run following a"
        "specified scenario.")
    return parser

  @classmethod
  @override
  def stories_from_cli_args(cls, args: argparse.Namespace) -> Sequence[Story]:
    config = cls.get_cujs_config(args)
    cujs = tuple(
      EmbedderStory(
        name=cuj_config.label,
        actions=cuj_config.blocks,
      )
      for cuj_config in config.cujs
    )
    return cujs

  @classmethod
  def get_cujs_config(cls, args: argparse.Namespace) -> CUJsConfig:
    if global_config := args.config:
      # TODO: migrate --config to an already parsed hjson/json dict
      config_file = global_config
      config_data = ObjectParser.hjson_file(config_file)
      if cujs_config_dict := config_data.get("cujs"):
        if args.cujs_config:
          raise argparse.ArgumentTypeError(
              "Conflicting arguments: "
              "either specify a --config file without a 'cujs' property "
              "or remove the --cuj-config argument.")
        # TODO: CUJsConfig.parse_dict should be able to parse the inner dict.
        return CUJsConfig.parse_dict({"cujs": cujs_config_dict})
    return args.cujs_config
