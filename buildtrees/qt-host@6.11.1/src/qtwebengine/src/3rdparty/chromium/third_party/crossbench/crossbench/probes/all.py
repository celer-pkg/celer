# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from crossbench.probes.android_logcat import LogcatAndroidProbe
from crossbench.probes.chrome_histograms import ChromeHistogramsProbe
from crossbench.probes.chromium_pgo import ChromiumPgoProbe
from crossbench.probes.debugger import DebuggerProbe
from crossbench.probes.downloads import DownloadsProbe
from crossbench.probes.dtrace import DTraceProbe
from crossbench.probes.dump_html import DumpHtmlProbe
from crossbench.probes.embedder import WebviewEmbedderProbe
from crossbench.probes.frequency import FrequencyProbe
from crossbench.probes.helper import INTERNAL_NAME_PREFIX
from crossbench.probes.internal.browser.driver_log import BrowserDriverLogProbe
from crossbench.probes.internal.durations import DurationsProbe
from crossbench.probes.internal.errors import ErrorsProbe
from crossbench.probes.internal.log import LogProbe
from crossbench.probes.internal.summary import ResultsSummaryProbe
from crossbench.probes.internal.system_details import SystemDetailsProbe
from crossbench.probes.js import JSProbe
from crossbench.probes.json import JsonResultProbe
from crossbench.probes.meminfo import MeminfoProbe
from crossbench.probes.perfetto.perfetto import PerfettoProbe
from crossbench.probes.perfetto.trace_processor.trace_processor import \
    TraceProcessorProbe
from crossbench.probes.perfetto.tracing import TracingProbe
from crossbench.probes.performance_entries import PerformanceEntriesProbe
from crossbench.probes.polling import PollingShellProbe
from crossbench.probes.power_sampler import PowerSamplerProbe
from crossbench.probes.powermetrics import PowerMetricsProbe
from crossbench.probes.probe import Probe
from crossbench.probes.profiling.browser_profiling import BrowserProfilingProbe
from crossbench.probes.profiling.system_profiling import ProfilingProbe
from crossbench.probes.screenshot import ScreenshotProbe
from crossbench.probes.shell import ShellProbe
from crossbench.probes.system_stats import SystemStatsProbe
from crossbench.probes.thermal_monitor import ThermalMonitorProbe
from crossbench.probes.v8.builtins_pgo import V8BuiltinsPGOProbe
from crossbench.probes.v8.log import V8LogProbe
from crossbench.probes.v8.rcs import V8RCSProbe
from crossbench.probes.v8.turbolizer import V8TurbolizerProbe
from crossbench.probes.video import VideoProbe
from crossbench.probes.video_acceleration import VideoAccelerationProbe
from crossbench.probes.web_page_replay.recorder import WebPageReplayProbe

if TYPE_CHECKING:
  from crossbench.probes.internal.base import InternalProbe
  InternalProbeTuple = tuple[Type[InternalProbe], ...]

ABSTRACT_PROBES: tuple[Type[Probe], ...] = (Probe, JsonResultProbe)

# Probes that are not user-configurable
# Order matters, not alpha-sorted:
# Internal probes depend on each other, for instance the ResultsSummaryProbe
# reads the values of the other internal probes and thus needs to be the first
# to be initialized and the last to be teared down to write out a summary
# result of all the other probes.

# Internal probes that are always installed and are non configurable.
NON_CONFIGURABLE_INTERNAL_PROBES: InternalProbeTuple = (
    ResultsSummaryProbe,
    DurationsProbe,
    ErrorsProbe,
    LogProbe,
    SystemDetailsProbe,
)
# Internal probes that are configurable by command line flags but always
# installed.
CONFIGURABLE_INTERNAL_PROBES: InternalProbeTuple = (ThermalMonitorProbe,)
DEFAULT_INTERNAL_PROBES: InternalProbeTuple = (
    NON_CONFIGURABLE_INTERNAL_PROBES + CONFIGURABLE_INTERNAL_PROBES)

# Internal probes that are configurable and only optionally installed.
OPTIONAL_INTERNAL_PROBES: InternalProbeTuple = (BrowserDriverLogProbe,)

INTERNAL_PROBES: InternalProbeTuple = (
    DEFAULT_INTERNAL_PROBES + OPTIONAL_INTERNAL_PROBES)

# ResultsSummaryProbe should always be processed last, and thus must be the
# first probe to be added to any browser.
assert DEFAULT_INTERNAL_PROBES[0] == ResultsSummaryProbe
assert DEFAULT_INTERNAL_PROBES[1] == DurationsProbe


# Probes that can be used on arbitrary stories and may be user configurable.
GENERAL_PURPOSE_PROBES: tuple[Type[Probe], ...] = (
    BrowserProfilingProbe,
    ChromeHistogramsProbe,
    ChromiumPgoProbe,
    DebuggerProbe,
    DownloadsProbe,
    DTraceProbe,
    DumpHtmlProbe,
    WebviewEmbedderProbe,
    FrequencyProbe,
    JSProbe,
    LogcatAndroidProbe,
    MeminfoProbe,
    PerfettoProbe,
    PerformanceEntriesProbe,
    PollingShellProbe,
    PowerMetricsProbe,
    PowerSamplerProbe,
    ProfilingProbe,
    ScreenshotProbe,
    ShellProbe,
    SystemStatsProbe,
    TraceProcessorProbe,
    TracingProbe,
    V8BuiltinsPGOProbe,
    V8LogProbe,
    V8RCSProbe,
    V8TurbolizerProbe,
    VideoProbe,
    VideoAccelerationProbe,
    WebPageReplayProbe,
)

for probe_cls in GENERAL_PURPOSE_PROBES:
  assert probe_cls.IS_GENERAL_PURPOSE, (
      f"Probe {probe_cls} should be marked for GENERAL_PURPOSE")
  assert probe_cls.NAME
  assert not probe_cls.NAME.startswith(INTERNAL_NAME_PREFIX), (
      f"General purpose {probe_cls}.NAME cannot start with 'cb.'")

for probe_cls in DEFAULT_INTERNAL_PROBES:
  assert not probe_cls.IS_GENERAL_PURPOSE, (
      f"Internal Probe {probe_cls} should not marked for GENERAL_PURPOSE")
  assert probe_cls.NAME
  assert probe_cls.NAME.startswith(INTERNAL_NAME_PREFIX), (
      f"Internal {probe_cls}.NAME must start with 'cb.'")
