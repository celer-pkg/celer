# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import contextlib
import enum
import logging
from typing import TYPE_CHECKING, Iterable, Iterator, Optional

from typing_extensions import override

from crossbench.flags.js_flags import JSFlags
from crossbench.helper.cwd import ChangeCWD
from crossbench.helper.durations import Durations
from crossbench.helper.state import BaseState, StateMachine
from crossbench.probes.probe_context import ProbeSessionContext
from crossbench.probes.results import EmptyProbeResult, ProbeResultDict
from crossbench.runner.groups.base import RunGroup
from crossbench.runner.probe_context_manager import ProbeContextManager
from crossbench.runner.result_origin import ResultOrigin

if TYPE_CHECKING:
  from selenium.webdriver.common.options import ArgOptions

  from crossbench.benchmarks.base import Benchmark
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.exception import TInfoStack
  from crossbench.flags.base import Flags
  from crossbench.network.base import Network
  from crossbench.path import AnyPath, LocalPath
  from crossbench.probes.probe import Probe
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run
  from crossbench.runner.timing import Timing
  from crossbench.types import JsonDict, JsonMapping


@enum.unique
class State(BaseState):
  BUILDING = enum.auto()
  READY = enum.auto()
  SETUP = enum.auto()
  STARTING = enum.auto()
  RUNNING = enum.auto()
  STOPPING = enum.auto()
  DONE = enum.auto()


class BrowserSessionRunGroup(RunGroup, ResultOrigin):
  """
  Groups Run objects together that are run within the same browser session.
  At the beginning of a new session the caches are cleared and the
  browser is (re-)started.
  """

  def __init__(self, env: RunnerEnv, probes: Iterable[Probe], browser: Browser,
               extra_flags: Flags, index: int, root_dir: LocalPath,
               create_symlinks: bool, throw: bool) -> None:
    super().__init__(throw)
    self._state: StateMachine[State] = StateMachine(State.BUILDING)
    self._env = env
    self._create_symlinks = create_symlinks
    self._probes: tuple[Probe, ...] = tuple(probes)
    self._durations = Durations()
    self._browser = browser
    self._network: Network = browser.network
    self._index: int = index
    self._runs: list[Run] = []
    self._root_dir: LocalPath = root_dir
    self._browser_tmp_dir: AnyPath | None = None
    self._extra_js_flags = JSFlags()
    self._extra_flags = extra_flags
    # Temporary objects, reset after all runs are ready (see set_ready).
    self._probe_results = ProbeResultDict(root_dir)
    self._probe_session_context_manager = ProbeSessionContextManager(
        self, self._probe_results)

  def append(self, run: Run) -> None:
    self._state.expect(State.BUILDING)
    assert run.browser_session == self
    assert run.browser is self._browser
    # TODO: assert that the runs have compatible flags (likely we're only
    # allowing changes in the cache temperature)
    # TODO: Add session/run switch for probe results
    self._runs.append(run)

  def set_ready(self) -> None:
    self._state.transition(State.BUILDING, to=State.READY)
    self._validate()
    self._set_path(self._get_session_dir())
    self._probe_results = ProbeResultDict(self.path)
    self._probe_session_context_manager = ProbeSessionContextManager(
        self, self._probe_results)

  def _validate(self) -> None:
    if not self._runs:
      raise ValueError("BrowserSessionRunGroup must be non-empty.")
    self.browser.validate_env(self.env)
    for run in self.runs:
      run.validate_env(self.env)
    self._validate_same_browser_probes()

  def _validate_same_browser_probes(self) -> None:
    first_run = self._runs[0]
    first_probes = tuple(first_run.probes)
    for index, run in enumerate(self.runs):
      if first_run.browser is not run.browser:
        raise ValueError("A browser session can only contain "
                         "Runs with the same Browser.\n"
                         f"runs[0].browser == {first_run.browser} vs. "
                         f"runs[{index}].browser == {run.browser}")
      if first_probes != tuple(run.probes):
        raise ValueError("Got conflicting Probes within a browser session.\n"
                         "All runs must have the same probes within a session.")

  @property
  def raw_session_dir(self) -> LocalPath:
    return (self.root_dir / self.browser.unique_name / "sessions" /
            str(self.index))

  @property
  def is_single_run(self) -> bool:
    return len(self._runs) == 1

  @property
  def first_run(self) -> Run:
    return self._runs[0]

  def _get_session_dir(self) -> LocalPath:
    self._state.expect_at_least(State.READY)
    if self.is_single_run:
      return self.first_run.out_dir
    if not self._runs:
      raise ValueError("Cannot have empty browser session")
    return self.raw_session_dir

  @property
  @override
  def out_dir(self) -> LocalPath:
    return self._get_session_dir()

  @property
  def browser_dir(self) -> LocalPath:
    return self.root_dir / self.browser.unique_name

  @property
  def durations(self) -> Durations:
    return self._durations

  @property
  def env(self) -> RunnerEnv:
    return self._env

  @property
  @override
  def probes(self) -> Iterable[Probe]:
    return iter(self._probes)

  @property
  def network(self) -> Network:
    return self._network

  @property
  @override
  def browser(self) -> Browser:
    return self._browser

  @property
  def benchmark(self) -> Benchmark:
    return self.first_run.benchmark

  @property
  def index(self) -> int:
    return self._index

  @property
  def is_running(self) -> bool:
    return self._state == State.RUNNING

  @property
  def root_dir(self) -> LocalPath:
    return self._root_dir

  @property
  @override
  def runs(self) -> Iterable[Run]:
    return iter(self._runs)

  @property
  def timing(self) -> Timing:
    return self._runs[0].timing

  @property
  def extra_js_flags(self) -> JSFlags:
    self._state.expect_before(State.RUNNING)
    return self._extra_js_flags

  @property
  def extra_flags(self) -> Flags:
    self._state.expect_before(State.RUNNING)
    return self._extra_flags

  def add_flag_details(self, details_json: JsonDict) -> None:
    assert isinstance(details_json["js_flags"], tuple)
    details_json["js_flags"] += tuple(self._extra_js_flags)
    assert isinstance(details_json["flags"], tuple)
    details_json["flags"] += tuple(self._extra_flags)

  def setup_selenium_options(self, options: ArgOptions) -> None:
    # Using only the first run, since all runs need to have the same probes.
    self.first_run.setup_selenium_options(options)

  @property
  @override
  def info_stack(self) -> TInfoStack:
    return ("Merging results from multiple browser sessions",
            f"browser={self.browser.unique_name}", f"session={self.index}")

  @property
  @override
  def info(self) -> JsonMapping:
    info_dict = dict(super().info)
    info_dict.update({"index": self.index})
    return info_dict

  def __str__(self) -> str:
    return f"Session({self.browser}, {self.index})"

  @property
  @override
  def browser_tmp_dir(self) -> AnyPath:
    if not self._browser_tmp_dir:
      prefix = f"cb_browser_session_{self.index}"
      self._browser_tmp_dir = self.browser_platform.mkdtemp(prefix)
    return self._browser_tmp_dir

  @override
  def merge(self, probes: Iterable[Probe]) -> None:
    # TODO: implement merging of session probes
    pass

  @override
  def _merge_probe_results(self, probe: Probe) -> ProbeResult:
    return EmptyProbeResult()

  @contextlib.contextmanager
  def open(self, is_dry_run: bool = False) -> Iterator[bool]:
    self._state.transition(State.READY, to=State.SETUP)
    yielded = False
    with self.exceptions.capture():
      self._setup_session_dir()
      self._setup_browser()
      with ChangeCWD(self.path):
        with self._open(is_dry_run):
          yielded = True
          yield self.is_success
    # Contextmanager always needs to yield, even in the case of early
    # exceptions, the caller is responsible for skipping the body.
    if not yielded:
      assert not self.is_success
      yield False

  @contextlib.contextmanager
  def _open(self, is_dry_run: bool) -> Iterator[None]:
    self._state.expect(State.SETUP)
    with self.measure("browser-session-setup"):
      self._setup(is_dry_run)
    try:
      with self._start_network(is_dry_run), self._start_probes(is_dry_run):
        self._start(is_dry_run)
        try:
          self._state.expect(State.RUNNING)
          yield
        except Exception as e:
          logging.debug(
              "BrowserSessionRunGroup: got unexpected inner exception: %s", e)
          raise e
    finally:
      self._teardown(is_dry_run)

  def _setup(self, is_dry_run: bool) -> None:
    self._state.expect(State.SETUP)
    self._probe_session_context_manager.setup(self.probes, is_dry_run)
    # TODO: handle session vs run probe.
    for run in self.runs:
      with self._exceptions.annotate(f"Setting up {run}"):
        label = "RUN"
        if run.is_warmup:
          label = "WARMUP RUN"
        logging.info("Preparing SESSION %s, %s %s", self.index, label,
                     run.index)
        run.setup(is_dry_run)

  def _setup_browser(self) -> None:
    self._state.expect(State.SETUP)
    self.browser.setup()

  def _setup_session_dir(self) -> None:
    self._state.expect(State.SETUP)
    with self.measure("browser-session-setup-dir"):
      self.path.mkdir(parents=True, exist_ok=True)
      if not self._create_symlinks:
        logging.debug("Symlink disabled by command line option")
        return
      if self.is_single_run:
        # If there is a single run per session we reuse the run-dir.
        self.raw_session_dir.parent.mkdir(parents=True, exist_ok=True)
        self.raw_session_dir.symlink_to(self.path)

  @contextlib.contextmanager
  def _start_network(self, is_dry_run: bool = False):
    logging.debug("Starting network: %s", self.network)
    if is_dry_run:
      yield
      return
    with self._exceptions.annotate(f"Starting Network: {self.network}"):
      with self.network.open(self):
        yield

  @contextlib.contextmanager
  def _start_probes(self, is_dry_run: bool):
    with self._exceptions.annotate("Starting Session Probes"):
      with self._probe_session_context_manager.open(is_dry_run):
        yield

  def _start(self, is_dry_run: bool) -> None:
    self._state.transition(State.SETUP, to=State.STARTING)
    with self.measure("browser-session-start"):
      with self._exceptions.annotate(f"Starting Browser: {self.browser}"):
        self._start_browser(is_dry_run)
        self._state.transition(State.STARTING, to=State.RUNNING)

  def _start_browser(self, is_dry_run: bool) -> None:
    self._state.expect(State.STARTING)
    if is_dry_run:
      logging.info("BROWSER: %s", self.browser.path)
      return
    assert self.network.is_running, "Network isn't running yet"
    assert self._probe_session_context_manager.is_running
    browser_log_file = self.path / "browser.log"
    assert not browser_log_file.exists(), (
        f"Default browser log file {browser_log_file} already exists.")
    self._browser.set_log_file(browser_log_file)

    with self.measure("browser-setup"):
      try:
        # pytype somehow gets the package path wrong here, disabling for now.
        self._browser.start(self)
        assert self._browser.is_running, "Browser did not start up correctly"
      except Exception as e:
        logging.debug("Browser setup failed: %s", e)
        # Clean up half-setup browser instances
        self._browser.force_quit()
        raise

  def _teardown(self, is_dry_run: bool) -> None:
    self._state.transition(
        State.SETUP, State.STARTING, State.RUNNING, to=State.STOPPING)
    with self.measure("browser-session-teardown"):
      try:
        self._stop_browser(is_dry_run)
      finally:
        self._state.transition(State.STOPPING, to=State.DONE)
    self._probe_session_context_manager.teardown(is_dry_run)

  def _stop_browser(self, is_dry_run: bool) -> None:
    self._state.expect(State.STOPPING)
    # TODO: move complete implementation here
    # This can happen if a browser / probe setup error occurs and we're
    # in a unclean state.
    if self.browser.is_running:
      self._runs[-1]._teardown_browser(is_dry_run)  # pylint: disable=protected-access

  def handle_startup_failure(self) -> None:
    runs = tuple(self.runs)
    self.exceptions.log(f"SESSION STARTUP ERRORS: Skipping {len(runs)} runs")
    for run in runs:
      with self.exception_capture(f"Processing startup failures for {run}"):
        run.exceptions.extend(self.exceptions)
        run.teardown_write_results_db()

  # TODO: remove once cleanly implemented
  def is_first_run(self, run: Run) -> bool:
    return self.first_run is run

  # TODO: remove once cleanly implemented
  def is_last_run(self, run: Run) -> bool:
    return self._runs[-1] is run


class ProbeSessionContextManager(ProbeContextManager[BrowserSessionRunGroup,
                                                     ProbeSessionContext]):

  def __init__(self, session: BrowserSessionRunGroup,
               probe_results: ProbeResultDict) -> None:
    super().__init__(session, probe_results)

  def get_probe_context(self, probe: Probe) -> Optional[ProbeSessionContext]:
    return probe.get_session_context(self._origin)
