# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.benchmarks.jetstream.jetstream_2 import (JetStream2Benchmark,
                                                         JetStream2Probe,
                                                         JetStream2ProbeContext,
                                                         JetStream2Story,
                                                         ProbeClsTupleT)

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts

class JetStream20Probe(JetStream2Probe):
  __doc__ = JetStream2Probe.__doc__
  NAME: str = "jetstream_2.0"

  @override
  def get_context_cls(self) -> Type[JetStream20ProbeContext]:
    return JetStream20ProbeContext


class JetStream20ProbeContext(JetStream2ProbeContext):
  pass


class JetStream20Story(JetStream2Story):
  __doc__ = JetStream2Story.__doc__
  NAME: str = "jetstream_2.0"
  URL: str = "https://chromium-workloads.web.app/jetstream/v2.0/"
  URL_OFFICIAL: str = "https://browserbench.org/JetStream2.0/"
  URL_CHROME_FORK: str = "https://chromium-workloads.web.app/jetstream/v2.0-custom/"


class JetStream20Benchmark(JetStream2Benchmark):
  """
  Benchmark runner for JetStream 2.0.
  """

  NAME: str = "jetstream_2.0"
  DEFAULT_STORY_CLS = JetStream20Story
  PROBES: ProbeClsTupleT = (JetStream20Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (2, 0)
