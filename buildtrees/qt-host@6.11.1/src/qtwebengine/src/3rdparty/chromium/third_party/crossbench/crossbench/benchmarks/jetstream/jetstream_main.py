# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.benchmarks.jetstream.jetstream_3 import (
    JetStream3Benchmark, JetStream3Probe, JetStream3ProbeContext,
    JetStream3Story, ProbeClsTupleT)

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts


class JetStreamMainProbe(JetStream3Probe):
  __doc__ = JetStream3Probe.__doc__
  NAME: str = "jetstream_main"

  @override
  def get_context_cls(self) -> Type[JetStreamMainProbeContext]:
    return JetStreamMainProbeContext


class JetStreamMainProbeContext(JetStream3ProbeContext):
  pass


class JetStreamMainStory(JetStream3Story):
  __doc__ = JetStream3Story.__doc__
  NAME: str = "jetstream_main"
  URL: str = "https://chromium-workloads.web.app/jetstream/main/"
  URL_OFFICIAL: str = "https://chromium-workloads.web.app/jetstream/main/"
  URL_CHROME_FORK: str = "https://chromium-workloads.web.app/jetstream/main-custom/"
  # Contents of running:
  # JSON.stringify(JetStream.benchmarks.map(e => e.name), undefined, " ")
  SUBSTORIES: tuple[str, ...] = (
      "zlib-wasm",
      "WSL",
      "UniPoker",
      "uglify-js-wtb",
      "typescript",
      "tsf-wasm",
      "tfjs-wasm-simd",
      "tfjs-wasm",
      "tagcloud-SP",
      "sync-fs",
      "string-unpack-code-SP",
      "stanford-crypto-sha256",
      "stanford-crypto-pbkdf2",
      "stanford-crypto-aes",
      "sqlite3-wasm",
      "splay",
      "segmentation",
      "richards-wasm",
      "richards",
      "regexp",
      "regex-dna-SP",
      "raytrace-public-class-fields",
      "raytrace-private-class-fields",
      "raytrace",
      "quicksort-wasm",
      "proxy-vue",
      "proxy-mobx",
      "prepack-wtb",
      "pdfjs",
      "OfflineAssembler",
      "octane-code-load",
      "navier-stokes",
      "n-body-SP",
      "multi-inspector-code-load",
      "ML",
      "mandreel",
      "lebab-wtb",
      "lazy-collections",
      "json-stringify-inspector",
      "json-parse-inspector",
      "jshint-wtb",
      "js-tokens",
      "HashSet-wasm",
      "hash-map",
      "gcc-loops-wasm",
      "gbemu",
      "gaussian-blur",
      "FlightPlanner",
      "first-inspector-code-load",
      "espree-wtb",
      "earley-boyer",
      "doxbee-promise",
      "doxbee-async",
      "delta-blue",
      "date-format-xparb-SP",
      "date-format-tofte-SP",
      "Dart-flute-wasm",
      "crypto-sha1-SP",
      "crypto-md5-SP",
      "crypto-aes-SP",
      "crypto",
      "coffeescript-wtb",
      "chai-wtb",
      "cdjs",
      "Box2D",
      "bomb-workers",
      "bigint-noble-ed25519",
      "Basic",
      "base64-SP",
      "babylon-wtb",
      "Babylon",
      "async-fs",
      "argon2-wasm",
      "Air",
      "ai-astar",
      "acorn-wtb",
      "8bitbench-wasm",
      "3d-raytrace-SP",
      "3d-cube-SP",
  )


class JetStreamMainBenchmark(JetStream3Benchmark):
  """
  Benchmark runner for the JetStream main developement vresion.
  """

  NAME: str = "jetstream_main"
  DEFAULT_STORY_CLS = JetStreamMainStory
  PROBES: ProbeClsTupleT = (JetStreamMainProbe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return ("main",)
