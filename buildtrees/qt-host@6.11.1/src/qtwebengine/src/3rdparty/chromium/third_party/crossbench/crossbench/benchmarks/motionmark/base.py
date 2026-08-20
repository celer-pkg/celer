# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing_extensions import override

from crossbench.benchmarks.base import PressBenchmark


class MotionMarkBenchmark(PressBenchmark):

  @classmethod
  @override
  def short_base_name(cls) -> str:
    return "mm"

  @classmethod
  @override
  def base_name(cls) -> str:
    return "motionmark"
