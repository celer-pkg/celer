# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
from datetime import datetime as dt
import logging
from pathlib import Path
import re
import sys

from typing import List

from crossbench.parse import ObjectParser, PathParser

from runner.runner import run_benchmark, run_cuj
from runner.run_config import TargetPlatform, WebTestsRunConfig


def get_run_config_from_args(argv: List[str]) -> WebTestsRunConfig:
  # TODO this will break if cli.py is ever moved within web-tests
  web_tests_root: Path = Path(
      __file__).resolve().parent.parent.parent.parent.parent

  if not (web_tests_root / "cuj" / "crossbench").is_dir():
    logging.error(
        "web-tests does not have the expected layout. Did this file move?")
    sys.exit(1)

  parser = argparse.ArgumentParser()
  parser.add_argument("--platform", type=TargetPlatform, required=True)
  parser.add_argument(
      "--device",
      type=ObjectParser.any_str,
      default=None,
  )
  parser.add_argument(
      "--browser",
      type=ObjectParser.any_str,
      default=None,
  )
  parser.add_argument(
      "--secrets",
      type=PathParser.hjson_file_path,
      default=None,
  )
  parser.add_argument("--playback", type=ObjectParser.any_str, default=None)
  parser.add_argument("--tests", type=ObjectParser.non_empty_str, default=".*")
  parser.add_argument(
      "--variants", type=ObjectParser.non_empty_str, default=".*")
  parser.add_argument("--debug", action="store_true", default=False)
  parser.add_argument("--dry-run", action="store_true", default=False)
  parser.add_argument(
      "--results-prefix", type=ObjectParser.any_str, default=None)

  parsed = parser.parse_args(argv)

  results_root: Path = web_tests_root / "cuj/crossbench/runner/results/"
  results_prefix = f"{parsed.results_prefix}_" if parsed.results_prefix else ""
  run_results_path: Path = results_root / dt.now().strftime(
      f"{results_prefix}%Y-%m-%d_%H%M%S")
  run_results_path.mkdir(parents=True)

  latest_results: Path = results_root / "latest"
  latest_results.unlink(missing_ok=True)
  latest_results.symlink_to(run_results_path, target_is_directory=True)

  secrets_file = None
  if parsed.secrets:
    secrets_file: Path = parsed.secrets.resolve()

  return WebTestsRunConfig(
      platform=parsed.platform,
      device_id=parsed.device,
      browser=parsed.browser,
      secrets_file=secrets_file,
      playback=parsed.playback,
      tests_regex=re.compile(parsed.tests),
      variants_regex=re.compile(parsed.variants),
      results_path=run_results_path,
      web_tests_root=web_tests_root,
      debug=parsed.debug,
      dry_run=parsed.dry_run)


def runner_cli(argv: List[str]) -> None:
  logging.getLogger().setLevel(logging.INFO)

  failed_tests: List[str] = []

  run_config = get_run_config_from_args(argv)

  for benchmark_path in (run_config.web_tests_root /
                         "cuj/crossbench/benchmarks").iterdir():

    if not benchmark_path.is_dir() or not run_config.tests_regex.match(
        benchmark_path.name):
      continue

    failed_benchmarks = run_benchmark(
        benchmark_path=benchmark_path,
        run_config=run_config,
    )

    failed_tests.extend(failed_benchmarks)

  for cuj_path in (run_config.web_tests_root / "cuj/crossbench/cujs").iterdir():

    if not cuj_path.is_dir() or not run_config.tests_regex.fullmatch(
        cuj_path.name):
      continue

    failed_cujs = run_cuj(
        cuj_path=cuj_path,
        run_config=run_config,
    )

    failed_tests.extend(failed_cujs)

  if failed_tests:
    for failed_test in failed_tests:
      logging.error("Test failed: %s", failed_test)
    sys.exit(1)

  sys.exit(0)
