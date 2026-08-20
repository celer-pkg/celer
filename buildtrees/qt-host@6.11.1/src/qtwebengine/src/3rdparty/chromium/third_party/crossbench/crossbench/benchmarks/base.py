# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import argparse
import logging
import re
from typing import (TYPE_CHECKING, Any, Generic, Mapping, Optional, Sequence,
                    Type, TypeAlias, TypeVar, cast)

from ordered_set import OrderedSet
from typing_extensions import override

from crossbench.action_runner.config import ActionRunnerConfig
from crossbench.cli.parser import CrossBenchArgumentParser
from crossbench.flags.base import Flags
from crossbench.helper import txt_helper
from crossbench.helper.collection_helper import close_matches_message
from crossbench.parse import ObjectParser
from crossbench.stories.press_benchmark import PressBenchmarkStory
from crossbench.stories.story import Story

if TYPE_CHECKING:
  from crossbench import path as pth
  from crossbench.action_runner.base import ActionRunner
  from crossbench.benchmarks.benchmark_probe import BenchmarkProbeMixin
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.plt.base import Platform
  from crossbench.runner.runner import Runner



class Benchmark(abc.ABC):
  NAME: str = ""
  DEFAULT_STORY_CLS: Type[Story] = Story  # type: ignore
  PROBES: tuple[Type[BenchmarkProbeMixin], ...] = ()
  DEFAULT_REPETITIONS: int = 1

  @classmethod
  def cli_help(cls) -> str:
    assert cls.__doc__, (f"Benchmark class {cls} must provide a doc string.")
    # Return the first non-empty line
    help_str: str = cls.__doc__.strip().splitlines()[0]
    if aliases := cls.aliases():
      help_str += f" [{', '.join(aliases)}]"
    return help_str

  @classmethod
  def cli_description(cls) -> str:
    assert cls.__doc__
    return cls.__doc__.strip()

  @classmethod
  def cli_epilog(cls) -> str:
    return ""

  @classmethod
  def aliases(cls) -> tuple[str, ...]:
    return tuple()

  @classmethod
  def add_cli_parser(cls, subparsers) -> CrossBenchArgumentParser:
    parser = subparsers.add_parser(
        cls.NAME,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        help=cls.cli_help(),
        description=cls.cli_description(),
        epilog=cls.cli_epilog(),
    )
    assert isinstance(parser, CrossBenchArgumentParser)
    parser.add_argument(
        "--action-runner-config",
        "--action-runner",
        type=ActionRunnerConfig.parse,
        help="Set the action runner for interactive pages.",
        required=False)
    return parser

  @classmethod
  def describe(cls) -> dict[str, Any]:
    return {
        "name":
            cls.NAME,
        "aliases":
            cls.aliases() or "None",
        "description":
            "\n".join(txt_helper.wrap_lines(cls.cli_description(), 70)),
        "stories": [],
        "probes-default": {
            probe_cls.NAME:
                "\n".join(
                    list(
                        txt_helper.wrap_lines((probe_cls.__doc__ or "").strip(),
                                              70))) for probe_cls in cls.PROBES
        }
    }

  @classmethod
  def default_probe_config_path(cls) -> Optional[pth.LocalPath]:
    return None

  @classmethod
  def default_network_config_path(cls) -> Optional[pth.LocalPath]:
    return None

  @classmethod
  def extra_flags(cls, browser_attributes: BrowserAttributes) -> Flags:
    del browser_attributes
    return Flags()

  @classmethod
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    return {"action_runner_config": args.action_runner_config}

  @classmethod
  def from_cli_args(cls, args: argparse.Namespace) -> Benchmark:
    kwargs = cls.kwargs_from_cli(args)
    return cls(**kwargs)

  def __init__(
      self,
      stories: Sequence[Story],
      action_runner_config: Optional[ActionRunnerConfig] = None) -> None:
    assert self.NAME is not None, f"{self} has no .NAME property"
    assert self.DEFAULT_STORY_CLS != Story, (
        f"{self} has no .DEFAULT_STORY_CLS property")
    self.stories: list[Story] = self._validate_stories(stories)
    self.log_stories(self.stories)
    self._action_runner_config = action_runner_config or ActionRunnerConfig()

  def _validate_stories(self, stories: Sequence[Story]) -> list[Story]:
    assert stories, "No stories provided"
    for story in stories:
      assert isinstance(story, self.DEFAULT_STORY_CLS), (
          f"story={story} should be a subclass/the same "
          f"class as {self.DEFAULT_STORY_CLS}")
    return list(stories)

  def new_action_runner(self, platform: Platform) -> ActionRunner:
    return self._action_runner_config.instantiate(platform)

  def setup(self, runner: Runner) -> None:
    del runner

  def log_stories(self, stories: Sequence[StoryT]) -> None:
    substory_names = [name for story in stories for name in story.substories]
    stories_str = ", ".join(substory_names)
    logging.info("📚 SELECTED %s STORIES AND %s SUBSTORIES: %s", len(stories),
                 len(substory_names), stories_str)


StoryT = TypeVar("StoryT", bound=Story)


class StoryFilter(Generic[StoryT], metaclass=abc.ABCMeta):
  DEFAULT_STORY_NAME: str = "default"

  @classmethod
  def add_cli_arguments(
      cls, parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser.add_argument(
        "--stories",
        "--story",
        dest="stories",
        default=cls.DEFAULT_STORY_NAME,
        help="Comma-separated list of story names. "
        "Use 'all' for selecting all available stories. "
        "Use 'default' for the standard selection of stories.")
    cls._add_story_grouping_arguments(parser)
    return parser

  @classmethod
  def _add_story_grouping_arguments(cls,
                                    parser: argparse.ArgumentParser) -> None:
    is_combined_group = parser.add_mutually_exclusive_group()
    is_combined_group.add_argument(
        "--combined",
        dest="separate",
        default=False,
        action="store_false",
        help="Run each story in the same session. (default)")
    is_combined_group.add_argument(
        "--separate",
        action="store_true",
        help="Run each story in a fresh browser.")

  @classmethod
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    return {"patterns": args.stories.split(","), "args": args}

  @classmethod
  def from_cli_args(cls, story_cls: Type[StoryT],
                    args: argparse.Namespace) -> StoryFilter[StoryT]:
    kwargs = cls.kwargs_from_cli(args)
    return cls(story_cls, **kwargs)

  def __init__(self,
               story_cls: Type[StoryT],
               patterns: Sequence[str],
               args: Optional[argparse.Namespace] = None,
               separate: bool = False) -> None:
    self._args = args
    self.story_cls: Type[StoryT] = story_cls
    assert issubclass(
        story_cls, Story), (f"Subclass of {Story} expected, found {story_cls}")
    # Using order-preserving dict instead of set
    self._known_names: dict[str,
                            None] = dict.fromkeys(story_cls.all_story_names())
    self.stories: Sequence[StoryT] = []
    # TODO: only use one method.
    self.process_all(patterns)
    self.stories = self.create_stories(separate)

  @property
  def args(self) -> argparse.Namespace:
    if args := self._args:
      return args
    raise RuntimeError("Missing args for additional filtering")

  @abc.abstractmethod
  def process_all(self, patterns: Sequence[str]) -> None:
    pass

  @abc.abstractmethod
  def create_stories(self, separate: bool) -> Sequence[StoryT]:
    pass


class SubStoryBenchmark(Benchmark, metaclass=abc.ABCMeta):
  STORY_FILTER_CLS: Type[StoryFilter] = StoryFilter  # type: ignore

  @classmethod
  @override
  def cli_description(cls) -> str:
    desc = super().cli_description()
    desc += "\n\n"
    desc += ("Stories (alternatively use the 'describe benchmark "
             f"{cls.NAME}' command):\n")
    desc += ", ".join(cls.all_story_names())
    desc += "\n\n"
    desc += "Filtering (for --stories): "
    assert cls.STORY_FILTER_CLS.__doc__, (
        f"{cls.STORY_FILTER_CLS} has no doc string.")
    desc += cls.STORY_FILTER_CLS.__doc__.strip()

    return desc

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["stories"] = cls.stories_from_cli_args(args)
    return kwargs

  @classmethod
  def stories_from_cli_args(cls, args: argparse.Namespace) -> Sequence[Story]:
    return cls.STORY_FILTER_CLS.from_cli_args(cls.DEFAULT_STORY_CLS,
                                              args).stories

  @classmethod
  @override
  def describe(cls) -> dict[str, Any]:
    data = super().describe()
    data["stories"] = cls.describe_stories()
    return data

  @classmethod
  def describe_stories(cls) -> Mapping[str, str]:
    # TODO: use story objects instead
    return {name: "" for name in cls.all_story_names()}

  @classmethod
  def all_stories(cls) -> Sequence[Story]:
    all_args = argparse.Namespace()
    return cls.STORY_FILTER_CLS(  # pylint: disable=abstract-class-instantiated
        cls.DEFAULT_STORY_CLS, ["all"],
        args=all_args,
        separate=True).stories

  @classmethod
  def all_story_names(cls) -> Sequence[str]:
    return sorted(cls.DEFAULT_STORY_CLS.all_story_names())


PressBenchmarkStoryT = TypeVar(
    "PressBenchmarkStoryT", bound=PressBenchmarkStory)


class RegexFilter():

  def __init__(self, all_names: Sequence[str], default_names: Sequence[str]):
    self._all_names: dict[str, None] = dict.fromkeys(all_names)
    self._default_names: dict[str, None] = dict.fromkeys(default_names)
    self._selected_names: OrderedSet[str] = OrderedSet()
    for name in self._all_names:
      assert name, "Invalid empty story name"
      assert not name.startswith("-"), (
          f"Known story names cannot start with '-', but got '{name}'.")
      assert not name == "all", "Known story name cannot match 'all'."

  def process_all(self, patterns: Sequence[str]) -> OrderedSet[str]:
    if not isinstance(patterns, (list, tuple)):
      raise ValueError("Expected Sequence of story name or patterns "
                       f"but got '{type(patterns)}'.")
    for pattern in patterns:
      self.process_pattern(pattern)
    return self._selected_names

  def process_pattern(self, pattern: str) -> None:
    if pattern.startswith("-"):
      self.remove(pattern[1:])
    else:
      self.add(pattern)

  def add(self, pattern: str) -> None:
    self._check_processed_pattern(pattern)
    regexp = self._pattern_to_regexp(pattern)
    self._add_matching(regexp, pattern)

  def remove(self, pattern: str) -> None:
    self._check_processed_pattern(pattern)
    regexp = self._pattern_to_regexp(pattern)
    self._remove_matching(regexp, pattern)

  def _pattern_to_regexp(self, pattern: str) -> re.Pattern:
    if pattern == "all":
      return re.compile(".*")
    if pattern == "default":
      if self._default_names == self._all_names:
        return re.compile(".*")
      joined_names = "|".join(re.escape(name) for name in self._default_names)
      return re.compile(f"^({joined_names})$")
    if pattern in self._all_names:
      return re.compile(re.escape(pattern))
    return re.compile(pattern)

  def _check_processed_pattern(self, pattern: str) -> None:
    if not pattern:
      raise ValueError("Empty pattern is not allowed")
    if pattern == "-":
      raise ValueError(f"Empty remove pattern not allowed: '{pattern}'")
    if pattern[0] == "-":
      raise ValueError(f"Unprocessed negative pattern not allowed: '{pattern}'")

  def _add_matching(self, regexp: re.Pattern, original_pattern: str) -> None:
    substories = self._regexp_match(regexp, original_pattern)
    self._selected_names.update(substories)

  def _remove_matching(self, regexp: re.Pattern, original_pattern: str) -> None:
    substories = self._regexp_match(regexp, original_pattern)
    for substory in substories:
      try:
        self._selected_names.remove(substory)
      except KeyError as e:
        raise ValueError(
            "Removing Story failed: "
            f"name='{substory}' extracted by pattern='{original_pattern}'"
            "is not in the filtered story list") from e

  def _regexp_match(self, regexp: re.Pattern,
                    original_pattern: str) -> list[str]:
    substories = [
        substory for substory in self._all_names if regexp.fullmatch(substory)
    ]
    if not substories:
      substories = self._regexp_match_ignorecase(regexp)
    if not substories:
      return self._handle_no_match(original_pattern)
    if len(substories) == len(self._all_names) and self._selected_names:
      raise ValueError(f"'{original_pattern}' matched all and overrode all"
                       "previously filtered story names.")
    return substories

  def _regexp_match_ignorecase(self, regexp: re.Pattern) -> list[str]:
    logging.warning(
        "No matching stories, using case-insensitive fallback regexp.")
    iregexp: re.Pattern = re.compile(regexp.pattern, flags=re.IGNORECASE)
    return [
        substory for substory in self._all_names if iregexp.fullmatch(substory)
    ]

  def _handle_no_match(self, original_pattern: str) -> list[str]:
    choices_ms, alternative = close_matches_message(original_pattern,
                                                    self._all_names)
    error_message: str = f"'{original_pattern}' didn't match any stories."
    error_message += choices_ms
    if alternative:
      logging.error(error_message)
      return [alternative]
    raise ValueError(error_message)


class PressBenchmarkStoryFilter(StoryFilter[PressBenchmarkStoryT],
                                Generic[PressBenchmarkStoryT]):
  """
  Filter stories by name or regexp.

  Syntax:
    "all"     Include all stories (defaults to story_names).
    "name"    Include story with the given name.
    "-name"   Exclude story with the given name'
    "foo.*"   Include stories whose name matches the regexp.
    "-foo.*"  Exclude stories whose name matches the regexp.

  These patterns can be combined:
    [".*", "-foo", "-bar"] Includes all except the "foo" and "bar" story
  """

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["separate"] = args.separate
    kwargs["url"] = args.custom_benchmark_url
    return kwargs

  def __init__(self,
               story_cls: Type[PressBenchmarkStoryT],
               patterns: Sequence[str],
               args: Optional[argparse.Namespace] = None,
               separate: bool = False,
               url: Optional[str] = None) -> None:
    self.url: str | None = url
    self._selected_names: OrderedSet[str] = OrderedSet()
    super().__init__(story_cls, patterns, args, separate)
    assert issubclass(self.story_cls, PressBenchmarkStory)

  @override
  def process_all(self, patterns: Sequence[str]) -> None:
    regex_filter = RegexFilter(
        all_names=self.story_cls.all_story_names(),
        default_names=self.story_cls.default_story_names())
    self._selected_names = regex_filter.process_all(patterns)

  @override
  def create_stories(self, separate: bool) -> Sequence[PressBenchmarkStoryT]:
    names = list(self._selected_names)
    stories = self.create_stories_from_names(names, separate)
    return stories

  def create_stories_from_names(
      self, names: list[str], separate: bool) -> Sequence[PressBenchmarkStoryT]:
    return self.story_cls.from_names(names, separate=separate, url=self.url)


VersionParts: TypeAlias = tuple[str] | tuple[int, ...]

class PressBenchmark(SubStoryBenchmark):
  STORY_FILTER_CLS = PressBenchmarkStoryFilter
  DEFAULT_STORY_CLS: Type[
      PressBenchmarkStory] = PressBenchmarkStory  # type: ignore

  @classmethod
  @abc.abstractmethod
  def short_base_name(cls) -> str:
    raise NotImplementedError()

  @classmethod
  @abc.abstractmethod
  def base_name(cls) -> str:
    raise NotImplementedError()

  @classmethod
  @abc.abstractmethod
  def version(cls) -> VersionParts:
    raise NotImplementedError()

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    raw_version: VersionParts = cls.version()
    is_branch_version = (
        len(raw_version) == 1 and isinstance(raw_version[0], str))
    if not is_branch_version:
      assert (all((isinstance(part, int)) for part in raw_version)), (
          "All version parts should be integers.")
    version = [str(v) for v in raw_version]
    assert version, "Expected non-empty version tuple."
    version_names = []
    dot_version = ".".join(version)
    for name in (cls.short_base_name(), cls.base_name()):
      assert name, "Expected non-empty base name."
      if not is_branch_version:
        version_names.append(f"{name}{dot_version}")
      version_name = f"{name}_{dot_version}"
      if version_name != cls.NAME:
        version_names.append(version_name)
    return tuple(version_names)

  @classmethod
  @override
  def add_cli_parser(cls, subparsers) -> CrossBenchArgumentParser:
    parser = super().add_cli_parser(subparsers)
    # TODO: Move story-related args to dedicated PressBenchmarkStoryFilter class
    cls._add_story_url_arguments(parser)
    cls.STORY_FILTER_CLS.add_cli_arguments(parser)
    return parser

  @classmethod
  def _add_story_url_arguments(cls, parser) -> None:
    benchmark_url_group = parser.add_argument_group(
        "Story URL Options").add_mutually_exclusive_group()
    live_url: str = cls.DEFAULT_STORY_CLS.URL
    local_url: str = cls.DEFAULT_STORY_CLS.URL_LOCAL
    official_url: str = cls.DEFAULT_STORY_CLS.URL_OFFICIAL
    benchmark_url_group.add_argument(
        "--live",
        "--live-url",
        "--browser-ben",
        "--browserben",
        dest="custom_benchmark_url",
        const=None,
        action="store_const",
        help=(f"Use chrome live benchmark url ({live_url}) "
              "on https://browserben.ch."))
    benchmark_url_group.add_argument(
        "--official",
        "--official-url",
        dest="custom_benchmark_url",
        const=official_url,
        action="store_const",
        help=(f"Use officially hosted live/online benchmark url "
              f"({official_url})."))
    benchmark_url_group.add_argument(
        "--local",
        "--local-url",
        "--url",
        "--custom-benchmark-url",
        type=ObjectParser.httpx_url_str,
        nargs="?",
        dest="custom_benchmark_url",
        const=local_url,
        help=(f"Use custom or locally (default={local_url}) "
              "hosted benchmark url."))

    if custom_fork_url := getattr(cls.DEFAULT_STORY_CLS, "URL_CHROME_FORK",
                                  None):
      benchmark_url_group.add_argument(
          "--custom",
          "--chrome-custom-fork",
          "--chrome-fork",
          action="store_const",
          dest="custom_benchmark_url",
          const=custom_fork_url,
          help=(f"Use custom chrome fork hosted on {custom_fork_url}. "
                "This include additional options and performance.mark calls "
                "for easier investigation."))

  @classmethod
  @override
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    kwargs = super().kwargs_from_cli(args)
    kwargs["custom_url"] = args.custom_benchmark_url
    return kwargs

  @classmethod
  @override
  def describe(cls) -> dict[str, Any]:
    data = super().describe()
    assert issubclass(cls.DEFAULT_STORY_CLS, PressBenchmarkStory)
    data["url"] = cls.DEFAULT_STORY_CLS.URL
    data["url-official"] = cls.DEFAULT_STORY_CLS.URL_OFFICIAL
    data["url-local"] = cls.DEFAULT_STORY_CLS.URL_LOCAL
    data["version"] = ".".join(map(str, cls.version()))
    return data

  def __init__(self,
               stories: Sequence[Story],
               action_runner_config: Optional[ActionRunnerConfig] = None,
               custom_url: Optional[str] = None) -> None:
    super().__init__(stories, action_runner_config)
    self.custom_url = custom_url
    if custom_url:
      for story in stories:
        press_story = cast(PressBenchmarkStory, story)
        assert press_story.url == custom_url

  @override
  def setup(self, runner: Runner) -> None:
    super().setup(runner)
    self.validate_url(runner)

  def validate_url(self, runner: Runner) -> None:
    if self.custom_url:
      if runner.has_any_live_network():
        self._validate_custom_url(runner, self.custom_url)
      return
    first_story = cast(PressBenchmarkStory, self.stories[0])
    url = first_story.url
    if not runner.has_all_live_network() and not url:
      # For non-live networks we create a matching URL
      return
    if not url:
      raise ValueError("Invalid empty url")
    if all(runner.env.validate_url(url, p) for p in runner.platforms):
      return
    msg = [
        f"Could not reach live benchmark URL: '{url}'."
        f"Please make sure you're connected to the internet."
    ]
    local_url = first_story.URL_LOCAL
    if local_url:
      msg.append(
          f"Alternatively use --local for the default local URL: {local_url}")
    raise ValueError("\n".join(msg))

  def _validate_custom_url(self, runner: Runner, url: str) -> None:
    if not all(runner.env.validate_url(url, p) for p in runner.platforms):
      raise ValueError(
          f"Could not reach custom benchmark URL: '{self.custom_url}'. "
          f"Please make sure your local web server is running.")
