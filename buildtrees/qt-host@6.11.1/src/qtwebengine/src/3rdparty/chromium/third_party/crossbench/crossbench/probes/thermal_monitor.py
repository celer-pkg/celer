# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import json
import logging
import re
from enum import IntEnum
from typing import TYPE_CHECKING, Iterable, Optional

from typing_extensions import override

from crossbench.helper.wait import WaitRange
from crossbench.probes.internal.base import (InternalJsonResultProbe,
                                             InternalJsonResultProbeContext)
from crossbench.probes.probe import ProbeIncompatibleBrowser
from crossbench.probes.result_location import ResultLocation
from crossbench.probes.results import EmptyProbeResult, LocalProbeResult

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.probes.probe_context import ProbeContext
  from crossbench.probes.results import ProbeResult, ProbeResultDict
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run
  from crossbench.types import Json

THERMAL_STATUS_RE: re.Pattern[str] = re.compile(
    r"Thermal Status: (?P<status>\d+)")
COOLDOWN_WAIT_RANGE = WaitRange(
    min=dt.timedelta(seconds=1), timeout=dt.timedelta(minutes=5))


class ThermalStatus(IntEnum):
  UNAVAILABLE = -1
  NONE = 0
  LIGHT = 1
  MODERATE = 2
  SEVERE = 3
  CRITICAL = 4
  EMERGENCY = 5
  SHUTDOWN = 6

  @classmethod
  def parse(cls, value: str) -> ThermalStatus:
    try:
      return ThermalStatus(int(value))
    except ValueError:
      pass

    for member in ThermalStatus:
      if value.upper().endswith(member.name):
        return member

    raise ValueError(f"Invalid ThermalStatus: {repr(value)}")


class ThermalMonitorProbe(InternalJsonResultProbe):
  """
  Internal probe to monitor device thermal status.
  """
  NAME = "cb.thermal_monitor"
  RESULT_LOCATION = ResultLocation.LOCAL

  def __init__(self,
               cool_down_time: dt.timedelta = dt.timedelta(),
               threshold: Optional[ThermalStatus] = None) -> None:
    super().__init__()
    self._threshold: ThermalStatus | None = threshold
    self._cool_down_time: dt.timedelta = cool_down_time
    if threshold is not None and threshold <= 0:
      raise ValueError("Threshold must be positive")

  @property
  @override
  def result_path_name(self) -> str:
    return "cb.thermal_monitor.json"

  @property
  def threshold(self) -> Optional[ThermalStatus]:
    return self._threshold

  @property
  def cool_down_time(self) -> dt.timedelta:
    return self._cool_down_time

  def to_json(self, actions: Actions) -> Json:
    raise NotImplementedError("Should not be called, data comes from context")

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    if self.threshold is not None and not browser.platform.is_android:
      raise ProbeIncompatibleBrowser(
          self, browser, "Thermal thresholds only supported on android")

  @override
  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    return self._merge_group(group, (run.results for run in group.runs))

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    return self._merge_group(
        group, (rep_group.results for rep_group in group.repetitions_groups))

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self._merge_group(
        group, (story_group.results for story_group in group.story_groups))

  def _merge_group(self, group,
                   results_iter: Iterable[ProbeResultDict]) -> ProbeResult:
    group_max_status: ThermalStatus = ThermalStatus.UNAVAILABLE
    has_results: bool = False
    for results in results_iter:
      result = results[self]
      if not result:
        continue
      with result.json.open(encoding="utf-8") as f:
        thermals = json.load(f)
        if "max_observed_status" not in thermals:
          continue
        repetition_max_status = ThermalStatus(thermals["max_observed_status"])
        group_max_status = max(group_max_status, repetition_max_status)
        has_results = True

    if not has_results:
      return EmptyProbeResult()

    merged_path = group.get_local_probe_result_path(self)
    with merged_path.open("w", encoding="utf-8") as f:
      json.dump({"max_observed_status": group_max_status}, f, indent=2)
      # TODO(375390958): figure out why files aren't fully written to
      # pyfakefs here.
      f.write("\n")

    return LocalProbeResult(json=(merged_path,))

  @override
  def log_browsers_result(self, group: BrowsersRunGroup) -> None:
    if self not in group.results:
      return
    result = group.results[self]
    if not result:
      return

    with result.json.open(encoding="utf-8") as f:
      thermals = json.load(f)
      max_observed_status = ThermalStatus(thermals["max_observed_status"])

    if max_observed_status == ThermalStatus.LIGHT:
      logging.info("-" * 80)
      logging.error("Light thermal throttling detected during execution, "
                    "scores may be affected.")
    elif max_observed_status > ThermalStatus.LIGHT:
      logging.info("-" * 80)
      logging.error("Significant thermal throttling detected during execution, "
                    "scores are not representative of the device performance.")

  def get_context(self, run: Run) -> Optional[ProbeContext]:
    if run.browser.platform.is_android:
      return AndroidThermalMonitorProbeContext(self, run)
    return ThermalMonitorProbeContext(self, run)


class ThermalMonitorProbeContext(
    InternalJsonResultProbeContext[ThermalMonitorProbe]):

  def __init__(self, probe: ThermalMonitorProbe, run: Run) -> None:
    super().__init__(probe, run)

  @property
  def probe(self) -> ThermalMonitorProbe:
    return self._probe

  def setup(self) -> None:
    self.run.runner.wait(self.probe.cool_down_time, absolute_time=True)

    if not self.browser_platform.is_thermal_throttled():
      return
    logging.info("COOLDOWN")
    for _ in COOLDOWN_WAIT_RANGE.wait_with_backoff():
      if not self.browser_platform.is_thermal_throttled():
        break
      logging.info("COOLDOWN: still hot, waiting some more")

  @override
  def to_json(self, actions: Actions) -> Json:
    del actions
    return {}


class AndroidThermalMonitorProbeContext(ThermalMonitorProbeContext):

  def __init__(self, probe: ThermalMonitorProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._max_observed_status: ThermalStatus = ThermalStatus.UNAVAILABLE

  def _get_thermal_status(self) -> ThermalStatus:
    stdout = self.browser_platform.sh_stdout("dumpsys", "thermalservice")
    if match := THERMAL_STATUS_RE.search(stdout):
      return ThermalStatus(int(match["status"]))
    return ThermalStatus.UNAVAILABLE

  def _wait_if_necessary(self, probe_threshold: ThermalStatus) -> None:
    current_status = self._get_thermal_status()
    if current_status < probe_threshold:
      return

    logging.info("Thermal throttling status too high: %s", current_status.name)
    logging.info("COOLDOWN")
    try:
      for _ in COOLDOWN_WAIT_RANGE.wait_with_backoff():
        current_status = self._get_thermal_status()
        logging.debug("Thermal status: %s", current_status.name)
        if current_status < probe_threshold:
          logging.info("COOLDOWN: complete")
          break
    except TimeoutError:
      logging.error("COOLDOWN: device is still too hot after waiting for %s",
                    COOLDOWN_WAIT_RANGE.timeout)

  @override
  def setup(self) -> None:
    if self.probe.threshold is not None:
      self._wait_if_necessary(self.probe.threshold)
    else:
      super().setup()

    current_status = self._get_thermal_status()
    self._max_observed_status = max(self._max_observed_status, current_status)
    logging.debug("Thermal throttling before run: %s", current_status.name)

  @override
  def teardown(self) -> ProbeResult:
    current_status = self._get_thermal_status()
    self._max_observed_status = max(self._max_observed_status, current_status)
    logging.debug("Thermal throttling after run: %s", current_status.name)
    # TODO(crbug.com/374737038): After crbug.com/374737038 is done, raise an
    # exception here if max status was at threshold or higher. This will
    # register the run as a failure to process it correctly later.
    return super().teardown()

  @override
  def to_json(self, actions: Actions) -> Json:
    del actions
    return {"max_observed_status": self._max_observed_status.value}
