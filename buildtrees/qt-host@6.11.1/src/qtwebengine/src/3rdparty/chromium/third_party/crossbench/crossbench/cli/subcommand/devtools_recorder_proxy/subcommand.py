# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.cli.subcommand.base import CrossbenchSubcommand
from crossbench.cli.subcommand.devtools_recorder_proxy.default import \
    CrossbenchDevToolsRecorderProxy

if TYPE_CHECKING:
  import argparse


class DevtoolsRecorderProxySubcommand(CrossbenchSubcommand):

  @override
  def add_cli_parser(self) -> argparse.ArgumentParser:
    parser = CrossbenchDevToolsRecorderProxy.add_cli_parser(self.cli.subparsers)
    return parser

  @override
  def run(self, args: argparse.Namespace) -> None:
    CrossbenchDevToolsRecorderProxy.run_subcommand(args)
