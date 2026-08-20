# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from crossbench.benchmarks.embedder import EmbedderBenchmark
from crossbench.benchmarks.jetstream import (JetStream11Benchmark,
                                             JetStream20Benchmark,
                                             JetStream21Benchmark,
                                             JetStream22Benchmark,
                                             JetStreamMainBenchmark)
from crossbench.benchmarks.loading.loading_benchmark import LoadingBenchmark
from crossbench.benchmarks.loadline import (
    LoadLine1PhoneBenchmark, LoadLine1PhoneDebugBenchmark,
    LoadLine1PhoneFastBenchmark, LoadLine1TabletBenchmark,
    LoadLine1TabletDebugBenchmark, LoadLine1TabletFastBenchmark,
    LoadLine2PhoneBenchmark, LoadLine2PhoneDebugBenchmark,
    LoadLine2TabletBenchmark, LoadLine2TabletDebugBenchmark)
from crossbench.benchmarks.manual import ManualBenchmark
from crossbench.benchmarks.memory.memory_benchmark import MemoryBenchmark
from crossbench.benchmarks.motionmark import (
    MotionMark10Benchmark, MotionMark11Benchmark, MotionMark12Benchmark,
    MotionMark13Benchmark, MotionMark131Benchmark, MotionMarkMainBenchmark)
from crossbench.benchmarks.powerline import PowerlineBenchmark
from crossbench.benchmarks.speedometer import (
    Speedometer10Benchmark, Speedometer20Benchmark, Speedometer21Benchmark,
    Speedometer30Benchmark, Speedometer31Benchmark, SpeedometerMainBenchmark)

__all__ = [
    "EmbedderBenchmark",
    "JetStream11Benchmark",
    "JetStream20Benchmark",
    "JetStream21Benchmark",
    "JetStream22Benchmark",
    "JetStreamMainBenchmark",
    "LoadingBenchmark",
    "LoadLine1PhoneBenchmark",
    "LoadLine1PhoneDebugBenchmark",
    "LoadLine1PhoneFastBenchmark",
    "LoadLine1TabletBenchmark",
    "LoadLine1TabletDebugBenchmark",
    "LoadLine1TabletFastBenchmark",
    "LoadLine2PhoneBenchmark",
    "LoadLine2PhoneDebugBenchmark",
    "LoadLine2TabletBenchmark",
    "LoadLine2TabletDebugBenchmark",
    "ManualBenchmark",
    "MemoryBenchmark",
    "MotionMark10Benchmark",
    "MotionMark11Benchmark",
    "MotionMark12Benchmark",
    "MotionMark131Benchmark",
    "MotionMark13Benchmark",
    "MotionMarkMainBenchmark",
    "PowerlineBenchmark",
    "Speedometer10Benchmark",
    "Speedometer20Benchmark",
    "Speedometer21Benchmark",
    "Speedometer30Benchmark",
    "Speedometer31Benchmark",
    "SpeedometerMainBenchmark",
]
