# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import Any, Final, Mapping

from immutabledict import immutabledict

# TODO: improve and double check
TRAFFIC_SETTINGS: Final[Mapping[str, Any]] = immutabledict({
    "3G-slow": {
        "rtt_ms": 400,
        "in_kbps": 400,
        "out_kbps": 400,
    },
    "3G-regular": {
        "rtt_ms": 300,
        "in_kbps": 1600,
        "out_kbps": 768,
    },
    "3G-fast": {
        "rtt_ms": 150,
        "in_kbps": 1600,
        "out_kbps": 768,
    },
    "4G": {
        "rtt_ms": 170,
        "in_kbps": 9000,
        "out_kbps": 9000,
    },
})
DEFAULT_WINDOW_SIZE: Final[int] = 10
DEFAULT_TIMEOUT: Final[int] = 5
