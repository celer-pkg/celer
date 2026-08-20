# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import TYPE_CHECKING

if TYPE_CHECKING:
  import argparse

  from crossbench.cli.cli import CrossBenchCLI


class CrossbenchSubcommand(abc.ABC):

  def __init__(self, cli: CrossBenchCLI) -> None:
    self._cli = cli
    self._parser: argparse.ArgumentParser = self.add_cli_parser()
    self._parser.set_defaults(crossbench_subcommand=self)

  @property
  def cli(self) -> CrossBenchCLI:
    return self._cli

  @property
  def parser(self) -> argparse.ArgumentParser:
    return self._parser

  @abc.abstractmethod
  def add_cli_parser(self) -> argparse.ArgumentParser:
    pass

  @abc.abstractmethod
  def run(self, args: argparse.Namespace) -> None:
    pass

  def error(self, message: str) -> None:
    self.cli.error(message)

  def fail(self, message: str) -> None:
    self._parser.error(message)
