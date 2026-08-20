# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from crossbench.benchmarks.jetstream.jetstream_1_1 import JetStream11Benchmark
from crossbench.benchmarks.jetstream.jetstream_2_0 import JetStream20Benchmark
from crossbench.benchmarks.jetstream.jetstream_2_1 import JetStream21Benchmark
from crossbench.benchmarks.jetstream.jetstream_2_2 import JetStream22Benchmark
from crossbench.benchmarks.jetstream.jetstream_main import \
    JetStreamMainBenchmark

__all__ = [
    "JetStream11Benchmark",
    "JetStream20Benchmark",
    "JetStream21Benchmark",
    "JetStream22Benchmark",
    "JetStreamMainBenchmark",
]
