# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import Final

from crossbench.benchmarks.speedometer.speedometer_1_0 import \
    Speedometer10Benchmark
from crossbench.benchmarks.speedometer.speedometer_2_0 import \
    Speedometer20Benchmark
from crossbench.benchmarks.speedometer.speedometer_2_1 import \
    Speedometer21Benchmark
from crossbench.benchmarks.speedometer.speedometer_3_0 import \
    Speedometer30Benchmark
from crossbench.benchmarks.speedometer.speedometer_3_1 import \
    Speedometer31Benchmark
from crossbench.benchmarks.speedometer.speedometer_main import \
    SpeedometerMainBenchmark

benchmark_classes: Final = (Speedometer10Benchmark, Speedometer20Benchmark,
                            Speedometer21Benchmark, Speedometer30Benchmark,
                            Speedometer31Benchmark, SpeedometerMainBenchmark)

_versions = set()
for benchmark_cls in benchmark_classes:
  assert benchmark_cls.version() not in _versions, (
      f"Got duplicated benchmark version for {benchmark_cls}")
  _versions.add(benchmark_cls.version())
