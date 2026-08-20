# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from crossbench.benchmarks.base import Benchmark


class BenchmarkProbeMixin:
  NAME: str = ""
  IS_GENERAL_PURPOSE: bool = False

  def __init__(self, *args, **kwargs) -> None:
    self._benchmark: Benchmark = kwargs.pop("benchmark")
    #assert isinstance(self._benchmark, Benchmark)
    super().__init__(*args, **kwargs)

  @property
  def benchmark(self) -> Benchmark:
    return self._benchmark
