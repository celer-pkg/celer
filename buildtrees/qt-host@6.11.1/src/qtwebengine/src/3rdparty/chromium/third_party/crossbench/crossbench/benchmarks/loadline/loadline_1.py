# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
from typing import TYPE_CHECKING, Type

import numpy as np
import pandas as pd
from typing_extensions import override

from crossbench import config
from crossbench import path as pth
from crossbench.benchmarks.loadline.loadline import (LoadLineBenchmark,
                                                     LoadLineProbe)
from crossbench.flags.base import Flags
from crossbench.probes.perfetto.trace_processor.trace_processor import \
    TraceProcessorProbe
from crossbench.probes.probe_context import ProbeContext

if TYPE_CHECKING:
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.groups.browsers import BrowsersRunGroup

# We should increase the minor version number every time there are any changes
# that might affect the benchmark score.
VERSION_STRING = "1.3.0"


def process_scores(df: pd.DataFrame) -> pd.DataFrame:
  df = df.groupby(["cb_browser",
                   "cb_story"])["score"].mean().reset_index().pivot(
                       columns=["cb_story"],
                       index=["cb_browser"],
                       values=["score"])
  df = df.droplevel(0, axis=1)
  df["TOTAL_SCORE"] = np.exp(np.log(df).mean(axis=1))
  df.index.rename("browser", inplace=True)
  df = df.reindex(
      columns=(["TOTAL_SCORE"] +
               sorted(list(c for c in df.columns if c != "TOTAL_SCORE"))))
  return df


def process_breakdown(df: pd.DataFrame) -> pd.DataFrame:
  df["os"] = df[["network", "process_launch"]].max(axis=1)
  df = df.groupby(["cb_browser", "cb_story"
                  ])[["os", "renderer", "compositor", "gpu",
                      "surfaceflinger"]].mean()
  df.index.names = ["browser", "story"]
  return df


class LoadLine1Probe(LoadLineProbe):
  NAME = "loadline_probe"
  BENCHMARK_NAME = "LoadLine"
  BENCHMARK_VERSION = VERSION_STRING

  @override
  def get_context_cls(self,) -> Type[LoadLine1ProbeContext]:
    return LoadLine1ProbeContext

  def _load_query_result(self, group: BrowsersRunGroup,
                         query: str) -> pd.DataFrame:
    all_results = group.results.get_by_name(TraceProcessorProbe.NAME).csv_list
    query_result: pth.LocalPath | None = None
    for result in all_results:
      if result.stem == query:
        query_result = result
        break
    assert query_result is not None, f"{self.NAME}: {query} result not found"
    return pd.read_csv(query_result)

  @override
  def _compute_score(self, group: BrowsersRunGroup) -> pd.DataFrame:
    df = self._load_query_result(group, "loadline_benchmark_score")
    return process_scores(df)

  @override
  def _compute_breakdown(self, group: BrowsersRunGroup) -> pd.DataFrame:
    df = self._load_query_result(group, "loadline_breakdown")
    if any(df["network"] > df["process_launch"]):
      logging.warning("Some runs were affected by network latency. "
                      "Results can be non-representative.")
    return process_breakdown(df)



class LoadLine1ProbeContext(ProbeContext[LoadLine1Probe]):

  def start(self) -> None:
    pass

  @override
  def start_story_run(self) -> None:
    benchmark_type = ("loadline-phone" if "phone" in self.probe.benchmark.NAME
                      else "loadline-tablet")
    self.browser.performance_mark(
        f"LoadLine/{benchmark_type}/{self.run.story.name}", prefix="")

  def stop(self) -> None:
    pass

  def teardown(self) -> ProbeResult:
    return self.empty_result()


class LoadLine1Benchmark(LoadLineBenchmark):
  PROBES = (LoadLine1Probe,)
  DEFAULT_REPETITIONS = 100

  @classmethod
  def _base_dir(cls) -> pth.LocalPath:
    return config.config_dir() / "benchmark" / "loadline"

  @classmethod
  @override
  def default_probe_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "probe_config.hjson"


class LoadLine1PhoneBenchmark(LoadLine1Benchmark):
  """LoadLine benchmark for phones.
  """
  NAME = "loadline-phone"

  @classmethod
  @override
  def default_pages_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "page_config_phone.hjson"

  @classmethod
  @override
  def default_network_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "network_config_phone.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("loadline1-phone", "ld-phone", "ld1-phone")


class LoadLine1TabletBenchmark(LoadLine1Benchmark):
  """LoadLine benchmark for tablets.
  """
  NAME = "loadline-tablet"

  @classmethod
  @override
  def default_pages_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "page_config_tablet.hjson"

  @classmethod
  @override
  def default_network_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "network_config_tablet.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("loadline1-tablet", "ld-tablet", "ld1-tablet")

  @classmethod
  @override
  def extra_flags(cls, browser_attributes: BrowserAttributes) -> Flags:
    assert browser_attributes.is_chromium_based
    return Flags(["--request-desktop-sites"])


class LoadLine1PhoneDebugBenchmark(LoadLine1PhoneBenchmark):
  """LoadLine benchmark for phones, with more tracing categories, for easier
  performance analysis.
  """
  NAME = "loadline-phone-debug"
  DEFAULT_REPETITIONS = 1

  @classmethod
  @override
  def default_probe_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "probe_config_experimental_lightweight.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("loadline1-phone-debug", "ld-phone-debug", "ld1-phone-debug")


class LoadLine1TabletDebugBenchmark(LoadLine1TabletBenchmark):
  """LoadLine benchmark for tablets, with more tracing categories, for easier
  performance analysis.
  """
  NAME = "loadline-tablet-debug"
  DEFAULT_REPETITIONS = 1

  @classmethod
  @override
  def default_probe_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "probe_config_experimental_lightweight.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("loadline1-tablet-debug", "ld-tablet-debug", "ld1-tablet-debug")


class LoadLine1PhoneFastBenchmark(LoadLine1PhoneBenchmark):
  """LoadLine benchmark for phones, with less repetitions, for faster local
  experiments.
  """
  NAME = "loadline-phone-fast"
  DEFAULT_REPETITIONS = 10

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("loadline1-phone-fast", "ld-phone-fast", "ld1-phone-fast")


class LoadLine1TabletFastBenchmark(LoadLine1TabletBenchmark):
  """LoadLine benchmark for tablets, with less repetitions, for faster local
  experiments.
  """
  NAME = "loadline-tablet-fast"
  DEFAULT_REPETITIONS = 10

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("loadline1-tablet-fast", "ld-tablet-fast", "ld1-tablet-fast")
