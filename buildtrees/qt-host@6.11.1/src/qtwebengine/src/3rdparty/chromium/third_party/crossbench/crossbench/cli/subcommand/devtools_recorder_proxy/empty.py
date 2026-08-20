# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
  import argparse


class CrossbenchDevToolsRecorderProxy:
  """Empty dummy implementation that can be used in case async is not
  supported."""

  @classmethod
  def add_cli_parser(cls, subparsers) -> argparse.ArgumentParser:
    return subparsers.add_parser(
        "devtools-recorder-proxy",
        aliases=["devtools"],
        help="Unsupported operation")

  @classmethod
  def run_subcommand(cls, args: argparse.Namespace) -> None:
    raise NotImplementedError()
