# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import datetime as dt
import enum
import subprocess
from typing import TYPE_CHECKING, Self, Sequence, Type

from typing_extensions import override

from crossbench.parse import DurationParser
from crossbench.probes.probe import (Probe, ProbeConfigParser, ProbeContext,
                                     ProbeKeyT)
from crossbench.probes.result_location import ResultLocation
from crossbench.str_enum_with_help import StrEnumWithHelp

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.path import AnyPath
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


@enum.unique
class SamplerType(StrEnumWithHelp):
  BATTERY = ("battery", "Battery level")
  CPU_POWER = ("cpu_power",
               "CPU power and per-core frequency and idle residency")
  DISK = ("disk", "Number of read/write ops/bytes")
  GPU_POWER = ("gpu_power",
               "GPU power consumption, frequency and active residency")
  INTERRUPTS = ("interrupts", "Per-core interrupt count")
  NETWORK = ("network", "Number of in/out packets/bytes")
  TASKS = ("tasks", "Per-task stats including CPU usage and wakeups")
  THERMAL = ("thermal", "Thermal pressure state")


class PowerMetricsProbe(Probe):
  """
  Probe to collect data using macOS's powermetrics command-line tool.
  """

  NAME = "powermetrics"
  RESULT_LOCATION = ResultLocation.BROWSER
  SAMPLERS: tuple[SamplerType,
                  ...] = (SamplerType.BATTERY, SamplerType.CPU_POWER,
                          SamplerType.DISK, SamplerType.GPU_POWER,
                          SamplerType.INTERRUPTS, SamplerType.NETWORK,
                          SamplerType.TASKS, SamplerType.THERMAL)

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "sampling_interval",
        type=DurationParser.positive_duration,
        default=1000)
    parser.add_argument(
        "samplers", type=SamplerType, default=cls.SAMPLERS, is_list=True)
    return parser

  def __init__(self,
               sampling_interval: dt.timedelta = dt.timedelta(),
               samplers: Sequence[SamplerType] = SAMPLERS) -> None:
    super().__init__()
    self._sampling_interval = sampling_interval
    if sampling_interval.total_seconds() < 0:
      raise ValueError(f"Invalid sampling_interval={sampling_interval}")
    self._samplers = tuple(samplers)

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (
        ("sampling_interval", self.sampling_interval.total_seconds()),
        ("samplers", tuple(map(str, self.samplers))),
    )

  @property
  def sampling_interval(self) -> dt.timedelta:
    return self._sampling_interval

  @property
  def samplers(self) -> tuple[SamplerType, ...]:
    return self._samplers

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    self.expect_macos(browser)

  @override
  def get_context_cls(self) -> Type[PowerMetricsProbeContext]:
    return PowerMetricsProbeContext


class PowerMetricsProbeContext(ProbeContext[PowerMetricsProbe]):

  def __init__(self, probe: PowerMetricsProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._power_metrics_process: subprocess.Popen | None = None
    self._output_plist_file: AnyPath = self.result_path.with_suffix(".plist")

  def start(self) -> None:
    self._power_metrics_process = self.browser_platform.popen(
        "sudo",
        "powermetrics",
        "-f",
        "plist",
        f"--samplers={','.join(map(str, self.probe.samplers))}",
        "-i",
        f"{int(self.probe.sampling_interval.total_seconds())}",
        "--output-file",
        self._output_plist_file,
        stdout=subprocess.DEVNULL)
    if self._power_metrics_process.poll():
      raise ValueError("Could not start powermetrics")
    atexit.register(self.stop_process)

  def stop(self) -> None:
    if self._power_metrics_process:
      self._power_metrics_process.terminate()

  def teardown(self) -> ProbeResult:
    self.stop_process()
    return self.browser_result(file=(self._output_plist_file,))

  def stop_process(self) -> None:
    if self._power_metrics_process:
      self.browser_platform.terminate_gracefully(self._power_metrics_process)
      self._power_metrics_process = None
