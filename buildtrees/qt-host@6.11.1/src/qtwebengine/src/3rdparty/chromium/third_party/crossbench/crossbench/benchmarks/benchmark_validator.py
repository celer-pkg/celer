# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import inspect
from typing import TYPE_CHECKING, Type

from crossbench.benchmarks.benchmark_probe import BenchmarkProbeMixin
from crossbench.probes.probe import Probe

if TYPE_CHECKING:
  from crossbench.benchmarks.base import Benchmark


def validate_cls(cls: Type[Benchmark]) -> None:
  for benchmark_probe_cls in cls.PROBES:
    assert inspect.isclass(benchmark_probe_cls), (
        f"{cls}.PROBES must contain classes only, "
        f"but got {type(benchmark_probe_cls)}")
    assert issubclass(
        benchmark_probe_cls,
        Probe), (f"Expected Probe class but got {type(benchmark_probe_cls)}")
    assert issubclass(benchmark_probe_cls, BenchmarkProbeMixin), (
        f"{benchmark_probe_cls} should be BenchmarkProbeMixin "
        f"for {type(cls)}.PROBES")
    assert benchmark_probe_cls.NAME, (  # type: ignore
        f"Expected probe.NAME for {benchmark_probe_cls}")
