# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.benchmarks.motionmark.motionmark_1 import (
    MotionMark1Benchmark, MotionMark1Probe, MotionMark1ProbeContext,
    MotionMark1Story)

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts


class MotionMarkMainProbe(MotionMark1Probe):
  __doc__ = MotionMark1Probe.__doc__
  NAME = "motionmark_main"

  @override
  def get_context_cls(self) -> Type[MotionMarkMainProbeContext]:
    return MotionMarkMainProbeContext


class MotionMarkMainProbeContext(MotionMark1ProbeContext):
  pass


class MotionMarkMainStory(MotionMark1Story):
  NAME = "motionmark_main"
  URL: str = "https://chromium-workloads.web.app/motionmark/main/MotionMark/"
  URL_OFFICIAL: str = "https://chromium-workloads.web.app/motionmark/main/MotionMark/"
  READY_TIMEOUT: dt.timedelta = dt.timedelta(seconds=12)
  DEVELOPER_READY_JS: str = (
      "return !(document.querySelector('#frame-rate-detection span'));")
  READY_JS: str = (
      "return !!("
      "   document.querySelector('#frame-rate-label')?.textContent?.trim());")


class MotionMarkMainBenchmark(MotionMark1Benchmark):
  """
  Benchmark runner for MotionMark main.

  See https://browserbench.org/MotionMarkmain/ for more details.
  """

  NAME = "motionmark_main"
  DEFAULT_STORY_CLS = MotionMarkMainStory
  PROBES = (MotionMarkMainProbe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return ("main",)
