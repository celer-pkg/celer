# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import enum
from typing import (TYPE_CHECKING, FrozenSet, Optional, Self, Sequence, Set,
                    Type)

from typing_extensions import override

import crossbench.probes.perfetto.traceconv as cb_traceconv
from crossbench import path as pth
from crossbench.config import ConfigEnum
from crossbench.helper.path_finder import TraceconvFinder
from crossbench.parse import NumberParser, ObjectParser
from crossbench.probes.chromium_probe import ChromiumProbe
from crossbench.probes.probe import ProbeConfigParser, ProbeContext, ProbeKeyT
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.probes.results import ProbeResult

# TODO: go over these again and clean the categories.
MINIMAL_CONFIG: FrozenSet[str] = frozenset((
    "blink.user_timing",
    "toplevel",
    "v8",
    "v8.execute",
))
DEVTOOLS_TRACE_CONFIG: FrozenSet[str] = frozenset((
    "blink.console",
    "blink.user_timing",
    "devtools.timeline",
    "disabled-by-default-devtools.screenshot",
    "disabled-by-default-devtools.timeline",
    "disabled-by-default-devtools.timeline.frame",
    "disabled-by-default-devtools.timeline.layers",
    "disabled-by-default-devtools.timeline.picture",
    "disabled-by-default-devtools.timeline.stack",
    "disabled-by-default-lighthouse",
    "disabled-by-default-v8.compile",
    "disabled-by-default-v8.cpu_profiler",
    "disabled-by-default-v8.cpu_profiler.hires"
    "latencyInfo",
    "toplevel",
    "v8.execute",
))
V8_TRACE_CONFIG: FrozenSet[str] = frozenset((
    "blink",
    "blink.user_timing",
    "browser",
    "cc",
    "disabled-by-default-ipc.flow",
    "disabled-by-default-power",
    "disabled-by-default-v8.compile",
    "disabled-by-default-v8.cpu_profiler",
    "disabled-by-default-v8.cpu_profiler.hires",
    "disabled-by-default-v8.gc",
    "disabled-by-default-v8.inspector",
    "disabled-by-default-v8.runtime",
    "disabled-by-default-v8.runtime_stats",
    "disabled-by-default-v8.runtime_stats_sampling",
    "disabled-by-default-v8.stack_trace",
    "disabled-by-default-v8.turbofan",
    "disabled-by-default-v8.wasm.detailed",
    "disabled-by-default-v8.wasm.turbofan",
    "gpu",
    "io",
    "ipc",
    "latency",
    "latencyInfo",
    "loading",
    "log",
    "mojom",
    "navigation",
    "net",
    "netlog",
    "toplevel",
    "toplevel.flow",
    "v8",
    "v8.execute",
    "wayland",
))
V8_GC_STATS_TRACE_CONFIG: FrozenSet[str] = V8_TRACE_CONFIG | frozenset(
    ("disabled-by-default-v8.gc_stats",))

TRACE_PRESETS: dict[str, frozenset[str]] = {
    "empty": frozenset(),
    "minimal": MINIMAL_CONFIG,
    "devtools": DEVTOOLS_TRACE_CONFIG,
    "v8": V8_TRACE_CONFIG,
    "v8-gc-stats": V8_GC_STATS_TRACE_CONFIG,
}


@enum.unique
class RecordMode(ConfigEnum):
  CONTINUOUSLY = ("record-continuously",
                  "Record until the trace buffer is full.")
  UNTIL_FULL = ("record-until-full", "Record until the user ends the trace. "
                "The trace buffer is a fixed size and we use it as "
                "a ring buffer during recording.")
  AS_MUCH_AS_POSSIBLE = ("record-as-much-as-possible",
                         "Record until the trace buffer is full, "
                         "but with a huge buffer size.")
  TRACE_TO_CONSOLE = ("trace-to-console",
                      "Echo to console. Events are discarded.")


@enum.unique
class RecordFormat(ConfigEnum):
  JSON = ("json", "Old about://tracing compatible file format.")
  PROTO = ("proto", "New https://ui.perfetto.dev/ compatible format")


def parse_trace_config_file_path(value: str) -> pth.LocalPath:
  data = ObjectParser.json_file(value)
  if "trace_config" not in data:
    raise argparse.ArgumentTypeError("Missing 'trace_config' property.")
  NumberParser.positive_int(
      data.get("startup_duration", "0"), "for 'startup_duration'")
  if "result_file" in data:
    raise argparse.ArgumentTypeError(
        "Explicit 'result_file' is not allowed with crossbench. "
        "--probe=tracing sets a results location automatically.")
  config = data["trace_config"]
  if "included_categories" not in config and (
      "excluded_categories" not in config) and ("memory_dump_config"
                                                not in config):
    raise argparse.ArgumentTypeError(
        "Empty trace config: no trace categories or memory dumps configured.")
  RecordMode.parse(config.get("record_mode", RecordMode.CONTINUOUSLY))
  config_file_path = pth.LocalPath(value)
  return config_file_path.absolute()


ANDROID_TRACE_CONFIG_PATH = pth.AnyPosixPath(
    "/data/local/chrome-trace-config.json")


class TracingProbe(ChromiumProbe):
  """
  Chromium-only Probe to collect tracing / perfetto data that can be used by
  chrome://tracing or https://ui.perfetto.dev/.

  Configuration:
  Currently you can configure the tracing probe in three different ways:
  - preset:       Using a common preset, by default set to "minimal",
  - categories:   Add more categories to the current selected preset,
  - trace_config: Use a predefined trace config file that overrides the two
                  previous options.
  """
  NAME = "tracing"
  RESULT_LOCATION = ResultLocation.BROWSER
  CHROMIUM_FLAGS = ("--enable-perfetto",)

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "preset",
        type=str,
        default="minimal",
        choices=TRACE_PRESETS.keys(),
        help=("Use predefined trace categories, "
              f"see source {__file__} for more details. "
              "This is cumulative with the categories option."))
    parser.add_argument(
        "categories",
        is_list=True,
        default=[],
        type=str,
        help=("A list of trace categories to enable.\n"
              "https://bit.ly/chrome-about-tracing\n"
              "This is cumulative with the preset option."))
    parser.add_argument(
        "trace_config",
        type=parse_trace_config_file_path,
        help=("Sets Chromium's --trace-config-file to the given json config.\n"
              "https://bit.ly/chromium-memory-startup-tracing\n"
              "'trace_config' is incompatible with the preset and categories "
              "option."))
    parser.add_argument(
        "startup_duration",
        default=0,
        type=NumberParser.positive_zero_int,
        help=("Stop recording tracing after a given number of seconds. "
              "Use 0 (default) for unlimited recording time."))
    parser.add_argument(
        "record_mode",
        default=RecordMode.CONTINUOUSLY,
        type=RecordMode,
        help="")
    parser.add_argument(
        "record_format",
        default=RecordFormat.PROTO,
        type=RecordFormat,
        help=("Choose between 'json' or the default 'proto' format. "
              "Perfetto proto output is converted automatically to the "
              "legacy json format."))
    cb_traceconv.add_argument(parser)
    return parser

  def __init__(self,
               preset: Optional[str] = None,
               categories: Optional[Sequence[str]] = None,
               trace_config: Optional[pth.LocalPath] = None,
               startup_duration: int = 0,
               record_mode: RecordMode = RecordMode.CONTINUOUSLY,
               record_format: RecordFormat = RecordFormat.PROTO,
               traceconv: Optional[pth.LocalPath] = None) -> None:
    super().__init__()
    self._trace_config: pth.LocalPath | None = trace_config
    self._categories: Set[str] = set(categories or MINIMAL_CONFIG)
    self._preset: str | None = preset
    if preset:
      self._categories.update(TRACE_PRESETS[preset])
    if self._trace_config:
      if self._categories and self._categories != set(MINIMAL_CONFIG):
        raise argparse.ArgumentTypeError(
            "TracingProbe requires either a list of "
            "trace categories or a trace_config file.")
      self._categories = set()

    self._startup_duration: int = startup_duration
    self._record_mode: RecordMode = record_mode
    self._record_format: RecordFormat = record_format
    self._traceconv: pth.LocalPath | None = traceconv
    if not traceconv and self._record_format == RecordFormat.PROTO:
      self._traceconv = TraceconvFinder(self.host_platform).local_path

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (("preset", self._preset),
                          ("categories", tuple(self._categories)),
                          ("startup_duration", self._startup_duration),
                          ("record_mode", str(self._record_mode)),
                          ("record_format", str(self._record_format)),
                          ("traceconv", str(self._traceconv)))

  @property
  @override
  def result_path_name(self) -> str:
    return f"trace.{self._record_format.value}"  # pylint: disable=no-member

  @property
  def traceconv(self) -> pth.LocalPath | None:
    return self._traceconv

  @property
  def record_format(self) -> RecordFormat:
    return self._record_format

  @property
  def record_mode(self) -> RecordMode:
    return self._record_mode

  @property
  def categories(self) -> Set[str]:
    return set(self._categories)

  @property
  def trace_config_file(self) -> pth.LocalPath | None:
    return self._trace_config

  @property
  def startup_duration(self) -> int:
    return self._startup_duration

  @override
  def attach(self, browser: Browser) -> None:
    assert browser.attributes().is_chromium_based
    flags = browser.flags
    flags.update(self.CHROMIUM_FLAGS)
    # Force proto file so we can convert it to legacy json as well.
    flags["--trace-startup-format"] = str(self._record_format)
    # pylint: disable=no-member
    flags["--trace-startup-duration"] = str(self._startup_duration)
    if self._trace_config:
      # TODO: use ANDROID_TRACE_CONFIG_PATH
      assert not browser.platform.is_android, (
          "Trace config files not supported on android yet")
      flags["--trace-config-file"] = str(self._trace_config.absolute())
    else:
      flags["--trace-startup-record-mode"] = str(self._record_mode)
      assert self._categories, "No trace categories provided."
      flags["--enable-tracing"] = ",".join(self._categories)
    super().attach(browser)

  @override
  def get_context_cls(self) -> Type[TracingProbeContext]:
    return TracingProbeContext


class TracingProbeContext(ProbeContext[TracingProbe]):
  _record_format: RecordFormat

  def setup(self) -> None:
    self.session.extra_flags["--trace-startup-file"] = str(self.result_path)
    self._record_format = self.probe.record_format

  def start(self) -> None:
    pass

  def stop(self) -> None:
    pass

  def teardown(self) -> ProbeResult:
    if self._record_format == RecordFormat.JSON:
      return self.browser_result(json=(self.result_path,))
    # Use intermediate browser result to copy over remote files.
    result = self.browser_result(trace=(self.result_path,))
    trace_file = result.get("proto")
    if legacy_json_file := cb_traceconv.convert_to_json(self.host_platform,
                                                        self.probe.traceconv,
                                                        trace_file):
      return self.local_result(trace=(trace_file,), json=(legacy_json_file,))
    return result
