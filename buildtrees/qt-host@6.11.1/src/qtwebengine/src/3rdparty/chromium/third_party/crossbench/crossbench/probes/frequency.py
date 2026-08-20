# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
from typing import TYPE_CHECKING, Self, Type

from typing_extensions import override

from crossbench.probes.cpu_frequency_map import CPUFrequencyMap
from crossbench.probes.env_modifier import EnvModifier
from crossbench.probes.probe import ProbeConfigParser, ProbeContext, ProbeKeyT

if TYPE_CHECKING:
  from immutabledict import immutabledict

  from crossbench import path as pth
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


class FrequencyProbe(EnvModifier):
  """
  Probe to pin a frequency for certain parts of the system, e.g. CPUs and
  memory on platforms with SysFS (Linux and Android). As of 10/2024, only CPUs
  are supported. The probe can be configured as follows:

  // Probe config HJSON.
  frequency: {
    cpus: {
      cpu0: 1111,
      cpu1: "min", // Will use the minimum allowed frequency.
      cpu2: "max"  // Will use the maximum allowed frequency.
    }
  }

  Generally, the system only allows a certain set of frequency values (for CPUs
  the values can be found in [1]). Using an invalid value in the probe config
  will cause a runtime error, but also print the list of valid values. Numerical
  values can be specified as both integers (1111) and strings ("1111").

  Wildcards are supported in 2 ways:

  frequency: {
    cpus: "max"
  }


  frequency: {
    cpus: {
      // When * is used, there should be no other keys in the map.
      *: "max"
    }
  }

  Note that when running with different platforms (e.g.
  --browser=android:chrome-stable --browser=linux:chrome-stable), "*", "min"
  and "max" might mean different things for each platform.

  [1] https://docs.kernel.org/admin-guide/pm/cpufreq.html#:~:text=scaling_available_frequencies
  """

  NAME = "frequency"

  IS_GENERAL_PURPOSE = True
  PRODUCES_DATA = False

  def __init__(self, cpus: CPUFrequencyMap) -> None:
    super().__init__()
    self._cpu_frequency_map: CPUFrequencyMap = cpus

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "cpus",
        type=CPUFrequencyMap,
        default=CPUFrequencyMap.parse({}),
        help="CPU frequency map, see FrequencyProbe docs")
    return parser

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (("cpus", self._cpu_frequency_map.key),)

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    # As long as a valid platform map can be derived, all is good.
    self._cpu_frequency_map.get_target_frequencies(browser.platform)

  @property
  def cpu_frequency_map(self) -> CPUFrequencyMap:
    return self._cpu_frequency_map

  @override
  def get_context_cls(self) -> Type[FrequencyProbeContext]:
    return FrequencyProbeContext


@dataclasses.dataclass(frozen=True)
class _FrequencyState:
  dir: pth.AnyPosixPath
  min: str
  max: str


class FrequencyProbeContext(ProbeContext[FrequencyProbe]):

  _MIN_FREQUENCY_FILE: str = "scaling_min_freq"
  _MAX_FREQUENCY_FILE: str = "scaling_max_freq"


  def __init__(self, probe: FrequencyProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._previous_frequencies: list[_FrequencyState] = []

  def start(self) -> None:
    target_cpu_frequencies: immutabledict[pth.AnyPosixPath, int] = (
        self.probe.cpu_frequency_map.get_target_frequencies(
            self.browser_platform))
    for cpu_dir in target_cpu_frequencies.keys():
      self._previous_frequencies.append(
          _FrequencyState(
              dir=cpu_dir,
              min=self.browser_platform.cat(cpu_dir / self._MIN_FREQUENCY_FILE),
              max=self.browser_platform.cat(cpu_dir /
                                            self._MAX_FREQUENCY_FILE)))

    try:
      for cpu_dir, frequency in target_cpu_frequencies.items():
        self.browser_platform.write_text(cpu_dir / self._MIN_FREQUENCY_FILE,
                                         f"{frequency}\n")
        self.browser_platform.write_text(cpu_dir / self._MAX_FREQUENCY_FILE,
                                         f"{frequency}\n")
    except Exception:
      self._restore_frequencies()
      raise

  def stop(self) -> None:
    self._restore_frequencies()

  def _restore_frequencies(self) -> None:
    for state in self._previous_frequencies:
      self.browser_platform.write_text(state.dir / self._MIN_FREQUENCY_FILE,
                                       state.min)
      self.browser_platform.write_text(state.dir / self._MAX_FREQUENCY_FILE,
                                       state.max)

  def teardown(self) -> ProbeResult:
    return self.empty_result()
