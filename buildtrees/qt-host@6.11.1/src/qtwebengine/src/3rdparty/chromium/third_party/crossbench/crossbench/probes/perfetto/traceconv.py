# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
import re
import sys
from typing import TYPE_CHECKING

from crossbench.parse import PathParser

if TYPE_CHECKING:
  import crossbench.path as pth
  from crossbench.config import ConfigParser
  from crossbench.plt.base import Platform
  from crossbench.plt.types import ListCmdArgs


def add_argument(parser: ConfigParser) -> None:
  parser.add_argument(
      "traceconv",
      default=None,
      type=PathParser.file_path,
      help=("Path to the 'traceconv.py' helper on the runner platform "
            "to convert '.proto' traces to legacy '.json'. "
            "If not specified, tries to find it in a v8 or chromium checkout."))


_PROTO_SUFFIX_RE = re.compile(r"\.(?:proto|pb|pb\.gz)$")


def convert_to_json(platform: Platform, traceconv: pth.LocalPath | None,
                    input_proto: pth.LocalPath) -> pth.LocalPath | None:
  if not traceconv:
    logging.info(
        "No traceconv binary: skipping converting proto to legacy traces")
    return None
  input_name = input_proto.name
  json_name = _PROTO_SUFFIX_RE.sub(".json", input_name)
  if input_name == json_name:
    raise ValueError(f"Unsupported input file: {input_proto}")
  output_json: pth.LocalPath = input_proto.with_name(json_name)
  logging.info("Converting to legacy .json trace on local machine: %s",
               output_json)
  cmd: ListCmdArgs = [traceconv, "json", input_proto, output_json]
  if not platform.is_posix:
    python_executable: ListCmdArgs = [sys.argv[0]]
    cmd = python_executable + cmd
  try:
    platform.sh(*cmd)
    return output_json
  except Exception as e:  # pylint: disable=broad-exception-caught
    logging.error("traceconv failure: %s", e)
    return None
