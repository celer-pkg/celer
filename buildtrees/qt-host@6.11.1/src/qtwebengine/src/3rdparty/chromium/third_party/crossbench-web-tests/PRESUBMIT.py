#!/usr/bin/env python3
# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from pathlib import Path
import platform
import subprocess
from typing import List


USE_PYTHON3 = True


def CheckChange(input_api, output_api):
  tests = []
  results = []
  # ---------------------------------------------------------------------------
  # Validate the vpython spec:
  # ---------------------------------------------------------------------------
  if platform.system() in ("Linux", "Darwin"):
    tests += input_api.canned_checks.CheckVPythonSpec(input_api, output_api)

  # ---------------------------------------------------------------------------
  # License header checks:
  # ---------------------------------------------------------------------------
  files_to_check = list(input_api.DEFAULT_FILES_TO_CHECK) + [
      r".+\.hjson$",
      r".+\.sql$",
  ]

  results += input_api.canned_checks.CheckLicense(
      input_api,
      output_api,
      source_file_filter=lambda x: input_api.FilterSourceFile(
          x, files_to_check=files_to_check))

  # ---------------------------------------------------------------------------
  # hjson:
  # ---------------------------------------------------------------------------
  for hjson_file in AllHjsonFiles(input_api):
    try:
      formatted_contents: str = FormatHjsonFile(input_api, hjson_file)
    except ValueError as e:
      results.append(
          output_api.PresubmitPromptWarning(
              "Malformed hjson file:",
              items=[str(hjson_file)],
              long_text=str(e)))
      continue

    original_contents = input_api.ReadFile(str(hjson_file), "r")

    if original_contents != formatted_contents:
      results.append(
          output_api.PresubmitPromptWarning(
              "Unformatted hjson file:",
              items=[str(hjson_file)],
              long_text=(
                  "Run format_hjson.py to automatically fix this error.\n"
                  f"Expected:\n{formatted_contents}"
                  f"Got:\n{original_contents}")))

  # ---------------------------------------------------------------------------
  # crossbench:
  # ---------------------------------------------------------------------------
  dry_run_py_path = str(
      Path(input_api.change.RepositoryRoot()) / "cuj" / "crossbench" /
      "runner" / "run.py")
  tests.append(
      input_api.Command(
          name="crossbench dry run",
          cmd=[
              input_api.python3_executable, dry_run_py_path, "--platform=local",
              "--dry-run"
          ],
          message=output_api.PresubmitError,
          kwargs={},
          python3=True,
      ))

  # ---------------------------------------------------------------------------
  # Pylint:
  # ---------------------------------------------------------------------------
  tests += input_api.canned_checks.GetPylint(
      input_api,
      output_api,
      files_to_check=[r"^[^\.]+\.py$"],
      pylintrc=".pylintrc",
      version="3.2")

  # ---------------------------------------------------------------------------
  # Run all test
  # ---------------------------------------------------------------------------
  results += input_api.RunTests(tests)
  return results


def AllHjsonFiles(input_api) -> List[Path]:
  hjson_files = []
  for file in input_api.change.AllFiles():
    if file.endswith(".hjson"):
      hjson_files.append(Path(input_api.change.RepositoryRoot()) / file)

  return hjson_files


def FormatHjsonFile(input_api, hjson_file: Path) -> str:
  node_bin = str(
      Path(input_api.change.RepositoryRoot()) /
      "third_party/node/linux/node-linux-x64/bin/node")
  hjson_js_bin = str(
      Path(input_api.change.RepositoryRoot()) /
      "third_party/hjson_js/bin/hjson")

  try:
    return subprocess.run([
        node_bin, hjson_js_bin, "-rt", "-sl", "-nocol", "-cond=0",
        str(hjson_file)
    ],
                          check=True,
                          capture_output=True).stdout.decode(encoding="utf-8")
  except subprocess.CalledProcessError as e:
    error = e.stderr.decode(encoding="utf=8")
    raise ValueError(f"Failed to parse hjson file: {error}") from e


def CheckChangeOnUpload(input_api, output_api):
  return CheckChange(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return CheckChange(input_api, output_api)
