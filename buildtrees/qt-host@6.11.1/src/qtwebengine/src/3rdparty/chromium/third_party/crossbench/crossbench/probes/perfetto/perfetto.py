# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import atexit
import logging
import subprocess
from typing import TYPE_CHECKING, Iterable, Self, cast

import google.protobuf.text_format as proto_text_format
from typing_extensions import override

from crossbench import path as pth
from crossbench.helper import fs_helper
from crossbench.helper.wait import WaitRange
from crossbench.parse import NumberParser, ObjectParser, PathParser
from crossbench.plt.android_adb import AndroidAdbPlatform
from crossbench.plt.chromeos_ssh import ChromeOsSshPlatform
from crossbench.probes.perfetto.downloader import PerfettoToolDownloader
from crossbench.probes.probe import (Probe, ProbeConfigParser, ProbeContext,
                                     ProbeKeyT)
from crossbench.probes.result_location import ResultLocation
from crossbench.probes.results import LocalProbeResult, ProbeResult
from protoc import trace_config_pb2

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.plt.types import TupleCmdArgs
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.run import Run

_PERFETTO_CONFIG_REMOTE_DIR_ANDROID = pth.AnyPath(
    "/data/misc/perfetto-configs/")
_PERFETTO_TRACE_REMOTE_DIR_ANDROID = pth.AnyPath("/data/misc/perfetto-traces/")
_PERFETTO_REMOTE_DIR_CROS = pth.AnyPath("/usr/local/tmp")


class PerfettoProbe(Probe):
  """
  A probe to collect Perfetto system traces that can be viewed on
  https://ui.perfetto.dev/. The probe supports Android and ChromeOS targets.

  Recommended way to use:
  1. Go to https://ui.perfetto.dev/, click "Record new trace" and set up your
     preferred tracing options.
  2. Click "Recording command" and copy the textproto config part of the
     command.
  3. Paste it into the textproto field of the probe config. An example probe
     config can be found at config/doc/probe/perfetto.config.hjson.
  4. Specify the config via the --probe-config command-line flag.

  After the run, the trace will be found among the results as
  "perfetto.trace.pb.gz".
  """
  NAME = "perfetto"
  RESULT_LOCATION = ResultLocation.BROWSER

  IS_GENERAL_PURPOSE = True

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "trace_config",
        aliases=("config", "textproto"),
        type=ObjectParser.proto_or_file(trace_config_pb2.TraceConfig),
        help=("Serialized perfetto configuration. "
              "See probe instructions for more details"))
    parser.add_argument(
        "perfetto_bin",
        type=PathParser.any_path,
        default=pth.AnyPath("perfetto"),
        help="Perfetto binary on the browser device (android, chrome-os)")
    parser.add_argument(
        "tracebox_bin",
        type=PathParser.any_path,
        default=pth.AnyPath("tracebox"),
        help="Tracebox binary on the browser device (linux, macos). "
        "Auto downloaded on local devices.")
    parser.add_argument(
        "trace_browser_startup",
        type=bool,
        default=False,
        help="Start perfetto tracing before launching the browser.")
    return parser

  def __init__(self,
               trace_config: trace_config_pb2.TraceConfig,
               perfetto_bin: pth.AnyPath,
               tracebox_bin: pth.AnyPath,
               trace_browser_startup: bool = False) -> None:
    super().__init__()
    if not trace_config:
      raise ValueError("Please specify a tracing config")
    self._trace_config: trace_config_pb2.TraceConfig = trace_config
    self._perfetto_bin = perfetto_bin
    self._tracebox_bin = tracebox_bin
    self._trace_browser_startup = trace_browser_startup

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (
        ("textproto", str(self.trace_config)),
        ("perfetto_bin", str(self.perfetto_bin)),
        ("tracebox_bin", str(self.tracebox_bin)),
        ("trace_browser_startup", str(self.trace_browser_startup)),
    )

  @property
  def trace_config(self) -> trace_config_pb2.TraceConfig:
    return self._trace_config

  @property
  def perfetto_bin(self) -> pth.AnyPath:
    return self._perfetto_bin

  @property
  def tracebox_bin(self) -> pth.AnyPath:
    return self._tracebox_bin

  @property
  def trace_browser_startup(self) -> bool:
    return self._trace_browser_startup

  @property
  @override
  def result_path_name(self) -> str:
    return "perfetto.trace.pb"

  @override
  def attach(self, browser: Browser) -> None:
    assert browser.attributes().is_chromium_based
    browser.features.enable("EnablePerfettoSystemTracing")
    super().attach(browser)

  @override
  def log_run_result(self, run: Run) -> None:
    self._log_results([run])

  @override
  def log_browsers_result(self, group: BrowsersRunGroup) -> None:
    self._log_results(group.runs)

  def _log_results(self, runs: Iterable[Run]) -> None:
    logging.info("-" * 80)
    logging.critical("Perfetto trace results:")
    for run in runs:
      result_file = run.results[self].file
      logging.critical("  - %s : %s", result_file,
                       fs_helper.get_file_size(result_file))

  def get_context(self, run: Run) -> PerfettoProbeContext:
    # TODO: support more platforms
    if run.browser_platform.is_chromeos:
      return ChromeOsPerfettoProbeContext(self, run)
    if run.browser_platform.is_android:
      return AndroidPerfettoProbeContext(self, run)
    return DesktopPerfettoProbeContext(self, run)


class PerfettoProbeContext(ProbeContext[PerfettoProbe], metaclass=abc.ABCMeta):
  def __init__(self, probe: PerfettoProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._host_config_file: pth.LocalPath = (
        run.out_dir / "perfetto_config.textproto")
    self._perfetto_pid: int | None = None

  def setup(self) -> None:
    assert self._perfetto_pid is None
    for p in self.browser_platform.processes():
      if p["name"] == "perfetto":
        logging.warning("PERFETTO: killing existing session pid: %s", p["pid"])
        self.browser_platform.terminate(p["pid"])
    self._setup_validate_bin()
    self._setup_push_perfetto_config()
    if self.probe.trace_browser_startup:
      self._start_perfetto()

  def _setup_validate_bin(self) -> None:
    binary = self.perfetto_cmd[0]
    if not self.browser_platform.which(binary):
      raise ValueError(
          f"{repr(binary)} cannot be found on {self.browser_platform}")

  def _setup_push_perfetto_config(self) -> None:
    self.host_platform.write_text(
        self._host_config_file,
        proto_text_format.MessageToString(self.probe.trace_config))
    self.browser_platform.push(self._host_config_file,
                               self.get_browser_config_path())

  @abc.abstractmethod
  def get_browser_config_path(self) -> pth.AnyPath:
    pass

  @abc.abstractmethod
  def get_default_result_path(self) -> pth.AnyPath:
    pass

  @property
  def perfetto_cmd(self) -> TupleCmdArgs:
    return (self.probe.perfetto_bin,)

  def start(self) -> None:
    if self.probe.trace_browser_startup:
      if not self._perfetto_pid:
        raise RuntimeError("Perfetto was not started")
      return
    self._start_perfetto()
    self.browser.performance_mark("probe-perfetto-start")

  def stop(self) -> None:
    self.browser.performance_mark("probe-perfetto-stop")
    logging.info("PERFETTO: stopping")
    if not self._perfetto_pid:
      raise RuntimeError("Perfetto was not started")
    self._stop_perfetto()

  def _start_perfetto(self) -> None:
    logging.info("PERFETTO: starting")
    cmd: TupleCmdArgs = self.perfetto_cmd + (
        "--background",
        "--config",
        self.get_browser_config_path(),
        "--txt",
        "--out",
        self.result_path,
    )
    proc = self.browser_platform.sh(*cmd, capture_output=True)
    if proc.returncode > 0:
      logging.error("perfetto command failed with stderr: %s", proc.stderr)
      raise subprocess.CalledProcessError(proc.returncode, proc.args,
                                          proc.stdout, proc.stderr)

    self._perfetto_pid = NumberParser.positive_int(
        proc.stdout.decode("utf-8").rstrip(), "perfetto pid")
    atexit.register(self._stop_perfetto)

  def _stop_perfetto(self) -> None:
    if not self._perfetto_pid:
      return
    atexit.unregister(self._stop_perfetto)
    # TODO(cbruni): replace with terminate_gracefully
    self.browser_platform.terminate(self._perfetto_pid)
    try:
      for _ in WaitRange(1, timeout=30).wait_with_backoff():
        if not self.browser_platform.process_info(self._perfetto_pid):
          break
    except TimeoutError:
      logging.error("perfetto process did not stop after 30s. "
                    "The trace might be incomplete.")
    self._perfetto_pid = None

  def teardown(self) -> ProbeResult:
    # Copy files:
    browser_result = self.browser_result(file=[self.result_path])
    local_result_file = browser_result.file
    assert local_result_file.is_file(), (
        f"Could not copy perfetto results: {local_result_file}")

    self.host_platform.sh("gzip", local_result_file)
    local_result_file = local_result_file.with_suffix(
        f"{local_result_file.suffix}.gz")

    return LocalProbeResult(trace=(local_result_file,))


class DesktopPerfettoProbeContext(PerfettoProbeContext):

  def __init__(self, probe: PerfettoProbe, run: Run) -> None:
    self._tracebox_proc: subprocess.Popen | None = None
    super().__init__(probe, run)
    self._tracebox_bin = self.probe.tracebox_bin

  @override
  def get_browser_config_path(self) -> pth.AnyPath:
    return self.result_path.with_name("perfetto_config.textproto")

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    return self._run.get_default_probe_result_path(
        self._probe).with_name("perfetto.trace.pb")

  @override
  def setup(self) -> None:
    super().setup()
    self._tracebox_proc = self._setup_tracebox()

  @override
  def _setup_validate_bin(self) -> None:
    if not self.browser_platform.which(self._tracebox_bin):
      self._tracebox_bin = PerfettoToolDownloader(
          "tracebox", platform=self.browser_platform).download()
    super()._setup_validate_bin()

  @override
  def teardown(self) -> ProbeResult:
    self._teardown_tracebox()
    return super().teardown()

  def _setup_tracebox(self) -> subprocess.Popen:
    tracebox_proc = self.browser_platform.popen(self._tracebox_bin, "traced",
                                                "traced_probes")
    atexit.register(self._teardown_tracebox)
    return tracebox_proc

  def _teardown_tracebox(self) -> None:
    if self._tracebox_proc:
      atexit.unregister(self._teardown_tracebox)
      self._tracebox_proc.terminate()
      self._tracebox_proc = None

  @property
  @override
  def perfetto_cmd(self) -> TupleCmdArgs:
    return (self._tracebox_bin, "perfetto")


class AndroidPerfettoProbeContext(PerfettoProbeContext):

  @override
  def get_browser_config_path(self) -> pth.AnyPath:
    return _PERFETTO_CONFIG_REMOTE_DIR_ANDROID / "perfetto_config.textproto"

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    return _PERFETTO_TRACE_REMOTE_DIR_ANDROID / "perfetto.trace.pb"

  @property
  @override
  def browser_platform(self) -> AndroidAdbPlatform:
    browser_platform = super().browser_platform
    assert isinstance(browser_platform, AndroidAdbPlatform)
    return cast(AndroidAdbPlatform, browser_platform)


class ChromeOsPerfettoProbeContext(PerfettoProbeContext):

  @property
  @override
  def browser_platform(self) -> ChromeOsSshPlatform:
    browser_platform = super().browser_platform
    isinstance(browser_platform, ChromeOsSshPlatform)
    return cast(ChromeOsSshPlatform, browser_platform)

  @override
  def get_browser_config_path(self) -> pth.AnyPath:
    return _PERFETTO_REMOTE_DIR_CROS / "perfetto_config.textproto"

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    return _PERFETTO_REMOTE_DIR_CROS / "perfetto.trace.pb"
