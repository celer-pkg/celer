# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import subprocess
from typing import TYPE_CHECKING, Self, TextIO, Type

from typing_extensions import override

from crossbench.parse import PathParser
from crossbench.probes.probe import (Probe, ProbeConfigParser, ProbeContext,
                                     ProbeKeyT)
from crossbench.probes.probe_error import ProbeValidationError
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.path import LocalPath
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


class DTraceProbe(Probe):
  """
  Probe to collect data using DTrace.
  """

  NAME = "dtrace"
  RESULT_LOCATION = ResultLocation.BROWSER

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "script_path", required=True, type=PathParser.non_empty_file_path)
    return parser

  def __init__(self, script_path: LocalPath) -> None:
    super().__init__()
    self._script_path = script_path.resolve()

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (("script_path", str(self.script_path)),)

  @property
  def script_path(self) -> LocalPath:
    return self._script_path

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    self.expect_macos(browser)

    # Check that it is possible to execute "sudo dtrace" without prompting for a
    # password. The best way to do this is to actually run the command and check
    # the return value of `sudo`.
    #
    # Under normal usage, DTrace expects an input script file and returns "2"
    # when it is missing. To force a return value of zero, without actually
    # providing a valid script file, the "-l -P $dtrace_probe_name" argument is
    # used, which tells DTrace to simply print all DTrace probes (do not confuse
    # with crossbench probes) whose name matches $dtrace_probe_name. This will
    # ensure the command either succeeds or fails fast. Use a non-existent probe
    # name to reduce output size
    dtrace_probe_name = "nonexistantprobename"
    # Execute and check the returncode, while ignoring output.
    try:
      browser.platform.sh(
          "sudo",
          "-n",
          "dtrace",
          "-l",
          "-P",
          dtrace_probe_name,
          stdout=subprocess.DEVNULL,
          stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      raise ProbeValidationError(
          self, "Cannot execute 'sudo dtrace'. "
          "This probe will fail to start.") from e

  @override
  def get_context_cls(self) -> Type[DTraceProbeContext]:
    return DTraceProbeContext


class DTraceProbeContext(ProbeContext[DTraceProbe]):

  def __init__(self, probe: DTraceProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._script_path: LocalPath = probe.script_path
    self._output_path: LocalPath = (
        self.local_result_path.with_suffix(".output.txt"))
    self._log_path: LocalPath = self.local_result_path.with_suffix(".log")
    self._dtrace_process: subprocess.Popen | None = None
    self._log_file: TextIO | None = None
    atexit.register(self.stop_dtrace_process)

  def start(self) -> None:
    self._log_file = self._log_path.open("w", encoding="utf-8")
    # TODO: copy over script to remote machine
    assert self.browser_platform.is_local, "Remote dtrace not supported yet"
    self._dtrace_process = self.browser_platform.popen(
        "sudo",
        "-n",
        "dtrace",
        "-p",
        str(self.browser_pid),
        "-o",
        self._output_path,
        "-s",
        self._script_path,
        stdout=self._log_file,
        stderr=self._log_file,
    )
    assert self._dtrace_process is not None, ("Could not start DTrace")

  def stop(self) -> None:
    if not self._dtrace_process:
      return
    # DTrace will close by itself once the browser exits.
    returncode = self._dtrace_process.poll()
    if returncode is not None:
      # Print an error message if DTrace was already closed.
      if returncode == 0:
        raise RuntimeError("DTrace exited early without errors.")
      raise RuntimeError(f"DTrace exited early with error {returncode}.\n"
                         f"Check {self._log_path} for the program's log.")

  def teardown(self) -> ProbeResult:
    self.stop_dtrace_process()
    assert self._log_file, "Did not open log file."
    self._log_file.close()
    return self.browser_result(file=(self._output_path,))

  def stop_dtrace_process(self) -> None:
    if self._dtrace_process:
      try:
        # Wait for the process to terminate normally.
        returncode = self._dtrace_process.wait(timeout=5)
        if returncode != 0:
          raise RuntimeError(f"DTrace exited with error {returncode}.\n"
                             f"Check {self._log_path} for the program's log.")
      except subprocess.TimeoutExpired:
        # DTrace took too long to terminate. Send SIGKILL.
        # Note: Not using .kill() because the process was started with sudo so
        # it would raise an PermissionError exception.
        subprocess.run(
            ["sudo", "-n", "kill", "-SIGKILL", f"{self._dtrace_process.pid}"],
            check=True)
