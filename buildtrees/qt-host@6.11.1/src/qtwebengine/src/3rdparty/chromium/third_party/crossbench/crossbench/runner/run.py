# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import enum
import logging
from typing import TYPE_CHECKING, Iterable, Optional, Set, Type

from typing_extensions import override

from crossbench import path as pth
from crossbench.browsers.splash_screen import SplashScreenData
from crossbench.cli import ui
from crossbench.cli.config.env import ValidationMode
from crossbench.env.run_env import RunEnv
from crossbench.env.runner_env import ValidationError
from crossbench.exception import Annotator, TInfoStack
from crossbench.helper.cwd import ChangeCWD
from crossbench.helper.durations import Durations
from crossbench.helper.state import State, StateMachine
from crossbench.probes.probe_context import ProbeContext
from crossbench.probes.results import ProbeResultDict
from crossbench.runner.actions import Actions
from crossbench.runner.exception import StopStoryException
from crossbench.runner.probe_context_manager import ProbeContextManager
from crossbench.runner.result_origin import ResultOrigin
from crossbench.runner.run_annotation import RunAnnotation
from crossbench.str_enum_with_help import StrEnumWithHelp

if TYPE_CHECKING:
  from selenium.webdriver.common.options import ArgOptions

  from crossbench.action_runner.base import ActionRunner
  from crossbench.benchmarks.base import Benchmark
  from crossbench.browsers.browser import Browser
  from crossbench.cli.config.secrets import Secrets
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.helper.wait import WaitRange
  from crossbench.probes.probe import Probe, ProbeT
  from crossbench.results_db.db import ResultsDB
  from crossbench.runner.groups.session import BrowserSessionRunGroup
  from crossbench.runner.runner import Runner
  from crossbench.runner.timing import AnyTimeUnit, Timing
  from crossbench.stories.story import Story
  from crossbench.types import JsonDict


@enum.unique
class Temperature(StrEnumWithHelp):
  COLD = ("cold", "first run")
  WARM = ("warm", "second run")
  HOT = ("hot", "third run")


class Run(ResultOrigin):

  def __init__(
      self,
      runner: Runner,
      browser_session: BrowserSessionRunGroup,
      story: Story,
      action_runner: ActionRunner,
      repetition: int,
      is_warmup: bool,
      temperature: str,
      index: int,
      name: Optional[str] = None,
      timeout: dt.timedelta = dt.timedelta(),
      throw: bool = False,
      env_validation_mode: ValidationMode = ValidationMode.THROW) -> None:
    super().__init__()
    self._state = StateMachine(State.INITIAL)
    self._runner = runner
    self._browser_session = browser_session
    self._browser: Browser = browser_session.browser
    browser_session.append(self)
    self._env = RunEnv(self, self._browser.settings.env_config,
                       env_validation_mode)
    self._story = story
    self._action_runner = action_runner
    assert repetition >= 0
    self._repetition = repetition
    self._is_warmup = is_warmup
    assert temperature, "Missing cache-temperature value."
    self._temperature = temperature
    assert index >= 0
    self._index = index
    self._name = name
    self._out_dir = self._get_out_dir().absolute()
    self._probe_results = ProbeResultDict(self._out_dir)
    self._durations = Durations()
    self._start_datetime = dt.datetime.utcfromtimestamp(0)
    self._timeout = timeout
    self._exceptions = Annotator(throw)
    self._browser_tmp_dir: pth.AnyPath | None = None
    self._probe_context_manager = ProbeRunContextManager(
        self, self._probe_results)
    self._annotations: Set[RunAnnotation] = set()

  def __str__(self) -> str:
    return f"Run({self.name}, state={self._state}, {self.browser})"

  def _get_out_dir(self) -> pth.LocalPath:
    return (self._browser_session.browser_dir / "stories" /
            pth.safe_filename(self.story.name) / str(self.repetition_name) /
            str(self._temperature))

  @property
  def group_dir(self) -> pth.LocalPath:
    return self.out_dir.parent

  def actions(self,
              name: str,
              verbose: bool = False,
              measure: bool = True) -> Actions:
    return Actions(name, self, verbose=verbose, measure=measure)

  def wait_range(self,
                 min_interval: AnyTimeUnit,
                 timeout: AnyTimeUnit,
                 delay: AnyTimeUnit = 0) -> WaitRange:
    return self.runner.wait_range(min_interval, timeout, delay)

  @property
  def info_stack(self) -> TInfoStack:
    return (
        f"Run({self.name})",
        (f"browser={self.browser.type_name()} label={self.browser.label} "
         f"binary={self.browser.path}"),
        f"story={self.story}",
        f"repetition={self.repetition_name}",
    )

  def details_json(self) -> JsonDict:
    return {
        "cwd": str(self.out_dir),
        "name": self.name,
        "story": self.story.details_json(),
        "browser": self.get_browser_details_json(),
        "run": {
            "name": self.name,
            "index": self.index,
            "repetition": self.repetition,
            "temperature": self.temperature,
            "isWarmup": self.is_warmup,
        },
        "session": {
            "index": self.browser_session.index,
            "cwd": str(self.browser_session.path)
        },
        "probes": self.results.to_json(),
        "timing": {
            "startDateTime": str(self.start_datetime),
            "duration": self.story.duration.total_seconds(),
            "durations": self.durations.to_json(),
            "timeout": self.timeout.total_seconds(),
            "global": self.timing.to_json(),
        },
        "success": self.is_success,
        "annotations": [
            annotation.to_json() for annotation in self._annotations
        ],
        "errors": self.exceptions.error_messages()
    }

  @property
  def temperature(self) -> str:
    return self._temperature

  @property
  def timing(self) -> Timing:
    return self.runner.timing

  @property
  def durations(self) -> Durations:
    return self._durations

  @property
  def start_datetime(self) -> dt.datetime:
    return self._start_datetime

  def max_end_datetime(self) -> dt.datetime:
    if not self._timeout:
      return dt.datetime.max
    return self._start_datetime + self._timeout

  @property
  def timeout(self) -> dt.timedelta:
    return self._timeout

  @property
  def repetition_name(self) -> str:
    if self.is_warmup:
      return f"warmup_{self.repetition}"
    return str(self.repetition)

  @property
  def repetition(self) -> int:
    return self._repetition

  @property
  def is_warmup(self) -> bool:
    return self._is_warmup

  @property
  def index(self) -> int:
    return self._index

  @property
  @override
  def runner(self) -> Runner:
    return self._runner

  @property
  def action_runner(self) -> ActionRunner:
    return self._action_runner

  @property
  def results_db(self) -> ResultsDB:
    return self.runner.results_db

  @property
  def benchmark(self) -> Benchmark:
    return self._runner.benchmark

  @property
  def browser_session(self) -> BrowserSessionRunGroup:
    return self._browser_session

  @property
  @override
  def browser(self) -> Browser:
    return self._browser

  @property
  def environment(self) -> RunnerEnv:
    # TODO: replace with custom BrowserEnvironment
    return self.runner.env

  @property
  @override
  def out_dir(self) -> pth.LocalPath:
    """A local directory where all result files are gathered.
    Results from browsers on remote platforms are transferred to this dir
    as well."""
    return self._out_dir

  @property
  @override
  def browser_tmp_dir(self) -> pth.AnyPath:
    """Returns a path to a tmp dir on the browser platform."""
    if not self._browser_tmp_dir:
      prefix = "cb_run_results"
      self._browser_tmp_dir = self.browser_platform.mkdtemp(prefix)
    return self._browser_tmp_dir

  @property
  def results(self) -> ProbeResultDict:
    return self._probe_results

  @property
  def story(self) -> Story:
    return self._story

  @property
  def name(self) -> Optional[str]:
    return self._name

  @property
  @override
  def exceptions(self) -> Annotator:
    return self._exceptions

  @property
  def is_success(self) -> bool:
    return self._exceptions.is_success

  @property
  def session(self) -> BrowserSessionRunGroup:
    return self._browser_session

  def annotate(self, annotation: RunAnnotation) -> None:
    self._annotations.add(annotation)

  @property
  def annotations(self) -> Iterable[RunAnnotation]:
    return iter(self._annotations)

  @property
  def secrets(self) -> Secrets:
    return self.story.secrets.merge(fallback=self.browser.secrets)

  @property
  def create_symlinks(self) -> bool:
    return self.runner.create_symlinks

  def get_browser_details_json(self) -> JsonDict:
    details_json = self.browser.details_json()
    self.session.add_flag_details(details_json)
    return details_json

  def get_local_probe_result_path(self, probe: Probe) -> pth.LocalPath:
    file = self._out_dir / probe.result_path_name
    assert not file.exists(), f"Probe results file exists already. file={file}"
    return file

  def validate_env(self, env: RunnerEnv) -> None:
    """Called before starting a browser / browser session to perform
    a pre-run checklist."""

  def setup(self, is_dry_run: bool) -> None:
    self._state.transition(State.INITIAL, to=State.SETUP)
    self._setup_dirs()
    with ChangeCWD(self._out_dir), self.exception_info(*self.info_stack):
      self._probe_context_manager.setup(self.probes, is_dry_run)
    self._log_setup()

  def setup_selenium_options(self, options: ArgOptions) -> None:
    # TODO: move explicitly to session.
    self._probe_context_manager.setup_selenium_options(options)

  def _setup_dirs(self) -> None:
    self._start_datetime = dt.datetime.now()
    logging.debug("Creating Run(%s) out dir: %s", self, self._out_dir)
    self._out_dir.mkdir(parents=True, exist_ok=True)
    if not self.runner.create_symlinks:
      logging.debug("Symlinks disabled by command line option")
      return
    self._setup_runs_dir()
    self._setup_session_dir()

  def _setup_runs_dir(self) -> None:
    browser_dir = self.browser_session.browser_dir
    runs_dir = browser_dir / "runs"
    runs_dir.mkdir(parents=True, exist_ok=True)
    if not self.create_symlinks:
      return
    # Source: BROWSER / "runs" / RUN
    # Target: BROWSER / "stories" / STORY / REPETITION / CACHE_TEMP
    run_dir = runs_dir / str(self.index)
    relative_out_dir = (
        pth.LocalPath("../") / self.out_dir.relative_to(browser_dir))
    run_dir.symlink_to(relative_out_dir, target_is_directory=True)

  def _setup_session_dir(self) -> None:
    session_run_dir = self._out_dir / "session"
    assert not session_run_dir.exists(), (
        f"Cannot setup session dir twice: {session_run_dir}")
    if not self.create_symlinks:
      return
    # Source: BROWSER / "stories" / STORY / REPETITION / CACHE_TEMP / "session"
    # Target: BROWSER / "sessions" / SESSION
    relative_session_dir = (
        pth.LocalPath("../../../..") /
        self.browser_session.path.relative_to(self.out_dir.parents[3]))
    session_run_dir.symlink_to(relative_session_dir, target_is_directory=True)

  def _log_setup(self) -> None:
    logging.debug("SETUP")
    logging.debug(
        "PROBES: %s",
        ", ".join(probe.NAME for probe in self.probes if not probe.is_internal))
    logging.debug("PROBES ALL: %s",
                  ", ".join(probe.NAME for probe in self.probes))
    self.story.log_run_details(self)
    logging.info("📁 RUN DIR:                  %s", self._out_dir)
    logging.debug("CWD %s", self._out_dir)

  def run(self, is_dry_run: bool) -> None:
    self._state.transition(State.SETUP, to=State.READY)
    self._start_datetime = dt.datetime.now()
    with ChangeCWD(self._out_dir), self.exception_info(*self.info_stack):
      assert self._probe_context_manager.is_ready
      try:
        self._run(is_dry_run)
      except Exception as e:  # pylint: disable=broad-except
        self._exceptions.append(e)
      finally:
        self.teardown(is_dry_run)

  def _run(self, is_dry_run: bool) -> None:
    self._state.transition(State.READY, to=State.RUN)
    if not is_dry_run:
      self._run_splashscreen()
    with self._probe_context_manager.open(is_dry_run):
      logging.info("RUNNING STORY")
      self._state.expect(State.RUN)
      try:
        with self.measure("run"), ui.spinner(), self.exceptions.capture():
          if not is_dry_run:
            self._run_story()
      except TimeoutError as e:
        # Handle TimeoutError earlier since they might be caused by
        # throttled down non-foreground browser.
        self._exceptions.append(e)
      if self.is_success:
        self._run_success_validation()

  def _run_splashscreen(self) -> None:
    with self.actions("SplashScreen") as actions:
      display_data = SplashScreenData(self.is_warmup, self.browser,
                                      self.details_json())
      self.browser.settings.splash_screen.run(actions, display_data)

  def _run_story(self) -> None:
    self._run_story_setup()
    if delay := self.timing.start_delay:
      self._run_story_wait(delay, "Start Delay")
    try:
      self._story.run(self)
    except StopStoryException as e:
      logging.debug("Stop story: %s", e)
    finally:
      if delay := self.timing.stop_delay:
        self._run_story_wait(delay, "Stop Delay")
      self._run_story_teardown()

  def _run_story_wait(self, delay: dt.timedelta, label: str) -> None:
    logging.info("%s: %s", label, delay)
    self.runner.wait(delay, absolute_time=True)

  def _run_story_setup(self) -> None:
    with self.measure("story-setup"):
      self._story.setup(self)
    self._probe_context_manager.start_story()

  def _run_story_teardown(self) -> None:
    self._probe_context_manager.stop_story()
    with self.measure("story-tear-down"):
      self._story.teardown(self)

  def _run_success_validation(self) -> None:
    try:
      self.environment.check_browser_focused(self.browser)
    except ValidationError as e:
      self.annotate(RunAnnotation.warning(str(e)))

  def teardown(self, is_dry_run: bool) -> None:
    self._state.transition(State.RUN, to=State.DONE)
    try:
      self._teardown_browser(is_dry_run)
      self._probe_context_manager.teardown(is_dry_run)
      if not is_dry_run:
        self._rm_browser_tmp_dir()
    finally:
      self.teardown_write_results_db()

  def teardown_write_results_db(self) -> None:
    self.results_db.teardown_run(self)

  def _teardown_browser(self, is_dry_run: bool) -> None:
    if is_dry_run:
      return
    if not self.browser_session.is_last_run(self):
      logging.debug("Skipping browser teardown (not last in session): %s", self)
      return
    if self._browser.is_running is False:
      logging.warning("Browser is no longer running (crashed or closed).")
      return
    with self.measure("browser-teardown"), self._exceptions.capture(
        "Quit browser"):
      try:
        self._browser.quit()
      except Exception as e:  # pylint: disable=broad-except
        logging.warning("Error quitting browser: %s", e)
        return

  def _rm_browser_tmp_dir(self) -> None:
    if not self._browser_tmp_dir:
      return
    self.browser_platform.rm(self._browser_tmp_dir, dir=True)

  def log_results(self) -> None:
    for probe in self.probes:
      probe.log_run_result(self)

  def log_failure(self) -> None:
    assert not self.is_success
    self._exceptions.log(f"❗ RUN {self.index+1} GOT ERRORS", separator="-")

  def log_annotations(self) -> None:
    if not self._annotations:
      return
    logging.info("- " * 40)
    RunAnnotation.log_all(self.annotations, limit=10)

  def find_probe_context(
      self, probe_cls: Type[ProbeT]) -> Optional[ProbeContext[ProbeT]]:
    return self._probe_context_manager.find_probe_context(probe_cls)


class ProbeRunContextManager(ProbeContextManager[Run, ProbeContext]):

  def __init__(self, run: Run, probe_results: ProbeResultDict) -> None:
    super().__init__(run, probe_results)

  @property
  def run(self) -> Run:
    return self._origin

  def get_probe_context(self, probe: Probe) -> Optional[ProbeContext]:
    return probe.get_context(self.run)

  def setup_selenium_options(self, options: ArgOptions) -> None:
    for probe_context in self._probe_contexts.values():
      probe_context.setup_selenium_options(options)

  def start_story(self) -> None:
    with self._measure("probes-start_story_run"):
      for probe_context in self._probe_contexts.values():
        with self.run.exception_capture(
            f"Probe {probe_context.name} start_story_run"):
          probe_context.start_story_run()

  def stop_story(self) -> None:
    with self._measure("probes-stop_story_run"):
      for probe_context in self._probe_contexts.values():
        with self.run.exception_capture(
            f"Probe {probe_context.name} stop_story_run"):
          probe_context.stop_story_run()
