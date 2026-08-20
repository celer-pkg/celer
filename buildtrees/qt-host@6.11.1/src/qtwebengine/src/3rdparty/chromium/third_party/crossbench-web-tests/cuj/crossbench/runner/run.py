#!/usr/bin/env vpython3
# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

from pathlib import Path

# This is the earliest entrypoint into the runner.
# Try to import some simple crossbench package here to
# check that crossbench is setup properly.
try:
  # pylint: disable=unused-import
  from crossbench.types import Json
except ImportError:
  # Manually add crossbench to the path.
  # This is necessary when running under vpython within web-tests
  # (such as when running presubmit).
  web_tests_root = Path(__file__).resolve().parent.parent.parent.parent
  sys.path.append(str(web_tests_root / "third_party" / "crossbench"))

from runner.cli import runner_cli

if __name__ == "__main__":
  argv = sys.argv
  runner_cli(argv[1:])
