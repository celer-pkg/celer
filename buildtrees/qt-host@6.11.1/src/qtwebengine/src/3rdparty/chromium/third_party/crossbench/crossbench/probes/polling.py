# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import logging
import threading
import time
from typing import TYPE_CHECKING, Iterable, Self, Type

from typing_extensions import override

from crossbench.parse import DurationParser, ObjectParser
from crossbench.probes.probe import (Probe, ProbeConfigParser, ProbeContext,
                                     ProbeKeyT)
from crossbench.probes.results import LocalProbeResult, ProbeResult

if TYPE_CHECKING:
  from crossbench import plt
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.path import LocalPath
  from crossbench.plt.types import CmdArg, TupleCmdArgs
  from crossbench.runner.run import Run


class PollingProbe(Probe, metaclass=abc.ABCMeta):
  """
  Abstract probe to periodically collect the results of any bash cmd.
  """
  NAME = "polling"
  IS_GENERAL_PURPOSE = False

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "interval",
        type=DurationParser.positive_duration,
        default=dt.timedelta(seconds=1),
        help="Run the cmd at this interval and produce separate results.")
    return parser

  def __init__(
      self,
      cmd: Iterable[CmdArg],
      interval: dt.timedelta = dt.timedelta(seconds=1)
  ) -> None:
    super().__init__()
    self._cmd: TupleCmdArgs = tuple(cmd)
    self._interval = interval
    if interval.total_seconds() < 0.1:
      raise ValueError(f"Polling interval must be >= 0.1s, but got: {interval}")

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (("cmd", tuple(self.cmd)),
                          ("interval", self.interval.total_seconds()))

  @property
  def interval(self) -> dt.timedelta:
    return self._interval

  @property
  def cmd(self) -> TupleCmdArgs:
    return self._cmd

  @override
  def validate_env(self, env: RunnerEnv) -> None:
    super().validate_env(env)
    if env.repetitions != 1:
      env.handle_warning(f"Probe={self.NAME} cannot merge data over multiple "
                         f"repetitions={env.repetitions}.")

  @override
  def get_context_cls(self) -> Type[PollingProbeContext]:
    return PollingProbeContext


class PollingShellProbe(PollingProbe):
  """
  General-purpose probe to periodically collect the stdout of a given bash cmd.
  """

  IS_GENERAL_PURPOSE = True
  NAME = "poll"

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "cmd",
        type=ObjectParser.sh_cmd,
        required=True,
        help="Write stdout of this CMD as a result.")
    return parser


class PollingProbeContext(ProbeContext[PollingProbe]):
  _poller: CMDPoller

  def __init__(self, probe: PollingProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._poller = CMDPoller(self.browser_platform, self.probe.cmd,
                             self.probe.interval, self.local_result_path)

  @override
  def setup(self) -> None:
    self.local_result_path.mkdir()

  def start(self) -> None:
    self._poller.start()

  def stop(self) -> None:
    self._poller.stop()

  def teardown(self) -> ProbeResult:
    return LocalProbeResult(file=(self.local_result_path,))


class CMDPoller(threading.Thread):

  def __init__(self, platform: plt.Platform, cmd: Iterable[CmdArg],
               interval: dt.timedelta, path: LocalPath) -> None:
    super().__init__()
    self._platform = platform
    self._cmd: TupleCmdArgs = tuple(cmd)
    self._path: LocalPath = path
    if interval < dt.timedelta(seconds=0.1):
      raise ValueError("Poller interval should be >= 0.1s for accuracy, "
                       f"but got {interval}s")
    self._interval_seconds = interval.total_seconds()
    self._event = threading.Event()

  def stop(self) -> None:
    self._event.set()
    self.join()

  def run(self) -> None:
    start_time = time.monotonic_ns()
    while not self._event.is_set():
      poll_start = dt.datetime.now()

      data = self._platform.sh_stdout(*self._cmd)
      datetime_str = poll_start.strftime("%Y-%m-%d_%H%M%S_%f")
      out_file = self._path / f"{datetime_str}.txt"
      out_file.write_text(data, encoding="utf-8")

      poll_end = dt.datetime.now()
      diff = (poll_end - poll_start).total_seconds()
      if diff > self._interval_seconds:
        logging.warning("Poller command took longer than expected %fs: %s",
                        self._interval_seconds, self._cmd)

      # Calculate wait_time against fixed start time to avoid drifting.
      total_time = (time.monotonic_ns() - start_time) / 10.0**9
      wait_time = self._interval_seconds - (total_time % self._interval_seconds)
      self._event.wait(wait_time)
