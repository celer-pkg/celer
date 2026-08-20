# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.benchmarks.motionmark.motionmark_1 import (
    MotionMark1Benchmark, MotionMark1Probe, MotionMark1ProbeContext,
    MotionMark1Story)

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts

class MotionMark11Probe(MotionMark1Probe):
  __doc__ = MotionMark1Probe.__doc__
  NAME = "motionmark_1.1"

  @override
  def get_context_cls(self) -> Type[MotionMark11ProbeContext]:
    return MotionMark11ProbeContext


class MotionMark11ProbeContext(MotionMark1ProbeContext):
  pass


class MotionMark11Story(MotionMark1Story):
  NAME = "motionmark_1.1"
  URL: str = "https://chromium-workloads.web.app/motionmark/v1.1/MotionMark"
  URL_OFFICIAL: str = "https://browserbench.org/MotionMark1.1"


class MotionMark11Benchmark(MotionMark1Benchmark):
  """
  Benchmark runner for MotionMark 1.1.

  See https://browserbench.org/MotionMark1.1/ for more details.
  """

  NAME = "motionmark_1.1"
  DEFAULT_STORY_CLS = MotionMark11Story
  PROBES = (MotionMark11Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (1, 1)
