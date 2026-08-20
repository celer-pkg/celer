# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import logging
from typing import TYPE_CHECKING, Hashable, Optional, Self, Set, Type, TypeVar

from typing_extensions import override

from crossbench import path as pth
from crossbench import plt
from crossbench.config import ConfigParser, UnusedPropertiesMode
# TODO: Keep commonly used classes here.
from crossbench.probes.probe_context import ProbeContext  # noqa: TC001
from crossbench.probes.probe_error import ProbeIncompatibleBrowser
from crossbench.probes.probe_result_key import ProbeResultKey
from crossbench.probes.result_location import ResultLocation
from crossbench.probes.results import (EmptyProbeResult, LocalProbeResult,
                                       ProbeResult)

if TYPE_CHECKING:
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.probes.probe_context import ProbeSessionContext
  from crossbench.runner.groups.base import RunGroup
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.cache_temperatures import \
      CacheTemperaturesRunGroup
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.groups.session import BrowserSessionRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run


ProbeT = TypeVar("ProbeT", bound="Probe")


class ProbeConfigParser(ConfigParser[ProbeT]):

  def __init__(self, probe_cls: Type[ProbeT]) -> None:
    probe_name: str = probe_cls.NAME
    if not probe_name:
      raise ValueError("Missing probe name.")
    super().__init__(
        probe_cls,
        key=probe_name,
        title=f"{probe_name} probe parser",
        unused_properties_mode=UnusedPropertiesMode.ERROR)
    self._probe_cls: Type[ProbeT] = probe_cls

  @property
  def probe_cls(self) -> Type[ProbeT]:
    return self._probe_cls



ProbeKeyT = tuple[tuple[str, Hashable], ...]


class Probe(ProbeResultKey, abc.ABC):
  """
  Abstract Probe class.

  Probes are responsible for extracting performance numbers from websites
  / stories.

  Probe interface:
  - scope(): Return a custom ProbeContext (see below)
  - validate_browser(): Customize to display warnings before using Probes with
    incompatible settings / browsers.
  The Probe object can the customize how to merge probe (performance) date at
  multiple levels:
  - multiple repetitions of the same story
  - merged repetitions from multiple stories (same browser)
  - Probe data from all Runs

  Probes use a ProbeContext that is active during a story-Run.
  The ProbeContext class defines a customizable interface
  - setup(): Used for high-overhead Probe initialization
  - start(): Low-overhead start-to-measure signal
  - stop():  Low-overhead stop-to-measure signal
  - teardown(): Used for high-overhead Probe cleanup

  """
  NAME: str = ""

  @classmethod
  def config_parser(cls) -> ProbeConfigParser[Self]:
    return ProbeConfigParser(cls)

  @classmethod
  def from_config(cls: Type[ProbeT], config_data: dict) -> ProbeT:
    return cls.config_parser().parse(config_data)

  @classmethod
  def help_text(cls) -> str:
    return cls.config_parser().help

  @classmethod
  def summary_text(cls) -> str:
    return cls.config_parser().summary

  # Set to False if the Probe cannot be used with arbitrary Stories or Pages
  IS_GENERAL_PURPOSE: bool = True
  PRODUCES_DATA: bool = True
  # Set the default probe result location, used to figure out whether result
  # files need to be transferred from a remote machine.
  RESULT_LOCATION = ResultLocation.LOCAL
  # Set to True if the probe only works on battery power with single runs
  BATTERY_ONLY: bool = False

  def __init__(self) -> None:
    assert self.name is not None, "A Probe must define a name"
    self._browsers: Set[Browser] = set()

  def __str__(self) -> str:
    return type(self).__name__

  def __eq__(self, other) -> bool:
    if self is other:
      return True
    if type(self) is not type(other):
      return False
    return self.key == other.key

  @property
  def is_internal(self) -> bool:
    """Returns True for subclasses of InternalProbe that are not
    directly user-accessible."""
    return False

  @property
  def key(self) -> ProbeKeyT:
    """Return a sort key."""
    return (("name", self.name),)

  def __hash__(self) -> int:
    return hash(self.key)

  @property
  def host_platform(self) -> plt.Platform:
    return plt.PLATFORM

  @property
  @override
  def name(self) -> str:
    return self.NAME

  @property
  def result_path_name(self) -> str:
    return self.name

  @property
  def is_attached(self) -> bool:
    return len(self._browsers) > 0

  def attach(self, browser: Browser) -> None:
    assert browser not in self._browsers, (
        f"Probe={self.name} is attached multiple times to the same browser")
    self._browsers.add(browser)

  def validate_env(self, env: RunnerEnv) -> None:
    """
    Part of the Checklist, make sure everything is set up correctly for a probe
    to run.
    Browser-only validation is handled in validate_browser(...).
    """
    # Ensure that the proper super methods for setting up a probe were
    # called.
    assert self.is_attached, (
        f"Probe {self.name} is not properly attached to a browser")
    for browser in self._browsers:
      self.validate_browser(env, browser)

  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    """
    Validate that browser is compatible with this Probe.
    - Raise ProbeValidationError for hard-errors,
    - Use env.handle_warning for soft errors where we expect recoverable errors
      or only partially broken results.
    """
    del env, browser

  def expect_browser(self,
                     browser: Browser,
                     attributes: BrowserAttributes,
                     message: Optional[str] = None) -> None:
    if attributes in browser.attributes():
      return
    if not message:
      message = f"Incompatible browser, expected {attributes}"
    raise ProbeIncompatibleBrowser(self, browser, message)

  def expect_macos(self, browser: Browser) -> None:
    if not browser.platform.is_macos:
      raise ProbeIncompatibleBrowser(self, browser, "Only supported on macOS")

  def expect_android(self, browser: Browser) -> None:
    if not browser.platform.is_android:
      raise ProbeIncompatibleBrowser(self, browser, "Only supported on Android")

  def setup(self, runner) -> None:
    """Called before any runs or browsers have been started."""
    pass

  def merge_cache_temperatures(self,
                               group: CacheTemperaturesRunGroup) -> ProbeResult:
    """
    For merging probe data from multiple browser cache temperatures with the
    same repetition, story and browser.
    """
    # Return the first result by default.
    return group.first_run.results[self]

  def symlinked_single_run_result(self, group: RunGroup) -> ProbeResult:
    runs = tuple(group.runs)
    if len(runs) != 1:
      return EmptyProbeResult()
    first_run = runs[0]
    if not first_run.create_symlinks:
      return EmptyProbeResult()

    first_run_results: ProbeResult = first_run.results[self]
    group_dir: pth.LocalPath = group.path
    symlinked_files: list[pth.LocalPath] = []
    for file in first_run_results.all_files():
      group_result_symlink = group_dir / file.name
      if group_result_symlink.exists():
        logging.debug("Skipping symlinking single run results: %s", file)
        continue
      group_result_symlink.symlink_to(file.relative_to(group_dir))
      symlinked_files.append(group_result_symlink)
    return LocalProbeResult(file=symlinked_files)

  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    """
    For merging probe data from multiple repetitions of the same story.
    """
    return self.symlinked_single_run_result(group)

  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    """
    For merging multiple stories for the same browser.
    """
    return self.symlinked_single_run_result(group)

  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    """
    For merging all probe data (from multiple stories and browsers.)
    """
    return self.symlinked_single_run_result(group)

  def get_context(self: Self, run: Run) -> Optional[ProbeContext[Self]]:
    probe_cls: Type[ProbeContext[Self]] = self.get_context_cls()
    return probe_cls(self, run)

  def get_context_cls(self: Self) -> Type[ProbeContext[Self]]:
    raise NotImplementedError(f"Missing default ProbeContext class for {self}")

  def get_session_context(  # pylint: disable=useless-return
      self: Self,
      session: BrowserSessionRunGroup) -> Optional[ProbeSessionContext[Self]]:
    del session
    return None

  def log_run_result(self, run: Run) -> None:
    """
    Override to print a short summary of the collected results after a run
    completes.
    """
    del run

  def log_browsers_result(self, group: BrowsersRunGroup) -> None:
    """
    Override to print a short summary of all the collected results.
    """
    del group
