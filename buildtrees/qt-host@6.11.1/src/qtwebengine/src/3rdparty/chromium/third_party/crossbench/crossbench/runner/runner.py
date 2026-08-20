# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import enum
import logging
from typing import TYPE_CHECKING, Any, Iterable, Optional, Sequence, Set, Type

from crossbench import exception
from crossbench import path as pth
from crossbench import plt
from crossbench.benchmarks import benchmark_validator
from crossbench.benchmarks.benchmark_probe import BenchmarkProbeMixin
from crossbench.env.runner_env import EnvConfig, RunnerEnv, ValidationMode
from crossbench.helper import collection_helper
from crossbench.helper.state import BaseState, StateMachine
from crossbench.helper.wait import WaitRange
from crossbench.helper.wake_lock import WakeLock
from crossbench.parse import NumberParser, ObjectParser
from crossbench.probes import all as all_probes
from crossbench.probes.internal.summary import ResultsSummaryProbe
from crossbench.probes.perfetto.trace_processor.trace_processor import \
    TraceProcessorProbe
from crossbench.probes.probe import Probe, ProbeIncompatibleBrowser
from crossbench.results_db.db import ResultsDB
from crossbench.runner.groups.browsers import BrowsersRunGroup
from crossbench.runner.groups.cache_temperatures import \
    CacheTemperaturesRunGroup
from crossbench.runner.groups.repetitions import RepetitionsRunGroup
from crossbench.runner.groups.session import BrowserSessionRunGroup
from crossbench.runner.groups.stories import StoriesRunGroup
from crossbench.runner.groups.thread import RunMainGroup, RunThreadGroup
from crossbench.runner.run import Run
from crossbench.runner.timing import Timing
from crossbench.str_enum_with_help import StrEnumWithHelp

if TYPE_CHECKING:
  import argparse

  from crossbench.action_runner.base import ActionRunner
  from crossbench.benchmarks.base import Benchmark
  from crossbench.browsers.browser import Browser
  from crossbench.probes.thermal_monitor import ThermalStatus
  from crossbench.runner.groups.base import RunGroup
  from crossbench.runner.timing import AnyTimeUnit
  from crossbench.stories.story import Story



class RunnerException(exception.MultiException):
  pass


@enum.unique
class ThreadMode(StrEnumWithHelp):
  NONE = ("none", (
      "Execute all browser-sessions sequentially, default. "
      "Low interference risk, use for worry-free time-critical measurements."))
  PLATFORM = ("platform", (
      "Execute browser-sessions from each platform in parallel threads. "
      "Might cause some interference with probes that do heavy "
      "post-processing."))
  BROWSER = ("browser", (
      "Execute browser-sessions from each browser in parallel thread. "
      "High interference risk, don't use for time-critical measurements."))
  SESSION = ("session", (
      "Execute run from each browser-session in a parallel thread. "
      "High interference risk, don't use for time-critical measurements."))

  def group(self, runs: list[Run]) -> list[RunThreadGroup]:
    if self == ThreadMode.NONE:
      return [RunMainGroup(runs)]
    groups: dict[Any, list[Run]] = {}
    if self == ThreadMode.SESSION:
      groups = collection_helper.group_by(
          runs, lambda run: run.browser_session, sort_key=None)
    elif self == ThreadMode.PLATFORM:
      groups = collection_helper.group_by(
          runs, lambda run: run.browser_platform, sort_key=None)
    elif self == ThreadMode.BROWSER:
      groups = collection_helper.group_by(
          runs, lambda run: run.browser, sort_key=None)
    else:
      raise ValueError(f"Unexpected thread mode: {self}")
    return [
        RunThreadGroup(runs, index=index)
        for index, runs in enumerate(groups.values())
    ]


@enum.unique
class RunnerState(BaseState):
  INITIAL = enum.auto()
  SETUP = enum.auto()
  RUNNING = enum.auto()
  TEARDOWN = enum.auto()

class Runner:

  @classmethod
  def get_out_dir(cls,
                  cwd: pth.LocalPath,
                  suffix: str = "",
                  test: bool = False) -> pth.LocalPath:
    if test:
      return cwd / "results" / "test"
    if suffix:
      suffix = "_" + suffix
    return (cwd / "results" /
            f"{dt.datetime.now().strftime('%Y-%m-%d_%H%M%S')}{suffix}")

  @classmethod
  def add_cli_parser(
      cls, benchmark_cls: Type[Benchmark],
      parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    cls._add_run_arguments(benchmark_cls, parser)
    cls._add_output_arguments(benchmark_cls, parser)
    return parser

  @classmethod
  def _add_run_arguments(cls, benchmark_cls: Type[Benchmark],
                         parser: argparse.ArgumentParser) -> None:
    run_group = parser.add_argument_group("Run & Repetition Options")
    run_group.add_argument(
        "--repetitions",
        "--repeat",
        "--invocations",
        "-r",
        default=benchmark_cls.DEFAULT_REPETITIONS,
        type=NumberParser.positive_int,
        help=("Number of times each benchmark story is repeated. "
              f"Defaults to {benchmark_cls.DEFAULT_REPETITIONS}. "
              "Metrics are aggregated over multiple repetitions"))
    run_group.add_argument(
        "--warmup-repetitions",
        "--warmups",
        default=0,
        type=NumberParser.positive_zero_int,
        help=("Number of times each benchmark story is repeated for warmup. "
              "Defaults to 0. "
              "Metrics for warmup-repetitions are discarded."))
    run_group.add_argument(
        "--cache-temperatures",
        default=["default"],
        const=["cold", "warm", "hot"],
        action="store_const",
        help=("Repeat each run with different cache temperatures without "
              "closing the browser in between."))
    run_group.add_argument(
        "--thread-mode",
        "--parallel",
        default=ThreadMode.NONE,
        type=ThreadMode,  # type: ignore
        help=("Change how Runs are executed.\n" +
              ThreadMode.help_text(indent=2)))
    run_group.add_argument(
        "--step-by-step-mode",
        action="store_true",
        help="Wait for user input before executing each action.")

  @classmethod
  def _add_output_arguments(cls, benchmark_cls: Type[Benchmark],
                            parser: argparse.ArgumentParser) -> None:
    out_dir_group = parser.add_argument_group("Output Directory Options")
    symlink_group = out_dir_group.add_mutually_exclusive_group()
    symlink_group.add_argument(
        "--no-symlinks",
        "--nosymlinks",
        dest="create_symlinks",
        action="store_false",
        default=not plt.PLATFORM.is_win,
        help=("Do not create symlinks in the output directory. "
              "Disabled by defauly on windows."))
    symlink_group.add_argument(
        "--symlinks",
        dest="create_symlinks",
        action="store_true",
        help="Allow create symlinks in the output directory.")

    out_dir_xor_group = out_dir_group.add_mutually_exclusive_group()
    out_dir_xor_group.add_argument(
        "--out-dir",
        "--output-directory",
        "-o",
        type=pth.LocalPath,
        help=("Results will be stored in this directory. "
              "Defaults to results/${DATE}_${LABEL}"))
    out_dir_xor_group.add_argument(
        "--label",
        "--name",
        type=ObjectParser.non_empty_str,
        default=benchmark_cls.NAME,
        help=("Add a name to the default output directory. "
              "Defaults to the benchmark name"))
    out_dir_group.add_argument(
        "--cache-dir",
        type=pth.LocalPath,
        default=None,
        help=("Used for caching browser binaries and archives. "
              "Defaults to binary_cache"))

  @classmethod
  def kwargs_from_cli(cls, args: argparse.Namespace) -> dict[str, Any]:
    if args.out_dir:
      out_dir = args.out_dir
    else:
      label = args.label
      assert label
      root_dir = pth.LocalPath(__file__).parents[2]
      out_dir = cls.get_out_dir(root_dir, label)
    return {
        "out_dir": out_dir,
        "browsers": args.browser,
        "repetitions": args.repetitions,
        "warmup_repetitions": args.warmup_repetitions,
        "cache_temperatures": args.cache_temperatures,
        "thread_mode": args.thread_mode,
        "throw": args.throw,
        "create_symlinks": args.create_symlinks,
        "cool_down_threshold": args.cool_down_threshold,
        "step_by_step_mode": args.step_by_step_mode,
    }

  def __init__(self,
               out_dir: pth.LocalPath,
               browsers: Sequence[Browser],
               benchmark: Benchmark,
               additional_probes: Iterable[Probe] = (),
               platform: Optional[plt.Platform] = None,
               env_config: Optional[EnvConfig] = None,
               env_validation_mode: ValidationMode = ValidationMode.THROW,
               repetitions: int = 1,
               warmup_repetitions: int = 0,
               cache_temperatures: Iterable[str] = ("default",),
               timing: Timing = Timing(),
               cool_down_threshold: Optional[ThermalStatus] = None,
               thread_mode: ThreadMode = ThreadMode.NONE,
               throw: bool = False,
               create_symlinks: bool = True,
               in_memory_result_db: bool = False,
               step_by_step_mode: bool = False) -> None:
    self._state = StateMachine(RunnerState.INITIAL)
    self.out_dir = out_dir.absolute()
    assert not self.out_dir.exists(), f"out_dir={self.out_dir} exists already"
    self.out_dir.mkdir(parents=True)
    self._timing = timing
    self._cool_down_threshold: ThermalStatus | None = cool_down_threshold
    self._browsers: tuple[Browser, ...] = tuple(browsers)
    self._validate_browser_labels()
    self._benchmark = benchmark
    self._stories = tuple(benchmark.stories)
    self._repetitions = NumberParser.positive_int(repetitions, "repetitions")
    self._warmup_repetitions = NumberParser.positive_zero_int(
        warmup_repetitions, "warmup repetitions")
    self._cache_temperatures: tuple[str, ...] = tuple(cache_temperatures)
    self._probes: list[Probe] = []
    self._default_probes: list[Probe] = []
    # Contains both measure and warmup runs:
    self._all_runs: list[Run] = []
    self._measured_runs: list[Run] = []
    self._thread_mode = thread_mode
    self._exceptions = exception.Annotator(throw)
    self._platform = platform or plt.PLATFORM
    self._env = RunnerEnv(self.platform, self.out_dir, self.browsers,
                          self.probes, self.repetitions, env_config,
                          env_validation_mode)
    self._attach_default_probes(additional_probes)
    self._prepare_benchmark()
    if in_memory_result_db:
      self._results_db = ResultsDB()
    else:
      self._results_db = ResultsDB(self.out_dir / "results.db")
    self._cache_temperatures_groups: tuple[CacheTemperaturesRunGroup, ...] = ()
    self._repetitions_groups: tuple[RepetitionsRunGroup, ...] = ()
    self._story_groups: tuple[StoriesRunGroup, ...] = ()
    self._browser_group: BrowsersRunGroup | None = None
    self._create_symlinks: bool = create_symlinks
    self._step_by_step_mode: bool = step_by_step_mode

  def _prepare_benchmark(self) -> None:
    benchmark_validator.validate_cls(type(self._benchmark))
    for benchmark_probe_cls in self._benchmark.PROBES:
      probe = benchmark_probe_cls(benchmark=self._benchmark)
      assert (isinstance(probe, Probe) and
              isinstance(probe, BenchmarkProbeMixin)), (
                  f"Expected BenchmarkProbe, got {probe}")
      self.attach_probe(probe)

  def _validate_browser_labels(self) -> None:
    assert self.browsers, "No browsers provided"
    browser_unique_names = [browser.unique_name for browser in self.browsers]
    ObjectParser.unique_sequence(browser_unique_names, "browser names")

  def _attach_default_probes(self, probe_list: Iterable[Probe]) -> None:
    assert len(self._probes) == 0
    assert len(self._default_probes) == 0
    self._attach_internal_probes()

    for index, probe in enumerate(probe_list):
      assert (not isinstance(probe, TraceProcessorProbe) or index == 0), (
          f"TraceProcessorProbe must be first in the list to be able "
          f"to process other probes data. Found it at index: {index}")
      self.attach_probe(probe)
    # Results probe must be first in the list, and thus last to be processed
    # so all other probes have data by the time we write the results summary.
    assert isinstance(self._probes[0], ResultsSummaryProbe)

  def _attach_internal_probes(self) -> None:
    for probe_cls in all_probes.NON_CONFIGURABLE_INTERNAL_PROBES:
      default_probe: Probe = probe_cls()  # pytype: disable=not-instantiable
      self._attach_default_probe(default_probe)

    thermal_monitor_probe = all_probes.ThermalMonitorProbe(
        cool_down_time=self._timing.cool_down_time,
        threshold=self._cool_down_threshold)
    self._attach_default_probe(thermal_monitor_probe)

    # TODO: pass in the flag to the runner for a cleaner setup.
    if any(browser.driver_logging for browser in self.browsers):
      if not all(browser.driver_logging for browser in self.browsers):
        raise RuntimeError("Driver logging must be enabled on all browsers")
      driver_logging_probe = all_probes.BrowserDriverLogProbe()
      self._attach_default_probe(driver_logging_probe)

  def _attach_default_probe(self, probe: Probe) -> None:
    self.attach_probe(probe)
    self._default_probes.append(probe)

  def attach_probe(self,
                   probe: Probe,
                   matching_browser_only: bool = False) -> Probe:
    if probe in self._probes:
      raise ValueError(f"Cannot add the same probe twice: {probe.NAME}")
    probe_was_used = False
    for browser in self.browsers:
      try:
        probe.validate_browser(self.env, browser)
        browser.attach_probe(probe)
        probe_was_used = True
      except ProbeIncompatibleBrowser as e:
        if matching_browser_only:
          logging.error("Skipping incompatible probe=%s for browser=%s:",
                        probe.name, browser.unique_name)
          logging.error("    %s", e)
          continue
        raise
    if probe_was_used:
      self._probes.append(probe)
    return probe

  @property
  def timing(self) -> Timing:
    return self._timing

  @property
  def cache_temperatures(self) -> tuple[str, ...]:
    return self._cache_temperatures

  @property
  def browsers(self) -> tuple[Browser, ...]:
    return self._browsers

  @property
  def stories(self) -> tuple[Story, ...]:
    return self._stories

  @property
  def probes(self) -> Iterable[Probe]:
    return iter(self._probes)

  @property
  def default_probes(self) -> Iterable[Probe]:
    return iter(self._default_probes)

  @property
  def benchmark(self) -> Benchmark:
    return self._benchmark

  @property
  def repetitions(self) -> int:
    return self._repetitions

  @property
  def warmup_repetitions(self) -> int:
    return self._warmup_repetitions

  @property
  def create_symlinks(self) -> bool:
    return self._create_symlinks

  @property
  def exceptions(self) -> exception.Annotator:
    return self._exceptions

  @property
  def is_success(self) -> bool:
    return len(self._measured_runs) > 0 and self._exceptions.is_success

  @property
  def platform(self) -> plt.Platform:
    return self._platform

  @property
  def env(self) -> RunnerEnv:
    return self._env

  @property
  def platforms(self) -> Set[plt.Platform]:
    return set(browser.platform for browser in self.browsers)

  @property
  def results_db(self) -> ResultsDB:
    return self._results_db

  @property
  def all_runs(self) -> tuple[Run, ...]:
    return tuple(self._all_runs)

  @property
  def runs(self) -> tuple[Run, ...]:
    return tuple(self._measured_runs)

  @property
  def cache_temperatures_groups(self) -> tuple[CacheTemperaturesRunGroup, ...]:
    assert self._cache_temperatures_groups, (
        f"No CacheTemperatureRunGroup in {self}")
    return self._cache_temperatures_groups

  @property
  def repetitions_groups(self) -> tuple[RepetitionsRunGroup, ...]:
    assert self._repetitions_groups, f"No RepetitionsRunGroup in {self}"
    return self._repetitions_groups

  @property
  def story_groups(self) -> tuple[StoriesRunGroup, ...]:
    assert self._story_groups, f"No StoriesRunGroup in {self}"
    return self._story_groups

  @property
  def browser_group(self) -> BrowsersRunGroup:
    assert self._browser_group, f"No BrowsersRunGroup in {self}"
    return self._browser_group

  @property
  def has_browser_group(self) -> bool:
    return self._browser_group is not None

  def wait_range(self,
                 min_interval: AnyTimeUnit,
                 timeout: AnyTimeUnit,
                 delay: AnyTimeUnit = 0) -> WaitRange:
    timing = self.timing
    return WaitRange(
        min=timing.timedelta(min_interval),
        timeout=timing.timeout_timedelta(timeout),
        delay=timing.timedelta(delay))

  def wait(self, time: AnyTimeUnit, absolute_time: bool = False) -> None:
    if not time:
      return
    delta = self.timing.timedelta(time, absolute_time)
    self._platform.sleep(delta)

  def run(self, is_dry_run: bool = False) -> None:
    self._state.expect(RunnerState.INITIAL)
    with WakeLock(self._platform):
      with self._exceptions.annotate("Preparing"):
        self._setup()
      with self._exceptions.capture("Running"):
        self._run(is_dry_run)

    if self._exceptions.throw:
      # Ensure that we bail out on the first exception.
      self.assert_successful_sessions_and_runs()
    if not is_dry_run:
      self._teardown()
    self.assert_successful_sessions_and_runs()

  def _setup(self) -> None:
    """ Mostly perform validations.
    Unlike later phases, any exception in here will cause the runner to stop.
    """
    self._state.transition(RunnerState.INITIAL, to=RunnerState.SETUP)
    logging.info("-" * 80)
    logging.info("SETUP")
    logging.info("-" * 80)
    assert self.repetitions > 0, (
        f"Invalid repetitions count: {self.repetitions}")
    assert self.browsers, "No browsers provided: self.browsers is empty"
    assert self.stories, "No stories provided: self.stories is empty"
    self._setup_validate_browsers()
    with self._exceptions.annotate("Preparing Probes"):
      self._setup_probes()
    with self._exceptions.annotate("Preparing Runs"):
      self._all_runs = list(self.get_runs())
      assert self._all_runs, f"{type(self)}.get_runs() produced no runs"
      logging.info("🏃 SETUP %d RUN(S)", len(self._all_runs))
      self._measured_runs = [run for run in self._all_runs if not run.is_warmup]
    with self._exceptions.annotate("Preparing Environment"):
      self._env.setup()
    with self._exceptions.annotate(
        f"Preparing Benchmark: {self._benchmark.NAME}"):
      self._benchmark.setup(self)
    self._results_db.setup_runs(self._all_runs)

  def _setup_validate_browsers(self) -> None:
    logging.info("🌐 SETUP %d BROWSER(S)", len(self.browsers))
    with self._exceptions.annotate("Validating all browsers"):
      for browser in self.browsers:
        with self._exceptions.capture(
            f"Preparing browser type={browser.type_name()} "
            f"unique_name={browser.unique_name}"):
          self._setup_validate_browser(browser)

  def _setup_validate_browser(self, browser: Browser) -> None:
    browser.validate()
    for probe in browser.probes:
      assert probe in self._probes, (
          f"Browser {browser} probe {probe} not in Runner.probes. "
          "Use Runner.attach_probe()")

  def _setup_probes(self) -> None:
    for probe in self.probes:
      with self._exceptions.annotate(f"Preparing Probe: {probe.name}"):
        probe.setup(self)

  def has_any_live_network(self) -> bool:
    return any(browser.network.is_live for browser in self.browsers)

  def has_all_live_network(self) -> bool:
    return all(browser.network.is_live for browser in self.browsers)

  def get_runs(self) -> Iterable[Run]:
    index = 0
    session_index = 0
    throw = self._exceptions.throw
    total_repetitions = self.repetitions + self.warmup_repetitions
    for repetition in range(total_repetitions):
      is_warmup: bool = repetition < self.warmup_repetitions
      for story in self.stories:
        for browser in self.browsers:
          # TODO: implement browser-session start/stop
          extra_benchmark_flags = self.benchmark.extra_flags(
              browser.attributes())
          browser_session = BrowserSessionRunGroup(self.env, self.probes,
                                                   browser,
                                                   extra_benchmark_flags,
                                                   session_index, self.out_dir,
                                                   self.create_symlinks, throw)
          session_index += 1
          for t_index, temperature in enumerate(self.cache_temperatures):
            name_parts = [f"story={story.name}"]
            if total_repetitions > 1:
              name_parts.append(f"repetition={repetition}")
            if len(self.cache_temperatures) > 1:
              name_parts.append(f"temperature={temperature_icon(temperature)}")
            name_parts.append(f"index={index}")
            action_runner = self.benchmark.new_action_runner(browser.platform)
            action_runner.set_step_by_step_mode(self._step_by_step_mode)
            yield self.create_run(
                browser_session,
                story,
                action_runner,
                repetition,
                is_warmup,
                f"{t_index}_{temperature}",
                index,
                name=", ".join(name_parts),
                timeout=self.timing.run_timeout,
                throw=throw,
                env_validation_mode=self.env.validation_mode)
            index += 1
          browser_session.set_ready()

  def create_run(self, browser_session: BrowserSessionRunGroup, story: Story,
                 action_runner: ActionRunner, repetition: int, is_warmup: bool,
                 temperature: str, index: int, name: str, timeout: dt.timedelta,
                 throw: bool, env_validation_mode: ValidationMode) -> Run:
    return Run(self, browser_session, story, action_runner, repetition,
               is_warmup, temperature, index, name, timeout, throw,
               env_validation_mode)

  def assert_successful_sessions_and_runs(self) -> None:
    if self._exceptions.is_success:
      return
    failed_runs: int = len(list(run for run in self.runs if not run.is_success))
    all_runs: int = len(tuple(self.runs))
    num_exceptions = len(self._exceptions)
    message: str = (
        f"{failed_runs}/{all_runs} Runs had {num_exceptions} error(s).")
    if not failed_runs:
      # No need to log the error here, since merging probe data is the last
      # step and errors have already been printed right before calling this
      # helper.
      message = f"Merged probe data with {num_exceptions} error(s)"
    else:
      # Print run failures, since they potentially have been printed the last
      # time very high up.
      logging.error("=" * 80)
      logging.error("❗ %s", message.upper())
      logging.error("=" * 80)
    # Raise a RunnerException to be handled in the CLI.
    self._exceptions.assert_success(message, RunnerException)

  def _get_thread_groups(self) -> list[RunThreadGroup]:
    # Also include warmup runs here.
    return self._thread_mode.group(self._all_runs)

  def _run(self, is_dry_run: bool = False) -> None:
    self._state.transition(RunnerState.SETUP, to=RunnerState.RUNNING)
    thread_groups: list[RunThreadGroup] = []
    with self._exceptions.info("Creating thread groups for all Runs"):
      thread_groups = self._get_thread_groups()
      for thread_group in thread_groups:
        thread_group.is_dry_run = is_dry_run

    group_count = len(thread_groups)
    if group_count == 1:
      self._run_single_threaded(thread_groups[0])
      return

    with self._exceptions.annotate(f"Starting {group_count} thread groups."):
      for thread_group in thread_groups:
        thread_group.start()
    with self._exceptions.annotate(
        "Waiting for all thread groups to complete."):
      for thread_group in thread_groups:
        thread_group.join()

  def _run_single_threaded(self, thread_group: RunThreadGroup) -> None:
    # Special case single thread groups
    with self._exceptions.annotate("Running single thread group"):
      thread_group.run()

  def _teardown(self) -> None:
    self._state.transition(RunnerState.RUNNING, to=RunnerState.TEARDOWN)
    logging.info("=" * 80)
    if self.is_success:
      logging.info("✅ %s RUNS COMPLETED SUCCESSFULLY", len(self.runs))
    else:
      logging.warning("❗ %s RUNS COMPLETED WITH ERRORS", len(self.runs))
    logging.info("=" * 80)
    logging.info("MERGING PROBE DATA")
    self._teardown_merge_probe_data()

  def _teardown_merge_probe_data(self) -> None:
    throw = self._exceptions.throw
    self._cache_temperatures_groups = CacheTemperaturesRunGroup.groups(
        self._measured_runs, throw)
    self._teardown_merge_run_group("cache temperatures",
                                   self._cache_temperatures_groups)

    self._repetitions_groups = RepetitionsRunGroup.groups(
        self._cache_temperatures_groups, throw)
    self._teardown_merge_run_group("repetitions", self._repetitions_groups)

    self._story_groups = StoriesRunGroup.groups(self._repetitions_groups, throw)
    self._teardown_merge_run_group("stories", self._story_groups)

    self._browser_group = BrowsersRunGroup(self._story_groups, throw)
    self._teardown_merge_run_group("browsers", [self._browser_group])

  def _teardown_merge_run_group(self, group_name: str,
                                groups: Iterable[RunGroup]) -> None:
    logging.debug("MERGING PROBE DATA: %s", group_name)
    group_exceptions = exception.ExceptionAnnotator(self._exceptions.throw)
    try:
      for group in groups:
        group.merge(self.probes)
        group_exceptions.extend(group.exceptions, is_nested=True)
    finally:
      self._exceptions.extend(group_exceptions)
      # Don't clutter the output if we have global failures.
      any_successful_group = any(group.is_success for group in groups)
      if any_successful_group and not group_exceptions.is_success:
        group_exceptions.log(
            f"❗ MERGED {group_name.upper()} PROBE DATA WITH ERRORS",
            separator="-")

  def update_symlinks(self) -> None:
    if not self.create_symlinks:
      logging.debug("Symlink disabled by command line option")
      return
    if self.out_dir.exists():
      self._create_runs_results_symlinks()

  def _create_runs_results_symlinks(self) -> None:
    assert self.create_symlinks
    results_root = self.out_dir.parent
    runs: tuple[Run, ...] = self.all_runs
    if not runs:
      logging.debug("Skip creating result symlinks in '%s': no runs produced.",
                    results_root)
      return
    self._create_first_last_run_symlinks(runs)
    self._create_runs_symlinks(runs)
    self._create_sessions_symlinks(runs)

  def _create_first_last_run_symlinks(self, runs: tuple[Run, ...]) -> None:
    out_dir = self.out_dir
    first_run_dir = out_dir / "first_run"
    if first_run_dir.exists():
      logging.error("Cannot create first_run symlink: %s", first_run_dir)
    else:
      first_run_dir.symlink_to(runs[0].out_dir.relative_to(out_dir))
    last_run_dir = out_dir / "last_run"
    if last_run_dir.exists():
      logging.error("Cannot create last_run symlink: %s", last_run_dir)
    else:
      last_run_dir.symlink_to(runs[-1].out_dir.relative_to(out_dir))

  def _create_runs_symlinks(self, runs: tuple[Run, ...]) -> None:
    out_dir = self.out_dir
    runs_dir = out_dir / "runs"
    runs_dir.mkdir()
    for run in runs:
      if not run.out_dir.exists():
        continue
      relative = pth.LocalPath("..") / run.out_dir.relative_to(out_dir)
      (runs_dir / str(run.index)).symlink_to(relative)

  def _create_sessions_symlinks(self, runs: tuple[Run, ...]) -> None:
    out_dir = self.out_dir
    sessions_dir = out_dir / "sessions"
    sessions_dir.mkdir()
    for session in set(run.browser_session for run in runs):
      relative = pth.LocalPath("..") / session.path.relative_to(out_dir)
      (sessions_dir / str(session.index)).symlink_to(relative)


TEMPERATURE_ICONS = {
    "cold": "🥶",
    "warm": "⛅",
    "hot": "🔥",
}


def temperature_icon(temperature: str) -> str:
  if icon := TEMPERATURE_ICONS.get(temperature):
    return icon
  return temperature
