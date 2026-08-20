#!/usr/bin/env python3
# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
from pathlib import Path
import subprocess
import sys


def format_hjson() -> None:
  logging.getLogger().setLevel(logging.INFO)

  logging.warning("This script will format *every* hjson file in web-tests "
                  "and does not scope changes to the current commit.")
  do_format = input("To continue, type 'CONTINUE'\n")
  if do_format != "CONTINUE":
    sys.exit()

  web_tests_root = Path(__file__).resolve().parent

  for hjson_file in web_tests_root.glob("cuj/**/*.hjson"):
    node_bin = str(web_tests_root /
                   "third_party/node/linux/node-linux-x64/bin/node")
    hjson_js_bin = str(web_tests_root / "third_party/hjson_js/bin/hjson")

    try:
      formatted_file = subprocess.run(
          [
              node_bin, hjson_js_bin, "-rt", "-sl", "-nocol", "-cond=0",
              str(hjson_file)
          ],
          check=True,
          capture_output=True).stdout.decode(encoding="utf-8")
      hjson_file.write_text(formatted_file, encoding="utf-8")
    except subprocess.CalledProcessError as e:
      error = e.stderr.decode(encoding="utf=8")
      logging.error("Failed to parse hjson file (%s): %s", str(hjson_file),
                    error)
      continue


if __name__ == "__main__":
  format_hjson()
