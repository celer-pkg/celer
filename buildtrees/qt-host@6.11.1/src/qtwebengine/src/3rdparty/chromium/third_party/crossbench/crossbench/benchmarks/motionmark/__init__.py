# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from crossbench.benchmarks.motionmark.motionmark_1_0 import \
    MotionMark10Benchmark
from crossbench.benchmarks.motionmark.motionmark_1_1 import \
    MotionMark11Benchmark
from crossbench.benchmarks.motionmark.motionmark_1_2 import \
    MotionMark12Benchmark
from crossbench.benchmarks.motionmark.motionmark_1_3 import \
    MotionMark13Benchmark
from crossbench.benchmarks.motionmark.motionmark_1_3_1 import \
    MotionMark131Benchmark
from crossbench.benchmarks.motionmark.motionmark_main import \
    MotionMarkMainBenchmark

benchmark_classes = (MotionMark10Benchmark, MotionMark11Benchmark,
                     MotionMark12Benchmark, MotionMark13Benchmark,
                     MotionMark131Benchmark, MotionMarkMainBenchmark)

_versions = set()
for benchmark_cls in benchmark_classes:
  assert benchmark_cls.version() not in _versions, (
      f"Got duplicated benchmark version for {benchmark_cls}")
  _versions.add(benchmark_cls.version())
