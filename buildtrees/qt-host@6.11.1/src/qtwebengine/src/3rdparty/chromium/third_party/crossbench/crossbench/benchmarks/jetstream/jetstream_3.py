# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import Type

from crossbench.benchmarks.jetstream.jetstream_2 import (JetStream2Benchmark,
                                                         JetStream2Probe,
                                                         JetStream2ProbeContext,
                                                         JetStream2Story)


# TODO: introduce JetStreamProbe
class JetStream3Probe(JetStream2Probe, metaclass=abc.ABCMeta):
  """
  JetStream3-specific Probe.
  Extracts all JetStream 3 times and scores.
  """


class JetStream3ProbeContext(JetStream2ProbeContext):
  pass


# TODO: introduce JetStreamStory
class JetStream3Story(JetStream2Story, metaclass=abc.ABCMeta):
  SUBSTORIES: tuple[str, ...] = ()


ProbeClsTupleT = tuple[Type[JetStream3Probe], ...]


# TODO: introduce JetStreamBenchmark
class JetStream3Benchmark(JetStream2Benchmark):
  pass
