# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Any, MutableMapping, Type

from typing_extensions import override

from crossbench.benchmarks.speedometer.speedometer_3 import (
    Speedometer3Benchmark, Speedometer3BenchmarkStoryFilter, Speedometer3Probe,
    Speedometer3ProbeContext, Speedometer3Story)

if TYPE_CHECKING:
  import argparse

  from crossbench.benchmarks.base import VersionParts
  from crossbench.benchmarks.speedometer.speedometer import ProbeClsTupleT


class SpeedometerMainProbe(Speedometer3Probe):
  """
  Speedometer3-specific probe (compatible with the main version).
  Extracts all speedometer times and scores.
  """
  NAME: str = "speedometer_main"

  @override
  def get_context_cls(self) -> Type[SpeedometerMainProbeContext]:
    return SpeedometerMainProbeContext


class SpeedometerMainProbeContext(Speedometer3ProbeContext):
  pass


class SpeedometerMainBenchmarkStoryFilter(Speedometer3BenchmarkStoryFilter):
  __doc__ = Speedometer3BenchmarkStoryFilter.__doc__

  @classmethod
  @override
  def add_cli_arguments(
      cls, parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser = super().add_cli_arguments(parser)
    parser.add_argument(
        "--measure-prepare",
        default=False,
        action="store_true",
        help="Include benchmark setup time in score.")
    return parser

  @classmethod
  @override
  def url_params_from_cli(cls,
                          args: argparse.Namespace) -> MutableMapping[str, Any]:
    url_params: MutableMapping[str, str] = super().url_params_from_cli(args)
    if args.measure_prepare:
      url_params["measurePrepare"] = ""
    return url_params


class SpeedometerMainStory(Speedometer3Story):
  __doc__ = Speedometer3Story.__doc__
  NAME: str = "speedometer_main"
  URL: str = "https://chromium-workloads.web.app/speedometer/main/"
  URL_OFFICIAL: str = "https://chromium-workloads.web.app/speedometer/main/"
  URL_CHROME_FORK: str = "https://chromium-workloads.web.app/speedometer/main-custom/"


class SpeedometerMainBenchmark(Speedometer3Benchmark):
  """
  Benchmark runner for the Speedometer main version.
  """
  NAME: str = "speedometer_main"
  DEFAULT_STORY_CLS = SpeedometerMainStory  # type: ignore
  PROBES: ProbeClsTupleT = (SpeedometerMainProbe,)
  STORY_FILTER_CLS = SpeedometerMainBenchmarkStoryFilter

  @classmethod
  @override
  def version(cls) -> VersionParts:
    # Using fake next version as a hack.
    return ("main",)
