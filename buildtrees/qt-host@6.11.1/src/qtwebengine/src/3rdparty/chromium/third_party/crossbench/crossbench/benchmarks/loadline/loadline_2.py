# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Sequence, Type

import numpy as np
import pandas as pd
from typing_extensions import override

from crossbench import config
from crossbench import path as pth
from crossbench.benchmarks.loading.page.combined import CombinedPage
from crossbench.benchmarks.loadline.loadline import (LoadLineBenchmark,
                                                     LoadLineProbe)
from crossbench.flags.base import Flags
from crossbench.probes.perfetto.trace_processor.trace_processor import \
    TraceProcessorProbe
from crossbench.probes.probe_context import ProbeContext

if TYPE_CHECKING:
  import argparse

  from crossbench.benchmarks.loading.page.base import Page
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.groups.browsers import BrowsersRunGroup


# We should increase the minor version number every time there are any changes
# that might affect the benchmark score.
VERSION_STRING = "experimental"


class LoadLine2Probe(LoadLineProbe):
  NAME = "loadline2_probe"
  BENCHMARK_NAME = "LoadLine2"
  BENCHMARK_VERSION = VERSION_STRING

  @override
  def get_context_cls(self,) -> Type[LoadLine2ProbeContext]:
    return LoadLine2ProbeContext

  @override
  def _compute_score(self, group: BrowsersRunGroup) -> pd.DataFrame:
    all_results = group.results.get_by_name(TraceProcessorProbe.NAME).csv_list
    loadline2_result: pth.LocalPath | None = None
    for result in all_results:
      if result.name.startswith("loadline2_benchmark_score"):
        loadline2_result = result
        break
    assert loadline2_result is not None, "LoadLine 2: query result not found"

    df = pd.read_csv(loadline2_result)
    total = df.drop(columns=["cb_story", "cb_temperature", "cb_run"]).groupby(
        ["cb_browser"]).mean()
    total["TOTAL_SCORE"] = np.exp(np.log(total).mean(axis=1))
    total.index.rename("browser", inplace=True)
    return total.reindex(
        columns=(["TOTAL_SCORE"] +
                 sorted(list(c for c in total.columns if c != "TOTAL_SCORE"))))

  @override
  def _compute_breakdown(self, group: BrowsersRunGroup) -> pd.DataFrame:
    # TODO(crbug.com/425325733): Implement breakdown for LoadLine 2.
    return pd.DataFrame(index=pd.Index([], name="Not implemented"))


class LoadLine2ProbeContext(ProbeContext[LoadLine2Probe]):

  def start(self) -> None:
    pass

  def stop(self) -> None:
    pass

  def teardown(self) -> ProbeResult:
    return self.empty_result()


class LoadLine2Benchmark(LoadLineBenchmark):
  PROBES = (LoadLine2Probe,)
  DEFAULT_REPETITIONS = 100

  @classmethod
  def _base_dir(cls) -> pth.LocalPath:
    return config.config_dir() / "benchmark" / "loadline2"

  @classmethod
  @override
  def stories_from_cli_args(cls, args: argparse.Namespace) -> Sequence[Page]:
    pages = super().stories_from_cli_args(args)
    return (CombinedPage(pages),)


class LoadLine2PhoneBenchmark(LoadLine2Benchmark):
  """LoadLine 2 benchmark for phones.
  """
  NAME = "loadline2-phone"

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
  def default_probe_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "probe_config_phone.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("ld2-phone",)


class LoadLine2TabletBenchmark(LoadLine2Benchmark):
  """LoadLine 2 benchmark for tablets.
  """
  NAME = "loadline2-tablet"

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
  def default_probe_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "probe_config_tablet.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("ld2-tablet",)

  @classmethod
  @override
  def extra_flags(cls, browser_attributes: BrowserAttributes) -> Flags:
    assert browser_attributes.is_chromium_based
    return Flags(["--request-desktop-sites"])


class LoadLine2PhoneDebugBenchmark(LoadLine2PhoneBenchmark):
  """LoadLine 2 benchmark for phones, with more tracing categories, for easier
  performance analysis.
  """
  NAME = "loadline2-phone-debug"
  DEFAULT_REPETITIONS = 1

  @classmethod
  @override
  def default_probe_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "probe_config_phone_debug.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("ld2-phone-debug",)


class LoadLine2TabletDebugBenchmark(LoadLine2TabletBenchmark):
  """LoadLine 2 benchmark for tablets, with more tracing categories, for easier
  performance analysis.
  """
  NAME = "loadline2-tablet-debug"
  DEFAULT_REPETITIONS = 1

  @classmethod
  @override
  def default_probe_config_path(cls) -> pth.LocalPath:
    return cls._base_dir() / "probe_config_tablet_debug.hjson"

  @classmethod
  @override
  def aliases(cls) -> tuple[str, ...]:
    return ("ld2-tablet-debug",)
