# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import logging
import time
from typing import TYPE_CHECKING, Final, Optional

from typing_extensions import override

import crossbench.path as pth
from crossbench.cli import ui
from crossbench.probes.profiling.context.base import PosixProfilingContext
from crossbench.probes.profiling.enum import TargetMode

if TYPE_CHECKING:
  from crossbench.probes.profiling.system_profiling import ProfilingProbe
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


_MAC_TRACE_TEMPLATE_PATH: Final[pth.LocalPath] = pth.LocalPath(
    __file__).parents[1] / "time-profile.tracetemplate"

_XPATH_EXPRESSION: Final[str] = (
    "//trace-toc/run/data/table["
    "@category=\"PointsOfInterest\" and @schema=\"os-signpost\"]|"
    "//trace-toc/run/data/table[@schema=\"cpu-profile\"]")


class MacOSProfilingContext(PosixProfilingContext):

  def __init__(self, probe: ProfilingProbe, run: Run) -> None:
    super().__init__(probe, run)
    assert self.probe.target in (
        TargetMode.SYSTEM_WIDE, TargetMode.RENDERER_PROCESS_ONLY), (
            f"Unsupported profiling mode for Mac: {str(self.probe.target)}")

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    return super().get_default_result_path().parent / "profile.trace"

  def _start_xctrace(self, pid: Optional[int] = None) -> None:
    assert self.browser_platform.is_file(_MAC_TRACE_TEMPLATE_PATH), (
        f"Didn't find {_MAC_TRACE_TEMPLATE_PATH}")

    atexit.register(self.stop_process)

    process_filter = ["--all-processes"
                     ] if pid is None else ["--attach", str(pid)]
    self._profiling_process = self.browser_platform.popen(
        "xctrace", "record", "--template", _MAC_TRACE_TEMPLATE_PATH,
        *process_filter, "--output", self.result_path)
    # xctrace takes some time to start up
    time.sleep(3)
    if self._profiling_process.poll():
      raise ValueError("Could not start xctrace")

  def start(self) -> None:
    pass

  @override
  def start_story_run(self) -> None:
    super().start_story_run()
    # In theory this could start earlier but we leave it here as the
    # renderer-process mode requires us to run when we are guaranteed
    # to have a renderer available.
    if self.probe.target == TargetMode.SYSTEM_WIDE:
      self._start_xctrace()
    elif self.probe.target == TargetMode.RENDERER_PROCESS_ONLY:
      self._start_xctrace(self.renderer_pid_tid[0])

  def stop(self) -> None:
    # Needs to be SIGINT for xctrace, terminate won't work.
    assert self._profiling_process
    self.browser_platform.send_signal(self._profiling_process,
                                      self.browser_platform.signals.SIGINT)

  def teardown(self) -> ProbeResult:
    self.stop_process()
    trace_xml_path = self._export_trace_xml()
    return self.browser_result(file=(self.result_path,), xml=(trace_xml_path,))

  def _export_trace_xml(self) -> pth.AnyPath:
    trace_xml_path = self.result_path.with_name("profile.trace.xml")
    with self.run.actions(
        f"Probe {self.probe.name}: Exporting {trace_xml_path.name}",
        verbose=True), ui.spinner():
      self.browser_platform.sh("xctrace", "export", "--input", self.result_path,
                               "--output", trace_xml_path, "--xpath",
                               _XPATH_EXPRESSION)
      return trace_xml_path

  def stop_process(self) -> None:
    if not self._profiling_process:
      return
    logging.info("  Waiting for xctrace profiles (slow)...")
    with ui.spinner():
      self.browser_platform.terminate_gracefully(
          self._profiling_process,
          signal=self.browser_platform.signals.SIGINT,
          timeout=60)
    self._profiling_process = None
    atexit.unregister(self.stop_process)
