# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.benchmarks.speedometer.speedometer import (ProbeClsTupleT,
                                                           SpeedometerBenchmark)
from crossbench.benchmarks.speedometer.speedometer_2 import (
    Speedometer2Probe, Speedometer2ProbeContext, Speedometer2Story)

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts

class Speedometer21Probe(Speedometer2Probe):
  NAME: str = "speedometer_2.1"

  @override
  def get_context_cls(self) -> Type[Speedometer21ProbeContext]:
    return Speedometer21ProbeContext


class Speedometer21ProbeContext(Speedometer2ProbeContext):
  pass



class Speedometer21Story(Speedometer2Story):
  NAME: str = "speedometer_2.1"
  URL: str = "https://chromium-workloads.web.app/speedometer/v2.1/"
  URL_OFFICIAL: str = "https://browserbench.org/Speedometer2.1/"
  URL_CHROME_FORK: str = "https://chromium-workloads.web.app/speedometer/v2.1-custom/"


class Speedometer21Benchmark(SpeedometerBenchmark):
  """
  Benchmark runner for Speedometer 2.1
  """
  NAME: str = "speedometer_2.1"
  DEFAULT_STORY_CLS = Speedometer21Story
  PROBES: ProbeClsTupleT = (Speedometer21Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (2, 1)

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("sp2", "speedometer2", "speedometer_2") + super().aliases()
