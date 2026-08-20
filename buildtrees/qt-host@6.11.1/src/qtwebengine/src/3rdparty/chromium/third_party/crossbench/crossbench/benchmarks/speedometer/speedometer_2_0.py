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

class Speedometer20Probe(Speedometer2Probe):
  NAME: str = "speedometer_2.0"

  @override
  def get_context_cls(self) -> Type[Speedometer20ProbeContext]:
    return Speedometer20ProbeContext


class Speedometer20ProbeContext(Speedometer2ProbeContext):
  pass


class Speedometer20Story(Speedometer2Story):
  NAME: str = "speedometer_2.0"
  URL: str = "https://chromium-workloads.web.app/speedometer/v2.0/"
  URL_OFFICIAL: str = "https://browserbench.org/Speedometer2.0/"
  URL_CHROME_FORK: str = "https://chromium-workloads.web.app/speedometer/v2.0-custom/"


class Speedometer20Benchmark(SpeedometerBenchmark):
  """
  Benchmark runner for Speedometer 2.0
  """
  NAME: str = "speedometer_2.0"
  DEFAULT_STORY_CLS = Speedometer20Story
  PROBES: ProbeClsTupleT = (Speedometer20Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (2, 0)
