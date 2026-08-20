# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import datetime as dt
from typing import TYPE_CHECKING, Any, Mapping, Optional, Sequence, Type

from typing_extensions import override

from crossbench.benchmarks.base import StoryFilter, SubStoryBenchmark
from crossbench.benchmarks.loading.config.pages import (
    DevToolsRecorderPagesConfig, ListPagesConfig, PageConfig, PagesConfig)
from crossbench.benchmarks.loading.page.base import DEFAULT_DURATION, Page
from crossbench.benchmarks.loading.page.combined import CombinedPage
from crossbench.benchmarks.loading.page.interactive import InteractivePage
from crossbench.benchmarks.loading.page.live import (PAGE_LIST,
                                                     PAGE_LIST_SMALL, PAGES,
                                                     LivePage)
from crossbench.benchmarks.loading.playback_controller import \
    PlaybackController
from crossbench.benchmarks.loading.tab_controller import TabController
from crossbench.parse import DurationParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.action_runner.config import ActionRunnerConfig
  from crossbench.cli.parser import CrossBenchArgumentParser
  from crossbench.stories.story import Story


class LoadingPageFilter(StoryFilter[Page]):
  """
  Filter / create loading stories

  Syntax:
    "name"            Include LivePage with the given name from predefined list.
    "name", 10        Include predefined page with given 10s timeout.
    "http://..."      Include custom page at the given URL with a default
                      timeout of 15 seconds.
    "http://...", 12  Include custom page at the given URL with a 12s timeout

  These patterns can be combined:
    ["http://foo.com", 5, "http://bar.co.jp", "amazon"]
  """
  stories: Sequence[Page]

  @classmethod
  @override
  def add_cli_arguments(
      cls, parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser = super().add_cli_arguments(parser)
    cls.add_page_config_parser(parser)
    tab_group = parser.add_mutually_exclusive_group()
    tab_group.add_argument(
        "--single-tab",
        dest="tabs",
        const=TabController.single(),
        default=TabController.default(),
        action="store_const",
        help="Open given urls in a single tab.")
    tab_group.add_argument(
        "--multiple-tab",
        dest="tabs",
        nargs="?",
        type=TabController.parse,
        const=TabController.multiple(),
        help="Open given urls in separate tabs "
        "(optional value for number of tabs for each url).")
    tab_group.add_argument(
        "--infinite-tab",
        dest="tabs",
        const=TabController.forever(),
        action="store_const",
        help="Open given urls in separate tabs infinitely.")

    playback_group = parser.add_mutually_exclusive_group()
    playback_group.add_argument(
        "--playback",
        "--cycle",
        type=PlaybackController.parse,
        default=PlaybackController.default(),
        help="Set limit on looping through/repeating the selected stories. "
        "Default is once."
        "Valid values are: 'once', 'forever', number, time. "
        "Cycle 10 times: '--playback=10x'. "
        "Repeat for 1.5 hours: '--playback=1.5h'.")
    playback_group.add_argument(
        "--forever",
        dest="playback",
        const=PlaybackController.forever(),
        action="store_const",
        help="Equivalent to --playback=infinity")

    parser.add_argument(
        "--about-blank-duration",
        "--about-blank",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta(),
        help="If non-zero, navigate to about:blank after every page.")

    block_modifier_group = parser.add_argument_group("Action Block Options")
    block_modifier_group.add_argument(
        "--skip-login",
        dest="run_login",
        default=True,
        action="store_const",
        const=False,
        help="Skip the login block, useful for replaying "
        "archive that filtered already all login requests "
        "to hide potential secrets. "
        "The login block is run by default.")
    block_modifier_group.add_argument(
        "--skip-setup",
        dest="run_setup",
        default=True,
        action="store_const",
        const=False,
        help="Skip the setup block, useful for replaying "
        "archive that filtered already all login requests "
        "to hide potential secrets. "
        "The setup block is run by default.")

    return parser

  @classmethod
  def add_page_config_arguments(cls, group: argparse._ArgumentGroup) -> None:
    group.add_argument(
        "--page-config",
        "--pages-config",
        dest="pages_config",
        type=PagesConfig.parse,
        help="Stories we want to perform in the benchmark run following a "
        "specified scenario. For a reference on how to build scenarios and "
        "possible actions check config/doc/pages.config.hjson")

  @classmethod
  def add_page_config_parser(cls, parser) -> None:
    page_config_group = parser.add_mutually_exclusive_group()
    # TODO: move --stories into mutually exclusive group as well
    page_config_group.add_argument(
        "--urls",
        "--url",
        dest="urls",
        help="List of urls and durations to load: url,seconds,...")
    cls.add_page_config_arguments(page_config_group)
    page_config_group.add_argument(
        "--url-file",
        "--urls-file",
        dest="pages_config",
        type=ListPagesConfig.parse,
        help=("List of urls and durations in a line-by-line file. "
              "Each line has the same format as --url for a single Page."))
    page_config_group.add_argument(
        "--devtools-recorder",
        dest="pages_config",
        type=DevToolsRecorderPagesConfig.parse,
        help="Run a single story from a serialized DevTools recorder session. "
        "See https://developer.chrome.com/docs/devtools/recorder/ "
        "for more details.")

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["separate"] = args.separate
    return kwargs

  @override
  def process_all(self, patterns: Sequence[str]) -> None:
    name_or_url_list = patterns
    if len(name_or_url_list) == 1:
      if name_or_url_list[0] == "all":
        self.stories = self.all_stories()
        return
      if name_or_url_list[0] == "default":
        self.stories = self.default_stories()
        return
    # Let the PageConfig handle the arg splitting again:
    config = PagesConfig.parse(",".join(patterns))
    self.stories = self.stories_from_config(self.args, config)

  @classmethod
  def all_stories(cls) -> tuple[Page, ...]:
    return tuple(PAGE_LIST)

  @classmethod
  def default_stories(cls) -> tuple[Page, ...]:
    return PAGE_LIST_SMALL

  @classmethod
  def stories_from_config(cls, args: argparse.Namespace,
                          config: PagesConfig) -> Sequence[Page]:
    labels = set(page_config.label for page_config in config.pages)
    use_labels = len(labels) == len(config.pages)

    stories: list[Page] = []
    for page_config in config.pages:
      stories.append(cls._story_from_config(args, page_config, use_labels))

    if not use_labels:
      # Double check that the urls are unique

      urls = set(page_config.first_url for page_config in config.pages)
      if len(urls) != len(config.pages):
        raise argparse.ArgumentTypeError(
            "Got non-unique story labels and urls.")
    return stories

  @classmethod
  def _story_from_config(cls, args: argparse.Namespace, config: PageConfig,
                         use_labels: bool) -> Page:
    playback: PlaybackController = args.playback
    tabs: TabController = args.tabs
    if config.playback:
      # TODO: support custom config playback
      playback = config.playback
    duration: dt.timedelta = config.duration
    if config.label in PAGES:
      page = PAGES[config.label]
      duration = duration or page.duration
      return LivePage(page.name, page.url, duration, playback, tabs,
                      args.about_blank_duration)

    label: str = config.any_label if use_labels else config.first_url
    duration = duration or DEFAULT_DURATION

    if not config.blocks:
      return LivePage(label, config.first_url, duration, playback, tabs,
                      args.about_blank_duration)
    return InteractivePage(
        name=label,
        blocks=config.blocks,
        login=config.login,
        setup=config.setup,
        teardown=config.teardown,
        secrets=config.secrets,
        playback=playback,
        tabs=tabs,
        about_blank_duration=args.about_blank_duration,
        run_login=args.run_login,
        run_setup=args.run_setup)

  @override
  def create_stories(self, separate: bool) -> Sequence[Page]:
    if not separate and len(self.stories) > 1:
      combined_name = "_".join(page.name for page in self.stories)
      args = self.args
      self.stories = (CombinedPage(self.stories, combined_name, args.playback,
                                   args.tabs),)
    return self.stories


class LoadingBenchmark(SubStoryBenchmark):
  """
  Benchmark runner for loading pages with complex interactions.

  Use --urls/--stories to either choose from an existing set of pages, or direct
  URLs. After each page you can also specify a custom wait/load duration in
  seconds. Multiple URLs/page names can be provided as a comma-separated list.

  Use --separate to load each page individually.

  Example:
    --urls=amazon
    --urls=http://cnn.com,10s
    --urls=http://twitter.com,5s,http://cnn.com,10s
  """
  NAME = "loading"
  DEFAULT_STORY_CLS = Page
  STORY_FILTER_CLS: Type[LoadingPageFilter] = LoadingPageFilter

  @classmethod
  @override
  def add_cli_parser(
      cls, subparsers: argparse.ArgumentParser) -> CrossBenchArgumentParser:
    parser = super().add_cli_parser(subparsers)
    cls.STORY_FILTER_CLS.add_cli_arguments(parser)
    return parser

  @classmethod
  @override
  def stories_from_cli_args(cls, args: argparse.Namespace) -> Sequence[Story]:
    has_default_stories: bool = (
        args.stories and args.stories == LoadingPageFilter.DEFAULT_STORY_NAME)
    if config := cls.get_pages_config(args):
      # TODO: make stories and page_config mutually exclusive.
      if not has_default_stories:
        raise argparse.ArgumentTypeError(
            f"Cannot specify --stories={repr(args.stories)} "
            "with any other page config option.")
      pages = cls.STORY_FILTER_CLS.stories_from_config(args, config)
      if args.separate:
        return pages
      if len(pages) == 1:
        return pages
      return (CombinedPage(pages, "Page Scenarios - Combined", args.playback,
                           args.tabs),)

    if args.urls:
      # TODO: make urls and stories mutually exclusive.
      if not has_default_stories:
        raise argparse.ArgumentTypeError(
            "Cannot specify --urls and --stories at the same time.")
      args.stories = args.urls

    # Fall back to story filter class.
    return super().stories_from_cli_args(args)

  @classmethod
  def get_pages_config(cls,
                       args: Optional[argparse.Namespace] = None
                      ) -> Optional[PagesConfig]:
    if not args:
      raise ValueError("Missing args")
    if global_config := args.config:
      # TODO: migrate --config to an already parsed hjson/json dict
      config_file = global_config
      config_data = ObjectParser.hjson_file(config_file)
      if pages_config_dict := config_data.get("pages"):
        if args.pages_config:
          raise argparse.ArgumentTypeError(
              "Conflicting arguments: "
              "either specify a --config file without a 'pages' property "
              "or remove the --page-config argument.")
        # TODO: PagesConfig.parse_dict should be able to parse the inner dict.
        return PagesConfig.parse_dict({"pages": pages_config_dict})
    return args.pages_config

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("load", "ld")

  @classmethod
  @override
  def describe_stories(cls) -> Mapping[str, str]:
    result: dict[str, str] = {}
    for story in cls.all_stories():
      story_help = story.help()
      if story_help == story.name:
        story_help = ""
      result[story.name] = story_help
    return result

  @classmethod
  @override
  def all_story_names(cls) -> Sequence[str]:
    # TODO: Use StoryFilter for listing stories everywhere.
    return sorted(story.name for story in cls.STORY_FILTER_CLS.all_stories())

  def __init__(
      self,
      stories: Sequence[Page],
      action_runner_config: Optional[ActionRunnerConfig] = None) -> None:
    for story in stories:
      assert isinstance(story, Page)
    super().__init__(stories, action_runner_config)
