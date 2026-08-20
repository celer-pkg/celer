# Copyright 2024 The Chromium Authors
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

class JetStream22Probe(JetStream2Probe):
  __doc__ = JetStream2Probe.__doc__
  NAME: str = "jetstream_2.2"

  @override
  def get_context_cls(self) -> Type[JetStream22ProbeContext]:
    return JetStream22ProbeContext


class JetStream22ProbeContext(JetStream2ProbeContext):
  pass


class JetStream22Story(JetStream2Story):
  __doc__ = JetStream2Story.__doc__
  NAME: str = "jetstream_2.2"
  URL: str = "https://chromium-workloads.web.app/jetstream/v2.2/"
  URL_OFFICIAL: str = "https://browserbench.org/JetStream2.2/"
  URL_CHROME_FORK: str = "https://chromium-workloads.web.app/jetstream/v2.2-custom/"


class JetStream22Benchmark(JetStream2Benchmark):
  """
  Benchmark runner for JetStream 2.2.
  """

  NAME: str = "jetstream_2.2"
  DEFAULT_STORY_CLS = JetStream22Story
  PROBES: ProbeClsTupleT = (JetStream22Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (2, 2)

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("js", "jetstream", "js2", "jetstream_2") + super().aliases()
