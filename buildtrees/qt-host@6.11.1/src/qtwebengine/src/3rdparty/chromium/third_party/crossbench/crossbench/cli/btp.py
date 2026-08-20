#!/usr/bin/env vpython3
# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import logging
from typing import Sequence

from perfetto.batch_trace_processor.api import (BatchTraceProcessor,
                                                BatchTraceProcessorConfig,
                                                FailureHandling)
from perfetto.trace_processor.api import TraceProcessorConfig
from perfetto.trace_uri_resolver.resolver import TraceUriResolver

from crossbench import path as pth
from crossbench.cli.config.probe_list import ProbeListConfig
from crossbench.cli.parser import CrossBenchArgumentParser
from crossbench.parse import PathParser
from crossbench.probes.perfetto.trace_processor.trace_processor import (
    _MODULES_DIR, _QUERIES_DIR, TraceProcessorProbe)

ROOT_DIR = pth.LocalPath(__file__).parents[2]
DEFAULT_RESULT_DIR = ROOT_DIR / "results" / "latest"
DEFAULT_CONFIG_PATH = (
    ROOT_DIR / "config" / "benchmark" / "loadline" / "probe_config.hjson")

class MergedTraceUriResolver(TraceUriResolver):

  def __init__(self, result_path: pth.LocalPath) -> None:

    def metadata(path) -> dict[str, str]:
      parts = str(path).split("/")
      return {
          "cb_browser": parts[-7],
          "cb_story": parts[-5],
          "cb_run": parts[-4],
          "cb_temperature": parts[-3],
      }

    listdir = list(result_path.glob("*/stories/**/merged_trace.zip"))

    self._resolved = [
        TraceUriResolver.Result(trace=str(path), metadata=metadata(path))
        for path in listdir]

  def resolve(self):
    return self._resolved

class BTPUtil:

  def __init__(self) -> None:
    self.parser = CrossBenchArgumentParser(
      description=("Runs trace processor queries in a batch mode on existing "
                   "benchmark results, without re-running the benchmark "
                   "itself."),
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    self.parser.add_argument(
        "--result-dir",
        type=PathParser.existing_path,
        default=DEFAULT_RESULT_DIR,
        help="Path to the benchmark result directory.")
    self.parser.add_argument(
        "--probe-config",
        type=PathParser.existing_file_path,
        default=DEFAULT_CONFIG_PATH,
        help="Path to the trace_processor probe config.")
    self.parser.add_argument(
        "--output-dir",
        type=pth.LocalPath,
        default=ROOT_DIR,
        help="Path to the directory where output files will be placed.")
    self.parser.add_argument(
        "--extra-query",
        type=str,
        default=[],
        action="append",
        help=("Name of the query to compute (the query must be present in the "
              "trace_processor/queries/ dir). Repeat for multiple queries."))

  def run(self, argv: Sequence[str]) -> None:
    args = self.parser.parse_args(argv)

    probe_config = ProbeListConfig.parse(args.probe_config)
    tp: TraceProcessorProbe | None = None
    for probe in probe_config.probes:
      if isinstance(probe, TraceProcessorProbe):
        tp = probe
    assert tp is not None

    tp_config = TraceProcessorConfig(
        bin_path=str(tp.trace_processor_bin),
        extra_flags=["--add-sql-module", _MODULES_DIR])
    btp_conf = BatchTraceProcessorConfig(
      tp_config=tp_config,
      load_failure_handling=FailureHandling.INCREMENT_STAT,
      execute_failure_handling=FailureHandling.INCREMENT_STAT)

    with BatchTraceProcessor(
      traces=MergedTraceUriResolver(args.result_dir), config=btp_conf) as btp:
      queries = list(tp.queries) + args.extra_query
      for query in queries:
        query_path = _QUERIES_DIR / f"{query}.sql"
        csv_file = args.output_dir / f"{pth.safe_filename(query)}.csv"
        btp.query_and_flatten(query_path.read_text()).to_csv(
          path_or_buf=csv_file, index=False)
        print(f"Query result written into {csv_file}")

    stats = btp.stats()
    if stats.load_failures + stats.execute_failures > 0:
      logging.error("Failures registered during BTP run: "
                  "%s load failures, %s execution failures.",
                  stats.load_failures, stats.execute_failures)
