# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import io
import logging
import subprocess
import time
from typing import TYPE_CHECKING, Iterable, Optional, cast

from typing_extensions import override

from crossbench.browsers.chromium_based.chromium_based import ChromiumBased
from crossbench.probes.profiling.context.base import PosixProfilingContext
from crossbench.probes.profiling.enum import CallGraphMode, TargetMode

if TYPE_CHECKING:
  import crossbench.path as pth
  from crossbench.plt.types import ListCmdArgs
  from crossbench.probes.results import ProbeResult


class AndroidProfilingContext(PosixProfilingContext):

  def _generate_command_line(self) -> ListCmdArgs:
    renderer_pid: int | None = None
    renderer_main_tid: int | None = None
    if self.probe.target in (TargetMode.RENDERER_MAIN_ONLY,
                             TargetMode.RENDERER_PROCESS_ONLY):
      renderer_pid, renderer_main_tid = self.renderer_pid_tid
    return generate_simpleperf_command_line(
        self.probe.target,
        str(self.run.browser.path),
        renderer_pid,
        renderer_main_tid,
        self.probe.call_graph_mode,
        self.probe.frequency,
        self.probe.count,
        self.probe.cpu,
        self.probe.events,
        self.probe.grouped_events,
        self.probe.add_counters,
        self.result_path,
    )

  def _start_simpleperf(self) -> None:
    command_line = self._generate_command_line()
    logging.info("Starting simpleperf with command line: %s.", command_line)
    self._profiling_process = self.browser_platform.popen(
        *command_line, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    # Wait a bit for simpleperf to start and (potentially) terminate on error.
    time.sleep(1)
    if self._profiling_process.poll():
      error_msg: str = ""
      if stdout := self._profiling_process.stdout:
        if isinstance(stdout, io.BufferedReader):
          error_msg = stdout.read().decode("utf-8")
          logging.error(error_msg)
      raise ValueError(f"Unable to start simpleperf. {error_msg}")
    atexit.register(self.stop_process)
    self.browser.performance_mark("probe-profiling-start")

  def _get_simpleperf_pids(self) -> list[int]:
    simpleperf_pids = []
    for process in self.browser_platform.processes():
      if process["name"] == "simpleperf":
        simpleperf_pids.append(process["pid"])
    return simpleperf_pids

  def _stop_existing_simpleperf(self) -> None:
    for simpleperf_pid in self._get_simpleperf_pids():
      logging.warning("Terminating existing simpleperf process: %d.",
                      simpleperf_pid)
      self.browser_platform.terminate(simpleperf_pid)

  def _cpu_mask(self, cpus: Iterable) -> str:
    assert max(cpus) < 32, "Cpu index too high"
    mask = 0
    for cpu in cpus:
      mask |= (1 << cpu)
    return f"{mask:x}"

  def _pin_renderer_main_core(self, cpu: int) -> None:
    _, renderer_main_tid = self.renderer_pid_tid
    self.browser_platform.sh("taskset", "-p", self._cpu_mask([cpu]),
                             str(renderer_main_tid))

  def get_default_result_path(self) -> pth.AnyPath:
    return super().get_default_result_path().parent / "simpleperf.perf.data"

  def setup(self) -> None:
    assert self.browser.platform.is_android, (
        f"Expected Android platform, found {type(self.browser.platform)}.")
    assert self.browser.attributes().is_chromium_based, (
        f"Expected Chromium-based browser, found {type(self.browser)}.")
    if (self.browser.platform.is_android and
        self.browser.attributes().is_chromium_based):
      chromium = cast(ChromiumBased, self.browser)
      # Set `--enable-benchmarking-extension` explicitly for
      # retrieving Renderer PID, if needed.
      chromium.flags.enable_benchmarking_api()
    self._stop_existing_simpleperf()

  def start(self) -> None:
    if not self.probe.start_profiling_after_setup:
      self._start_simpleperf()

  @override
  def start_story_run(self) -> None:
    super().start_story_run()
    if self.probe.pin_renderer_main_core is not None:
      self._pin_renderer_main_core(self.probe.pin_renderer_main_core)

    if self.probe.start_profiling_after_setup:
      self._start_simpleperf()

  def stop(self) -> None:
    self.stop_process()

  def stop_process(self) -> None:
    if self._profiling_process:
      self.browser_platform.terminate_gracefully(
          self._profiling_process,
          timeout=30,
          signal=self.browser_platform.signals.SIGINT)
      self._profiling_process = None
      self.browser.performance_mark("probe-profiling-stop")

  def teardown(self) -> ProbeResult:
    return self.browser_result(trace=[self.result_path])


def generate_simpleperf_command_line(
    target: TargetMode,
    app_name: str,
    renderer_pid: Optional[int],
    renderer_main_tid: Optional[int],
    call_graph_mode: CallGraphMode,
    frequency: Optional[int | str],
    count: Optional[int],
    cpus: tuple[int, ...],
    events: tuple[str, ...],
    grouped_events: tuple[str, ...],
    add_counters: tuple[str, ...],
    output_path: pth.AnyPath,
) -> ListCmdArgs:
  command_line: ListCmdArgs = ["simpleperf", "record"]
  if target == TargetMode.RENDERER_MAIN_ONLY:
    assert renderer_main_tid is not None
    command_line.extend(["-t", str(renderer_main_tid)])
  elif target == TargetMode.RENDERER_PROCESS_ONLY:
    assert renderer_pid is not None
    command_line.extend(["-p", str(renderer_pid)])
  elif target == TargetMode.BROWSER_APP_ONLY:
    command_line.extend(["--app", app_name])
  else:  # TargetMode.SYSTEM_WIDE
    command_line.append("-a")
  if call_graph_mode == CallGraphMode.FRAME_POINTER:
    command_line.extend(["--call-graph", "fp"])
  elif call_graph_mode == CallGraphMode.DWARF:
    # Use "--post-unwind=yes" while unwinding with DWARF, to reduce
    # unwinding overhead during profiling.
    command_line.extend(["--call-graph", "dwarf", "--post-unwind=yes"])
  else:
    assert call_graph_mode == CallGraphMode.NO_CALL_GRAPH, (
        f"Invalid call_graph_mode: {call_graph_mode}")
  if frequency is not None:
    command_line.extend(["-f", str(frequency)])
  if count is not None:
    command_line.extend(["-c", str(count)])
  if cpus:
    command_line.extend(["--cpu", ",".join(map(str, cpus))])
  # Events and counters need to be provided after `-f` and `-c`.
  if events:
    command_line.extend(["-e", ",".join(events)])
  if grouped_events:
    command_line.extend(["--group", ",".join(grouped_events)])
  if add_counters:
    command_line.extend(["--add-counter", ",".join(add_counters)])
    # `--no-inherit` is required by simpleperf when `--add-counter` is used.
    command_line.append("--no-inherit")
  command_line.extend(["-o", output_path])
  return command_line
