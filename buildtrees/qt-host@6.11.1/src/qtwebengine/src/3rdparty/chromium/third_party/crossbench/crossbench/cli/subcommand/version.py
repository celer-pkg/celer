# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import sys
from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench import __version__
from crossbench.cli.subcommand.base import CrossbenchSubcommand

if TYPE_CHECKING:
  import argparse


class VersionSubcommand(CrossbenchSubcommand):

  @override
  def add_cli_parser(self) -> argparse.ArgumentParser:
    version_parser = self.cli.subparsers.add_parser(
        "version",
        help="Show program's version number and exit, same as --version")
    return version_parser

  @override
  def run(self, args: argparse.Namespace) -> None:
    print(f"{sys.argv[0]} {__version__}")
    sys.exit(0)
