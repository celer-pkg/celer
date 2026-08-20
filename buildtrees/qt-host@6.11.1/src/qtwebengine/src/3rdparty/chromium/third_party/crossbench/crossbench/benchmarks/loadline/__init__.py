# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from crossbench.benchmarks.loadline.loadline_1 import (
    LoadLine1PhoneBenchmark, LoadLine1PhoneDebugBenchmark,
    LoadLine1PhoneFastBenchmark, LoadLine1TabletBenchmark,
    LoadLine1TabletDebugBenchmark, LoadLine1TabletFastBenchmark)
from crossbench.benchmarks.loadline.loadline_2 import (
    LoadLine2PhoneBenchmark, LoadLine2PhoneDebugBenchmark,
    LoadLine2TabletBenchmark, LoadLine2TabletDebugBenchmark)

__all__ = [
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
]
