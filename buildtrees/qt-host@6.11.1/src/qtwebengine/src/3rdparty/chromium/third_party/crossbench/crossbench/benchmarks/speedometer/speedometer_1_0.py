# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Any, Type

from typing_extensions import override

from crossbench.benchmarks.speedometer.speedometer import (
    ProbeClsTupleT, SpeedometerBenchmark, SpeedometerProbe,
    SpeedometerProbeContext, SpeedometerStory)
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts


class Speedometer10Probe(SpeedometerProbe):
  NAME: str = "speedometer_1.0"

  @override
  def get_context_cls(self) -> Type[Speedometer10ProbeContext]:
    return Speedometer10ProbeContext


class Speedometer10ProbeContext(SpeedometerProbeContext):

  @override
  def process_json_data(self, json_data) -> Any:
    json_data = ObjectParser.non_empty_sequence(json_data,
                                                f"{self.probe.name} metrics")
    # Move aggregate scores to the end
    for iteration_data in json_data:
      assert isinstance(iteration_data, dict)
      total = iteration_data.pop("total")
      suite_count = len(iteration_data["tests"])
      iteration_data["Total"] = total
      # Manually compute the v1.0 Score
      iteration_data["Score"] = 60 * 1000 * suite_count / total
    return json_data


class Speedometer10Story(SpeedometerStory):
  NAME: str = "speedometer_1.0"
  # TODO: Host on chromium-workloads
  # URL: str = "https://chromium-workloads.web.app/speedometer/v1.0/"
  URL: str = "https://browserbench.org/Speedometer/"
  URL_OFFICIAL: str = "https://browserbench.org/Speedometer/"
  SUBSTORIES: tuple[str, ...] = (
      "VanillaJS-TodoMVC",
      "EmberJS-TodoMVC",
      "BackboneJS-TodoMVC",
      "jQuery-TodoMVC",
      "AngularJS-TodoMVC",
      "React-TodoMVC",
      "FlightJS-TodoMVC",
      "FlightJS-MailClient",
  )


class Speedometer10Benchmark(SpeedometerBenchmark):
  """
  Benchmark runner for Speedometer 1.0
  """
  NAME: str = "speedometer_1.0"
  DEFAULT_STORY_CLS = Speedometer10Story
  PROBES: ProbeClsTupleT = (Speedometer10Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (1, 0)

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("sp1", "speedometer1") + super().aliases()
