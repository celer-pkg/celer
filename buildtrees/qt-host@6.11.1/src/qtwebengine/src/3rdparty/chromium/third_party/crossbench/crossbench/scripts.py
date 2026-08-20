# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Optional

import hjson

from crossbench.cli.btp import BTPUtil
from crossbench.cli.cli import CrossBenchCLI


def crossbench(argv: Optional[list[str]] = None) -> None:
  if not argv:
    argv = sys.argv
  cli = CrossBenchCLI()
  cli.run(argv[1:])


def cb_btp(argv: Optional[list[str]] = None) -> None:
  if not argv:
    argv = sys.argv
  btp = BTPUtil()
  btp.run(argv[1:])


def cb_validate_hjson(argv: Optional[list[str]] = None) -> None:
  if not argv:
    argv = sys.argv
  for path_str in argv[1:]:
    path = Path(path_str)
    with path.open(encoding="utf-8") as f:
      match path.suffix:
        case ".json":
          json.load(f)
        case ".hjson":
          hjson.load(f)
        case _:
          raise ValueError(f"Unsupported file format {path}")
