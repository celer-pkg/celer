# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import json
import logging
import multiprocessing
import time
from typing import TYPE_CHECKING, Optional

from typing_extensions import override

from crossbench import plt
from crossbench.browsers.chromium.version import ChromiumVersion
from crossbench.cli import ui
from crossbench.helper import fs_helper
from crossbench.probes.profiling.context.base import PosixProfilingContext
from crossbench.probes.profiling.enum import CleanupMode

if TYPE_CHECKING:
  import crossbench.path as pth
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run

V8_PERF_PROF_PATH_FLAG_MIN_VERSION = ChromiumVersion((118, 0, 5993, 48))
PERF_DATA_PATTERN = "*.perf.data"
JIT_DUMP_PATTERN = "jit-*.dump"


class LinuxProfilingContext(PosixProfilingContext):
  TEMP_FILE_PATTERNS = (
      "*.perf.data.jitted",
      "jitted-*.so",
      JIT_DUMP_PATTERN,
  )

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    result_dir = super().get_default_result_path()
    self.browser_platform.mkdir(result_dir)
    return result_dir

  @property
  def has_perf_prof_path(self) -> bool:
    return self.browser.version > V8_PERF_PROF_PATH_FLAG_MIN_VERSION

  @override
  def setup(self) -> None:
    self.setup_v8_log_path()
    if self.has_perf_prof_path:
      self.session.extra_js_flags["--perf-prof-path"] = str(self.result_path)

  def start(self) -> None:
    if not self.probe.sample_browser_process:
      return
    if self.run.browser.pid is None:
      logging.warning("Cannot sample browser process")
      return
    perf_data_file: pth.AnyPath = self.result_path / "browser.perf.data"
    # TODO: not fully working yet
    self._profiling_process = self.browser_platform.popen(
        "perf", "record", f"--call-graph={self.probe.call_graph_mode or 'fp'}",
        f"--freq={self.probe.frequency or 'max'}",
        f"--clockid={self.probe.clockid or 'mono'}",
        f"--output={perf_data_file}", f"--pid={self.run.browser.pid}")
    if self._profiling_process.poll():
      raise ValueError("Could not start linux profiler")
    atexit.register(self.stop_process)

  def stop(self) -> None:
    self.stop_process()

  def stop_process(self) -> None:
    if self._profiling_process:
      self.browser_platform.terminate_gracefully(self._profiling_process)
      self._profiling_process = None

  def teardown(self) -> ProbeResult:
    # Waiting for linux-perf to flush all perf data
    if self.probe.sample_browser_process:
      logging.debug("Waiting for browser process to stop")
      time.sleep(3)
    if self.probe.sample_browser_process:
      logging.info("Browser process did not stop after 3s. "
                   "You might get partial profiles")
    time.sleep(2)

    perf_files: list[pth.AnyPath] = fs_helper.sort_by_file_size(
        list(self.browser_platform.glob(self.result_path, PERF_DATA_PATTERN)),
        self.browser_platform)
    raw_perf_files = perf_files
    urls: list[str] = []
    try:
      if self.probe.sample_js:
        perf_files = self._inject_v8_symbols(self.run, perf_files)
      if self.probe.run_pprof:
        urls = self._export_to_pprof(self.run, perf_files)
    finally:
      self._clean_up_temp_files(self.run)
    if self.probe.run_pprof:
      logging.debug("Profiling results: %s", urls)
      return self.browser_result(url=urls, file=raw_perf_files)
    if self.browser_platform.which("pprof"):
      logging.info("Run pprof over all (or single) perf data files "
                   "for interactive analysis:")
      logging.info("   pprof --http=localhost:1984 %s",
                   " ".join(map(str, perf_files)))
    return self.browser_result(trace=perf_files)

  def _inject_v8_symbols(self, run: Run,
                         perf_files: list[pth.AnyPath]) -> list[pth.AnyPath]:
    with run.actions(
        f"Probe {self.probe.name}: "
        f"Injecting V8 symbols into {len(perf_files)} profiles",
        verbose=True), ui.spinner():
      # Filter out empty files
      perf_files = [
          file for file in perf_files
          if self.browser_platform.file_size(file) > 0
      ]
      if self.browser_platform.is_remote:
        # Use loop, as we cannot easily serialize the remote platform.
        perf_jitted_files = [
            linux_perf_probe_inject_v8_symbols(file, self.browser_platform)
            for file in perf_files
        ]
      else:
        assert self.browser_platform == plt.PLATFORM
        with multiprocessing.Pool() as pool:
          perf_jitted_files = list(
              pool.imap(linux_perf_probe_inject_v8_symbols, perf_files))
      return [file for file in perf_jitted_files if file is not None]

  def _export_to_pprof(self, run: Run,
                       perf_files: list[pth.AnyPath]) -> list[str]:
    assert self.probe.run_pprof
    run_details_json = json.dumps(run.get_browser_details_json())
    with run.actions(
        f"Probe {self.probe.name}: "
        f"exporting {len(perf_files)} profiles to pprof (slow)",
        verbose=True), ui.spinner():
      self.browser_platform.sh(
          "gcertstatus >&/dev/null || "
          "(echo 'Authenticating with gcert:'; gcert)",
          shell=True)
      size = len(perf_files)
      items = zip(perf_files, [run_details_json] * size)
      urls: list[str] = []
      if self.browser_platform.is_remote:
        # Use loop, as we cannot easily serialize the remote platform.
        for perf_data_file, run_details in items:
          url = linux_perf_probe_pprof(perf_data_file, run_details,
                                       self.browser_platform)
          if url:
            urls.append(url)
      else:
        assert self.browser_platform == plt.PLATFORM
        with multiprocessing.Pool() as pool:
          urls = [
              url for url in pool.starmap(linux_perf_probe_pprof, items) if url
          ]
      try:
        if perf_files:
          # TODO: Add "combined" profile again
          pass
      except Exception as e:  # pylint: disable=broad-except
        logging.debug("Failed to run pprof: %s", e)
      return urls

  def _clean_up_temp_files(self, run: Run) -> None:
    if self.probe.cleanup_mode == CleanupMode.NEVER:
      logging.debug("%s: skipping cleanup", self.probe)
      return
    if self.probe.cleanup_mode == CleanupMode.AUTO:
      if not self.probe.run_pprof:
        logging.debug("%s: skipping auto cleanup without pprof upload",
                      self.probe)
        return
    for pattern in self.TEMP_FILE_PATTERNS:
      for file in run.out_dir.glob(pattern):
        file.unlink()


def prepare_linux_perf_env(platform: plt.Platform,
                           cwd: pth.AnyPath) -> dict[str, str]:
  env: dict[str, str] = dict(platform.environ)
  env["JITDUMPDIR"] = str(platform.absolute(cwd))
  return env


KB = 1024


def linux_perf_probe_inject_v8_symbols(
    perf_data_file: pth.AnyPath,
    platform: Optional[plt.Platform] = None) -> Optional[pth.AnyPath]:
  platform = platform or plt.PLATFORM
  assert platform.is_file(perf_data_file)
  output_file = perf_data_file.with_suffix(".data.jitted")
  assert not platform.exists(output_file)
  env = prepare_linux_perf_env(platform, perf_data_file.parent)
  try:
    # TODO: use remote chdir
    platform.sh(
        "perf",
        "inject",
        "--jit",
        f"--input={perf_data_file}",
        f"--output={output_file}",
        env=env)
  except plt.SubprocessError as e:
    if platform.file_size(perf_data_file) > 200 * KB:
      logging.warning("Failed processing: %s\n%s", perf_data_file, e)
    else:
      # TODO: investigate why almost all small perf.data files fail
      logging.debug("Failed processing small profile (likely empty): %s\n%s",
                    perf_data_file, e)
  if not platform.exists(output_file):
    return None
  return output_file


def linux_perf_probe_pprof(
    perf_data_file: pth.AnyPath,
    run_details: str,
    platform: Optional[plt.Platform] = None) -> Optional[str]:
  platform = platform or plt.PLATFORM
  size = fs_helper.get_file_size(perf_data_file, platform=platform)
  env = prepare_linux_perf_env(platform, perf_data_file.parent)
  url = ""
  try:
    url = platform.sh_stdout(
        "pprof",
        "-flame",
        f"-add_comment={run_details}",
        perf_data_file,
        env=env,
    ).strip()
  except plt.SubprocessError as e:
    # Occasionally small .jitted files fail, likely due perf inject silently
    # failing?
    raw_perf_data_file = perf_data_file.with_suffix("")
    if (perf_data_file.suffix == ".jitted" and
        platform.exists(raw_perf_data_file)):
      logging.debug(
          "pprof best-effort: falling back to standard perf data "
          "without js symbols: %s \n"
          "Got failures for %s: %s", raw_perf_data_file, perf_data_file.name, e)
      try:
        perf_data_file = raw_perf_data_file
        url = platform.sh_stdout(
            "pprof",
            "-flame",
            f"-add_comment={run_details}",
            raw_perf_data_file,
        ).strip()
      except plt.SubprocessError as e2:
        logging.debug("pprof -flame failed: %s", e2)
    if not url:
      logging.warning("Failed processing: %s\n%s", perf_data_file, e)
      return None
  if perf_data_file.suffix == ".jitted":
    logging.info("PPROF (with js-symbols):")
  else:
    logging.info("PPROF (no js-symbols):")
  logging.info("  linux-perf:   %s [%s]", perf_data_file.name, size)
  logging.info("  pprof result: %s", url)
  return url
