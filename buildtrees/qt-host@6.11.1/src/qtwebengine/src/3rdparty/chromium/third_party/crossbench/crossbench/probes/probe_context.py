# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import contextlib
import datetime as dt
from typing import (TYPE_CHECKING, Generic, Iterable, Iterator, Optional,
                    TypeVar)

from typing_extensions import override

from crossbench.probes.results import (BrowserProbeResult, EmptyProbeResult,
                                       LocalProbeResult, ProbeResult)

if TYPE_CHECKING:
  from selenium.webdriver.common.options import BaseOptions

  from crossbench import plt
  from crossbench.browsers.browser import Browser
  from crossbench.path import AnyPath, LocalPath
  from crossbench.probes.probe import Probe
  from crossbench.runner.groups.session import BrowserSessionRunGroup
  from crossbench.runner.result_origin import ResultOrigin
  from crossbench.runner.run import Run
  from crossbench.runner.runner import Runner

# Redefine here to avoid circular imports
ProbeT = TypeVar("ProbeT", bound="Probe")


class BaseProbeContext(Generic[ProbeT], metaclass=abc.ABCMeta):
  """
    Base class for an activation of a probe where active data collection
    happens. See specific subclasses for implementations that can be used
    for data collection during runs or whole sessions.
    Override in Probe subclasses to implement actual performance data
    collection.
    - The data should be written to self.result_path.
    - A file / list / dict of result file Paths should be returned by the
      override teardown() method
  """

  def __init__(self, probe: ProbeT, result_origin: ResultOrigin) -> None:
    self._probe: ProbeT = probe
    self._result_origin = result_origin
    self._is_active: bool = False
    self._is_success: bool = False
    self._start_time: dt.datetime | None = None
    self._stop_time: dt.datetime | None = None

  def set_start_time(self, start_datetime: dt.datetime) -> None:
    assert self._start_time is None
    self._start_time = start_datetime

  @contextlib.contextmanager
  def open(self) -> Iterator[None]:
    assert self._start_time
    assert not self._is_active
    assert not self._is_success

    with self.result_origin.exception_capture(f"Probe {self.name} start"):
      self._is_active = True
      self.start()

    try:
      yield
    finally:
      with self.result_origin.exception_capture(f"Probe {self.name} stop"):
        self.stop()
        self._is_success = True
        assert self._stop_time is None
      self._stop_time = dt.datetime.now()

  @property
  def probe(self) -> ProbeT:
    return self._probe

  @property
  def result_origin(self) -> ResultOrigin:
    return self._result_origin

  @property
  def browser_platform(self) -> plt.Platform:
    return self.browser.platform

  @property
  def host_platform(self) -> plt.Platform:
    return self.browser.host_platform

  @property
  def browser(self) -> Browser:
    return self._result_origin.browser

  @property
  def runner(self) -> Runner:
    return self._result_origin.runner

  @property
  @abc.abstractmethod
  def session(self) -> BrowserSessionRunGroup:
    pass

  @property
  def start_time(self) -> dt.datetime:
    """
    Returns a unified start time that is the same for all probe contexts
    within a run. This can be used to account for startup delays caused by other
    Probes.
    """
    assert self._start_time
    return self._start_time

  @property
  def duration(self) -> dt.timedelta:
    assert self._start_time and self._stop_time
    return self._stop_time - self._start_time

  @property
  def is_success(self) -> bool:
    return self._is_success

  @property
  @abc.abstractmethod
  def result_path(self) -> AnyPath:
    pass

  @property
  def local_result_path(self) -> LocalPath:
    return self.host_platform.local_path(self.result_path)

  @property
  def name(self) -> str:
    return self.probe.name

  @property
  def browser_pid(self) -> int:
    maybe_pid = self.browser.pid
    assert maybe_pid, "Browser is not runner or does not provide a pid."
    return maybe_pid

  def browser_result(self,
                     url: Optional[Iterable[str]] = None,
                     file: Optional[Iterable[AnyPath]] = None,
                     **kwargs: Iterable[AnyPath]) -> BrowserProbeResult:
    """Helper to create BrowserProbeResult that might be stored on a remote
    browser/device and need to be copied over to the local machine."""
    return BrowserProbeResult(self.result_origin, url=url, file=file, **kwargs)

  def local_result(self,
                   url: Optional[Iterable[str]] = None,
                   file: Optional[Iterable[LocalPath]] = None,
                   **kwargs: Iterable[LocalPath]) -> LocalProbeResult:
    """Helper to create LocalProbeResult."""
    return LocalProbeResult(url=url, file=file, **kwargs)

  def empty_result(self) -> EmptyProbeResult:
    return EmptyProbeResult()

  def setup(self) -> None:
    """
    Called before starting the browser, typically used to set run-specific
    browser flags.
    If an error is thrown here, *none* of the other hooks
    (start, stop, teardown) will be called.
    """

  @abc.abstractmethod
  def start(self) -> None:
    pass

  @abc.abstractmethod
  def stop(self) -> None:
    pass

  @abc.abstractmethod
  def teardown(self) -> ProbeResult:
    pass


class ProbeContext(BaseProbeContext[ProbeT], metaclass=abc.ABCMeta):
  """
  A scope during which a probe is actively collecting data during a Run.
  See BaseProbeContext additional usage.
  """

  def __init__(self, probe: ProbeT, run: Run) -> None:
    super().__init__(probe, run)
    self._run: Run = run
    self._default_result_path: AnyPath = self.get_default_result_path()

  def get_default_result_path(self) -> AnyPath:
    return self._run.get_default_probe_result_path(self._probe)

  @property
  def run(self) -> Run:
    return self._run

  @property
  @override
  def result_origin(self) -> ResultOrigin:
    return self._run

  @property
  @override
  def session(self) -> BrowserSessionRunGroup:
    return self._run.session

  @property
  @override
  def result_path(self) -> AnyPath:
    return self._default_result_path

  def setup_selenium_options(self, options: BaseOptions) -> None:
    """
    Custom hook to change selenium options before starting the browser.
    """
    # TODO: move to SessionContext
    del options

  @abc.abstractmethod
  @override
  def start(self) -> None:
    """
    Called immediately before starting the given Run, after the browser started.
    This method should have as little overhead as possible. If possible,
    delegate heavy computation to the "setup" method.
    """

  def start_story_run(self) -> None:
    """
    Called before running a Story's core workload (Story.run)
    and after running Story.setup.
    """

  def stop_story_run(self) -> None:
    """
    Called after running a Story's core workload (Story.run) and before running
    Story.teardown.
    """

  @abc.abstractmethod
  @override
  def stop(self) -> None:
    """
    Called immediately after finishing the given Run with the browser still
    running.
    This method should have as little overhead as possible. If possible,
    delegate heavy computation to the "teardown" method.
    """
    return None

  @abc.abstractmethod
  @override
  def teardown(self) -> ProbeResult:
    """
    Called after stopping all probes and shutting down the browser.
    Not called if an error was thrown in the setup method.
    Returns
    - None if no data was collected
    - If Data was collected:
      - Either a path (or list of paths) to results file
      - Directly a primitive json-serializable object containing the data
    """
    return self.empty_result()


class ProbeSessionContext(BaseProbeContext[ProbeT], metaclass=abc.ABCMeta):
  """
  A scope during which a probe is actively collecting data during an active
  browser session, which might span several runs.
  See BaseProbeContext additional usage.
  """

  def __init__(self, probe: ProbeT, session: BrowserSessionRunGroup) -> None:
    super().__init__(probe, session)
    self._session: BrowserSessionRunGroup = session
    self._default_result_path: AnyPath = self.get_default_result_path()

  def get_default_result_path(self) -> AnyPath:
    return self._session.get_default_probe_result_path(self._probe)

  @property
  @override
  def session(self) -> BrowserSessionRunGroup:
    return self._session

  @property
  @override
  def result_origin(self) -> ResultOrigin:
    return self._session

  @property
  @override
  def result_path(self) -> AnyPath:
    return self._default_result_path
