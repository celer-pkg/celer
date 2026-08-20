# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.action_runner.action.enums import ReadyState
from crossbench.benchmarks.jetstream.jetstream import (JetStreamBenchmark,
                                                       JetStreamProbe,
                                                       JetStreamProbeContext,
                                                       JetStreamStory)

if TYPE_CHECKING:
  from crossbench.benchmarks.base import VersionParts
  from crossbench.runner.run import Run


class JetStream11Probe(JetStreamProbe):
  __doc__ = JetStreamProbe.__doc__
  NAME: str = "jetstream_1.1"

  @override
  def get_context_cls(self) -> Type[JetStream11ProbeContext]:
    return JetStream11ProbeContext


class JetStream11ProbeContext(JetStreamProbeContext):
  JS: str = f"""
  let results = Object.create(null);
  for (let name in JetStream.results) {{
    const benchmark = JetStream.results[name];
    if (name == "geomean") name = "{JetStreamProbe.TOTAL_METRIC_KEY}";
    const data = {{ score: benchmark.statistics.mean }};
    results[name] = data;
  }};
  return JSON.stringify(results);
  """


class JetStream11Story(JetStreamStory):
  NAME: str = "jetstream_1.1"
  URL: str = "https://chromium-workloads.web.app/jetstream/v1.1/"
  # TODO: host v1.1-custom on chromium-workloads.web.app/
  # URL_CHROME_FORK: str = "https://chromium-workloads.web.app/jetstream/v1.1-custom/"
  URL_OFFICIAL: str = "https://browserbench.org/JetStream1.1/"
  SUBSTORIES: tuple[str, ...] = (
      "3d-cube",
      "3d-raytrace",
      "base64",
      "cdjs",
      "code-first-load",
      "code-multi-load",
      "crypto-aes",
      "crypto-md5",
      "crypto-sha1",
      "date-format-tofte",
      "date-format-xparb",
      "mandreel",
      "n-body",
      "regex-dna",
      "splay",
      "tagcloud",
      "typescript",
      "bigfib.cpp",
      "box2d",
      "container.cpp",
      "crypto",
      "delta-blue",
      "dry.c",
      "earley-boyer",
      "float-mm.c",
      "gbemu",
      "gcc-loops.cpp",
      "hash-map",
      "n-body.c",
      "navier-stokes",
      "pdfjs",
      "proto-raytracer",
      "quicksort.c",
      "regexp-2010",
      "richards",
      "towers.c",
      "zlib",
  )

  @override
  def setup(self, run: Run) -> None:
    with run.actions("Setup") as actions:
      actions.show_url(
          url=self.get_run_url(run),
          ready_state=ReadyState.COMPLETE,
          timeout=dt.timedelta(seconds=10))
      actions.wait_js_condition(
          "return !!JetStream;", min_interval=0.01, timeout=2)
      actions.js("JetStream.initialize();")
      # Intercept console.log to capture the raw results.
      actions.js("""
          JetStream.results = undefined;
          const originalConsoleLog = console.log;
          console.log = function(name, value, ...args) {
            if (name === "Raw results:") {
              JetStream.results = JSON.parse(value);
            }
            originalConsoleLog(name, value, ...args);
          }
      """)
      # TODO: Support substories by intercepting JetStream.addPlan
      if self._substories != self.SUBSTORIES:
        logging.error("%s does not support story filtering yet.", self)

  def run_wait_until_done(self, run: Run) -> None:
    with run.actions("Waiting for completion") as actions:
      actions.wait_js_condition(
          """
          let summaryElement = document.getElementById("result-summary");
          return (summaryElement.innerHTML.indexOf("Score") >= 0);
          """,
          0.5,
          timeout=self.slow_duration,
          delay=self.substory_duration)


ProbeClsTupleT = tuple[Type[JetStream11Probe], ...]


class JetStream11Benchmark(JetStreamBenchmark):
  """
  Benchmark runner for JetStream 1.1.
  """

  NAME: str = "jetstream_1.1"
  DEFAULT_STORY_CLS = JetStream11Story
  PROBES: ProbeClsTupleT = (JetStream11Probe,)

  @classmethod
  @override
  def version(cls) -> VersionParts:
    return (1, 1)
