# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import sys
from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.cli.subcommand.base import CrossbenchSubcommand

if TYPE_CHECKING:
  import argparse


class HelpSubcommand(CrossbenchSubcommand):

  @override
  def add_cli_parser(self) -> argparse.ArgumentParser:
    # Just for completeness we want to support "--help" and "help"
    help_parser = self.cli.subparsers.add_parser(
        "help",
        help=("Print the top-level by default, same as --help. "
              "Use `help $PROBE`, or `help $BENCHMARK` to print more details."))
    help_parser.add_argument(
        "search_terms",
        nargs="*",
        help="Use a benchmark, probe or network name to display more details.")
    return help_parser

  @override
  def run(self, args: argparse.Namespace) -> None:
    if args.search_terms:
      self.cli.describe_subcommand.run_from_help(args)
    else:
      self.cli.parser.print_help()
    sys.exit(0)
