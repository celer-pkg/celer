# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.benchmarks.speedometer.speedometer_3 import (
    Speedometer3Benchmark, Speedometer3Probe, Speedometer3ProbeContext,
    Speedometer3Story)

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts
  from crossbench.benchmarks.speedometer.speedometer import ProbeClsTupleT


class Speedometer31Probe(Speedometer3Probe):
  """
  Speedometer3-specific probe (compatible with v3.1).
  Extracts all speedometer times and scores.
  """
  NAME: str = "speedometer_3.1"

  @override
  def get_context_cls(self) -> Type[Speedometer31ProbeContext]:
    return Speedometer31ProbeContext


class Speedometer31ProbeContext(Speedometer3ProbeContext):
  pass


class Speedometer31Story(Speedometer3Story):
  __doc__ = Speedometer3Story.__doc__
  NAME: str = "speedometer_3.1"
  URL: str = "https://chromium-workloads.web.app/speedometer/v3.1/"
  URL_OFFICIAL: str = "https://browserbench.org/Speedometer3.1/"
  URL_CHROME_FORK: str = "https://chromium-workloads.web.app/speedometer/v3.1-custom/"


class Speedometer31Benchmark(Speedometer3Benchmark):
  """
  Benchmark runner for Speedometer 3.1
  """
  NAME: str = "speedometer_3.1"
  DEFAULT_STORY_CLS = Speedometer31Story  # type: ignore
  PROBES: ProbeClsTupleT = (Speedometer31Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (3, 1)

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("sp", "speedometer", "sp3", "speedometer3",
            "speedometer_3") + super().aliases()
