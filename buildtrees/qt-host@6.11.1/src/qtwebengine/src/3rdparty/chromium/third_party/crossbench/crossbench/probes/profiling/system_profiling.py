# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
import shlex
from typing import (TYPE_CHECKING, Any, Final, Iterable, Optional, Self,
                    Sequence, cast)

from typing_extensions import override

from crossbench import path as pth
from crossbench import plt
from crossbench.browsers.chromium_based.chromium_based import ChromiumBased
from crossbench.helper import fs_helper
from crossbench.parse import NumberParser, ObjectParser
from crossbench.probes.probe import (Probe, ProbeConfigParser,
                                     ProbeIncompatibleBrowser, ProbeKeyT)
from crossbench.probes.profiling.context.android import AndroidProfilingContext
from crossbench.probes.profiling.context.linux import LinuxProfilingContext
from crossbench.probes.profiling.context.macos import MacOSProfilingContext
from crossbench.probes.profiling.enum import (CallGraphMode, CleanupMode,
                                              TargetMode)
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.probes.profiling.context.base import ProfilingContext
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.run import Run


V8_INTERPRETED_FRAMES_FLAG = "--interpreted-frames-native-stack"

RENDERER_CMD_PATH: Final[pth.LocalPath] = pth.LocalPath(
    __file__).parent / "linux-perf-chrome-renderer-cmd.sh"


def perf_frequency(value: Any) -> str | int:
  if value == "max":
    return "max"
  return NumberParser.positive_int(value, "frequency")


class ProfilingProbe(Probe):
  """
  General-purpose sampling profiling probe.

  Implementation:
  - Uses linux-perf on linux platforms (per browser/renderer process)
  - Uses xctrace on MacOS (currently only system-wide)
  - Uses simpleperf on Android (renderer-only, browser-only, or system-wide)

  For linux-based Chromium browsers it also injects JS stack samples with names
  from V8. For Googlers it additionally can auto-upload symbolized profiles to
  pprof.
  """
  NAME = "profiling"
  RESULT_LOCATION = ResultLocation.BROWSER
  IS_GENERAL_PURPOSE = True

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "js",
        type=bool,
        default=True,
        help=("Chrome-on-Linux-only: expose JS function names to the native "
              "profiler"))
    parser.add_argument(
        "browser_process",
        type=bool,
        default=False,
        help=("Chrome-on-Linux-only: also profile the browser process, "
              "(as opposed to only renderer processes)"))
    parser.add_argument(
        "spare_renderer_process",
        type=bool,
        default=False,
        help=("Chrome-only: Enable/Disable spare renderer processes via \n"
              "--enable-/--disable-features=SpareRendererForSitePerProcess.\n"
              "Spare renderers are disabled by default when profiling "
              "for fewer uninteresting processes."))
    parser.add_argument(
        "v8_interpreted_frames",
        type=bool,
        default=True,
        help=(
            f"Chrome-only: Sets the {V8_INTERPRETED_FRAMES_FLAG} flag for "
            "V8, which exposes interpreted frames as native frames. "
            "Note that this comes at an additional performance and memory cost."
        ))
    parser.add_argument(
        "pprof",
        type=bool,
        default=True,
        help="linux-only: process collected samples with pprof.")
    parser.add_argument(
        "cleanup",
        type=CleanupMode,
        default=CleanupMode.AUTO,
        help="Automatically clean up any temp files "
        "(perf.data.jitted and temporary .so files on linux "
        "cleaned up automatically if pprof is set to True)")
    # Android/simpleperf-specific arguments.
    parser.add_argument(
        "target",
        type=TargetMode,
        default=TargetMode.BROWSER_APP_ONLY,
        help=("Chrome-on-Android/Chrome-on-Mac: "
              "Profile either Renderer main/process only, "
              "or all processes of the Browser App, or system-wide. "
              "If Renderer main/process profiling is selected, "
              "profiling begins **after** browser has started "
              "and the benchmark story has been setup."))
    parser.add_argument(
        "pin_renderer_main_core",
        type=NumberParser.positive_zero_int,
        default=None,
        help=("Chrome-on-Android-only: "
              "Whether to pin the renderer main thread to a given core"))
    parser.add_argument(
        "call_graph_mode",
        aliases=("call-graph",),
        type=CallGraphMode,
        default=CallGraphMode.FRAME_POINTER,
        help=("Android/Linux-only: Specify whether to record a call graph, "
              "and, if yes, which kind of stack unwinding to run."))
    # Advanced Android/simpleperf/linux-perf-specific arguments.
    # Generally, the defaults should suffice.
    parser.add_argument(
        "frequency",
        aliases=("freq",),
        type=perf_frequency,
        default=None,
        help=("Android/Linux-only: Event sampling frequency "
              "(record at most `frequency` samples every second). "
              "Please refer to '--freq' in the simpleperf/linux perf "
              "documentation for more details."))
    parser.add_argument(
        "count",
        type=NumberParser.positive_int,
        default=None,
        help=("Android/Linux-only: Event sampling period "
              "(record one sample every `count` events). "
              "Please refer to '--count' in the simpleperf/linux perf "
              "documentation for more details."))
    parser.add_argument(
        "clockid",
        type=ObjectParser.non_empty_str,
        default=None,
        help=("Android/Linux-only: Defines the clock id used in perf events. "
              "Please refer to '--clockid' in the simpleperf/linux perf "
              "documentation for more details. Defaults to 'mono'."))
    parser.add_argument(
        "cpu",
        type=NumberParser.positive_zero_int,
        is_list=True,
        default=tuple(),
        help=("Android/Linux-only: Sample only on the selected cpus, "
              "specified as a list of 0-indexed cpu indices. "
              "Please refer to '--cpu' in the simpleperf/linux-perf "
              "documentation for more details."))
    parser.add_argument(
        "events",
        type=str,
        is_list=True,
        default=tuple(),
        help=("Android/Linux-only-only: Events to record. "
              "Please refer to the '-e' simpleperf/linux-perf "
              "documentation for more details."))
    parser.add_argument(
        "grouped_events",
        type=str,
        is_list=True,
        default=tuple(),
        help=("Android-only: Events to record as a single group. "
              "These events are monitored as a group, "
              "and scheduled in and out together. "
              "Please refer to simpleperf documentation for `--group` "
              "for more details."))
    parser.add_argument(
        "add_counters",
        type=str,
        is_list=True,
        default=tuple(),
        help=("Android-only: Add additional event counts in samples. NOTE: If "
              "`add_counter` is used, `--no-inherit` is implicitly set, since "
              "this is required by simpleperf. Please refer to simpleperf "
              "documentation for `--add-counter` and `--no-inherit` for more "
              "details."))
    return parser

  def __init__(
      self,
      js: bool = True,
      v8_interpreted_frames: bool = True,
      pprof: bool = True,
      cleanup: CleanupMode = CleanupMode.AUTO,
      browser_process: bool = False,
      spare_renderer_process: bool = False,
      target: TargetMode = TargetMode.BROWSER_APP_ONLY,
      pin_renderer_main_core: Optional[int] = None,
      call_graph_mode: CallGraphMode = CallGraphMode.FRAME_POINTER,
      frequency: Optional[int | str] = None,
      clockid: Optional[str] = None,
      count: Optional[int] = None,
      cpu: Sequence[int] = (),
      events: Sequence[str] = (),
      grouped_events: Sequence[str] = (),
      add_counters: Sequence[str] = ()
  ) -> None:
    super().__init__()
    self._sample_js: bool = js
    self._sample_browser_process: bool = browser_process
    self._spare_renderer_process: bool = spare_renderer_process
    self._run_pprof: bool = pprof
    self._cleanup_mode = cleanup
    self._expose_v8_interpreted_frames: bool = v8_interpreted_frames
    if v8_interpreted_frames:
      assert js, "Cannot expose V8 interpreted frames without js profiling."
    self._target: TargetMode = target
    self._pin_renderer_main_core: int | None = pin_renderer_main_core
    self._call_graph_mode: CallGraphMode = call_graph_mode
    self._start_profiling_after_setup: bool = target in (
        TargetMode.RENDERER_MAIN_ONLY,
        TargetMode.RENDERER_PROCESS_ONLY) or pin_renderer_main_core is not None
    self._frequency: int | str | None = frequency
    self._clockid: str | None = clockid
    self._count: int | None = count
    self._cpu: tuple[int, ...] = tuple(cpu)
    self._events: tuple[str, ...] = tuple(events)
    self._grouped_events: tuple[str, ...] = tuple(grouped_events)
    self._add_counters: tuple[str, ...] = tuple(add_counters)

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (
        ("js", self._sample_js),
        ("v8_interpreted_frames", self._expose_v8_interpreted_frames),
        ("pprof", self._run_pprof),
        ("cleanup", self._cleanup_mode),
        ("browser_process", self._sample_browser_process),
        ("spare_renderer_process", self._spare_renderer_process),
        ("target", str(self._target)),
        ("pin_renderer_main_core", self._pin_renderer_main_core),
        ("call_graph_mode", str(self._call_graph_mode)),
        ("start_profiling_after_setup", self._start_profiling_after_setup),
        ("frequency", self._frequency),
        ("count", self._count),
        ("cpu", self._cpu),
        ("events", self._events),
        ("grouped_events", self._grouped_events),
        ("add_counters", self._add_counters),
    )

  @property
  def sample_js(self) -> bool:
    return self._sample_js

  @property
  def sample_browser_process(self) -> bool:
    return self._sample_browser_process

  @property
  def run_pprof(self) -> bool:
    return self._run_pprof

  @property
  def cleanup_mode(self) -> CleanupMode:
    return self._cleanup_mode

  @property
  def target(self) -> TargetMode:
    return self._target

  @property
  def pin_renderer_main_core(self) -> Optional[int]:
    return self._pin_renderer_main_core

  @property
  def call_graph_mode(self) -> CallGraphMode:
    return self._call_graph_mode

  @property
  def start_profiling_after_setup(self) -> bool:
    return self._start_profiling_after_setup

  @property
  def frequency(self) -> Optional[int | str]:
    return self._frequency

  @property
  def clockid(self) -> Optional[str]:
    return self._clockid

  @property
  def count(self) -> Optional[int]:
    return self._count

  @property
  def cpu(self) -> tuple[int, ...]:
    return self._cpu

  @property
  def events(self) -> tuple[str, ...]:
    return self._events

  @property
  def grouped_events(self) -> tuple[str, ...]:
    return self._grouped_events

  @property
  def add_counters(self) -> tuple[str, ...]:
    return self._add_counters

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    browser_platform = browser.platform
    if browser_platform.is_linux:
      self._validate_linux(env, browser)
    elif browser_platform.is_macos:
      self._validate_macos(env, browser)
    elif browser_platform.is_android:
      self._validate_android(env, browser)
    else:
      raise ProbeIncompatibleBrowser(self, browser)
    if browser.attributes().is_chromium_based:
      chromium = cast(ChromiumBased, browser)
      self._validate_chromium_based(chromium)
    if self.run_pprof:
      self._validate_pprof(env, browser)
    # Check that certain Android-only options are
    # not provided by on other platforms.
    if not browser_platform.is_android and not browser_platform.is_linux:
      self._validate_perf_settings(browser)
    if not browser_platform.is_android:
      self._validate_non_android_perf_settings(browser)

  def _validate_chromium_based(self, browser: ChromiumBased) -> None:
    if self._start_profiling_after_setup:
      self._validate_benchmarking_extension_version(browser)

  def _validate_perf_settings(self, browser) -> None:
    unsupported_settings = (
        ("frequency", self._frequency),
        ("count", self._count),
        ("cpu", self._cpu),
        ("events", self._events),
    )
    self._validate_unsupported_settings(browser, unsupported_settings,
                                        "Android and Linux")

  def _validate_non_android_perf_settings(self, browser) -> None:
    unsupported_settings = (
        ("grouped_events", self._grouped_events),
        ("add_counters", self._add_counters),
    )
    self._validate_unsupported_settings(browser, unsupported_settings,
                                        "Android")

  def _validate_unsupported_settings(self, browser,
                                     unsupported_settings: Iterable[tuple[str,
                                                                          Any]],
                                     platforms) -> None:
    for name, value in unsupported_settings:
      if value:
        raise ProbeIncompatibleBrowser(
            self, browser,
            f"{repr(name)} is currently only supported on {platforms}")

  def _validate_linux(self, env: RunnerEnv, browser: Browser) -> None:
    env.check_installed(binaries=["pprof"])
    assert browser.platform.which("perf"), "Please install linux-perf"

  def _validate_macos(self, env: RunnerEnv, browser: Browser) -> None:
    assert browser.platform.which(
        "xctrace"), "Please install Xcode to use xctrace"
    # Only Linux-perf and Android-simpleperf results can be merged
    if env.repetitions > 1:
      env.handle_warning(f"Probe={self.NAME} cannot merge data over multiple "
                         f"repetitions={env.repetitions}.")

    supported_mac_targets = (TargetMode.SYSTEM_WIDE,
                             TargetMode.RENDERER_PROCESS_ONLY)
    assert self._target in supported_mac_targets, (
        f"Unsupported profile target for Mac: {self._target}. "
        f"Should be one of {str(supported_mac_targets)}.")

  def _validate_android(self, env: RunnerEnv, browser: Browser) -> None:
    del env
    assert browser.platform.which("simpleperf"), "simpleperf is not available"

  def _validate_benchmarking_extension_version(self,
                                               browser: ChromiumBased) -> None:
    assert (
        browser.attributes().is_chromium_based and
        browser.version.major >= 124), (
            "For RENDERER_MAIN_ONLY/RENDERER_PROCESS_ONLY profiling, "
            "browser version >= M124 https://crrev.com/c/5374765 is required.")

  def _validate_pprof(self, env: RunnerEnv, browser: Browser) -> None:
    assert self._run_pprof
    host_platform = browser.host_platform
    self._run_pprof = host_platform.which("gcert") is not None
    if not self.run_pprof:
      logging.warning(
          "Disabled automatic pprof uploading for non-googler machine.")
      return
    if browser.platform.is_macos:
      # Converting xctrace to pprof is not supported on macos
      return
    try:
      if gcertstatus := host_platform.which("gcertstatus"):
        host_platform.sh(gcertstatus)
        return
      env.handle_warning("Could not find gcertstatus")
    except plt.SubprocessError:
      env.handle_warning("Please run gcert for generating pprof results")

  @override
  def attach(self, browser: Browser) -> None:
    super().attach(browser)
    if browser.platform.is_linux or browser.platform.is_android:
      assert browser.attributes().is_chromium_based, (
          f"Expected Chromium-based browser, found {type(browser)}.")
    if browser.attributes().is_chromium_based:
      chromium = cast(ChromiumBased, browser)
      self._attach_chromium(chromium)

  def _attach_chromium(self, browser: ChromiumBased) -> None:
    if not self._spare_renderer_process:
      browser.features.disable("SpareRendererForSitePerProcess")
    if self._start_profiling_after_setup:
      browser.flags.enable_benchmarking_api()
    if self._sample_js:
      if browser.platform.is_linux:
        browser.js_flags.set("--perf-prof")
      if self._expose_v8_interpreted_frames:
        browser.js_flags.set(V8_INTERPRETED_FRAMES_FLAG)
    if browser.platform.is_linux and browser.platform.is_local:
      self._set_renderer_cmd_prefix(browser)
    # Disable sandbox to write profiling data
    browser.flags.set("--no-sandbox")

  def _set_renderer_cmd_prefix(self, browser) -> None:
    assert not browser.platform.is_remote, (
        "Copying renderer command prefix to remote platform is "
        "not implemented yet")
    assert RENDERER_CMD_PATH.is_file(), f"Didn't find {RENDERER_CMD_PATH}"
    cmd_prefix = [str(RENDERER_CMD_PATH), f"--perf-data-dir={self.NAME}"]
    if freq := self.frequency:
      cmd_prefix.append(f"--perf-freq={freq}")
    if count := self.count:
      cmd_prefix.append(f"--perf-count={count}")
    if self.call_graph_mode != CallGraphMode.FRAME_POINTER:
      cmd_prefix.append(f"--perf-call-graph={self.call_graph_mode}")
    if clockid := self.clockid:
      cmd_prefix.append(f"--perf-clockid={clockid}")
    custom_perf_args = []
    if cpu := self.cpu:
      cpu_str = ",".join(map(str, cpu))
      custom_perf_args.append(f"--cpu={cpu_str}")
    if events := self.events:
      events_str = ",".join(events)
      custom_perf_args.append(f"--event={events_str}")
    if custom_perf_args:
      cmd_prefix.append(f"--perf-args={shlex.join(custom_perf_args)}")
    browser.flags["--renderer-cmd-prefix"] = shlex.join(cmd_prefix)

  @override
  def log_run_result(self, run: Run) -> None:
    self._log_results([run])

  @override
  def log_browsers_result(self, group: BrowsersRunGroup) -> None:
    self._log_results(group.runs)

  def _log_results(self, runs: Iterable[Run]) -> None:
    filtered_runs = list(run for run in runs if self in run.results)
    if not filtered_runs:
      return
    logging.info("-" * 80)
    logging.critical("Profiling results:")
    self._log_results_overview(filtered_runs)
    logging.info("- " * 40)
    for i, run in enumerate(filtered_runs):
      self._log_run_result_summary(run, i)

  def _log_results_overview(self, filtered_runs) -> None:
    if len(filtered_runs) <= 1:
      return
    if any(run.browser_platform.is_macos for run in filtered_runs):
      logging.info("  *.trace:     'open $FILE'")
    if any(run.browser_platform.is_linux or run.browser_platform.is_android
           for run in filtered_runs):
      logging.info("  *.perf.data: 'perf report -i $FILE'")

  def _log_run_result_summary(self, run: Run, i: int) -> None:
    if self not in run.results:
      return
    urls = run.results[self].url_list
    perf_files = run.results[self].file_list
    if not urls and not perf_files:
      return
    logging.info("Run %d: %s", i + 1, run.name)
    if urls:
      logging.critical("    %s", urls[-1])
    if not perf_files:
      return
    largest_perf_file = perf_files[-1]
    logging.critical("    %s [%s]", largest_perf_file,
                     fs_helper.get_file_size(largest_perf_file))
    if len(perf_files) <= 1:
      return
    glob = "*.perf.data"
    if run.browser_platform.is_macos:
      glob = "*.trace"
    logging.info("    %s/%s: %d more files", largest_perf_file.parent, glob,
                 len(perf_files))

  def get_context(self, run: Run) -> ProfilingContext:
    if run.browser_platform.is_linux:
      return LinuxProfilingContext(self, run)
    if run.browser_platform.is_macos:
      return MacOSProfilingContext(self, run)
    if run.browser_platform.is_android:
      return AndroidProfilingContext(self, run)
    raise NotImplementedError("Invalid platform")
