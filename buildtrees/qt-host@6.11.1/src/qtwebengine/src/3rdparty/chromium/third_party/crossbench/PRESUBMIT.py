#!/usr/bin/env python3
# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import pathlib
import platform
import re
import subprocess
from typing import Iterable, Optional

USE_PYTHON3 = True

SOURCE_SKIP_RE = [r"^protoc/gen.*", r"^third_party/.*"]

def SourceFileFilter(input_api):
  """Returns filter that selects source code files only."""
  files_to_skip = list(input_api.DEFAULT_FILES_TO_SKIP) + SOURCE_SKIP_RE
  files_to_check = list(input_api.DEFAULT_FILES_TO_CHECK)
  return lambda x: input_api.FilterSourceFile(
      x, files_to_check=files_to_check, files_to_skip=files_to_skip)


def GlobalSkipChecks(input_api, file_path: str):
  if input_api.fnmatch.fnmatch(file_path, "*protoc/gen/*"):
    return True
  if input_api.fnmatch.fnmatch(file_path, "*third_party/*"):
    return True
  return False


def CheckChange(input_api, output_api, on_commit):
  tests = []
  results = []
  testing_env = dict(input_api.environ)
  root_path = pathlib.Path(input_api.PresubmitLocalPath())
  crossbench_test_path = root_path / "tests" / "crossbench"
  testing_env["PYTHONPATH"] = input_api.os_path.pathsep.join(
      map(str, [root_path, crossbench_test_path]))
  # ---------------------------------------------------------------------------
  source_file_filter = SourceFileFilter(input_api)
  modified_py_files: list[str] | None = ModifiedFiles(input_api, on_commit)
  modified_hjson_files: list[str] | None = ModifiedFiles(
      input_api, False, filename_pattern="*.hjson")

  # ---------------------------------------------------------------------------
  # Validate the vpython spec:
  # ---------------------------------------------------------------------------
  if platform.system() in ("Linux", "Darwin"):
    tests += input_api.canned_checks.CheckVPythonSpec(input_api, output_api)

  # ---------------------------------------------------------------------------
  # License header checks:
  # ---------------------------------------------------------------------------
  results += input_api.canned_checks.CheckLicense(
      input_api, output_api,
      source_file_filter=source_file_filter)

  # ---------------------------------------------------------------------------
  # Pylint:
  # ---------------------------------------------------------------------------
  pylint_file_patterns_to_check: list[str] = PylintFilePatternsToCheck(
      on_commit, modified_py_files)
  if pylint_file_patterns_to_check:
    tests += input_api.canned_checks.GetPylint(
        input_api,
        output_api,
        files_to_check=pylint_file_patterns_to_check,
        files_to_skip=SOURCE_SKIP_RE,
        pylintrc=".pylintrc",
        version="3.2")

  # ---------------------------------------------------------------------------
  # MyPy:
  # ---------------------------------------------------------------------------
  mypy_files_to_check: list[str] = MypyFilesToCheck(input_api, on_commit,
                                                  modified_py_files)
  if mypy_files_to_check:
    tests.append(
        input_api.Command(
            name="mypy",
            cmd=[
                input_api.python3_executable,
                "-m",
                "mypy",
                "--check-untyped-defs",
                "--pretty",
            ] + mypy_files_to_check,
            message=output_api.PresubmitError,
            kwargs={},
            python3=True,
        ))

  # ---------------------------------------------------------------------------
  # isort:
  # ---------------------------------------------------------------------------
  SortImports(input_api, output_api, results, modified_py_files)

  # ---------------------------------------------------------------------------
  # hjson:
  # ---------------------------------------------------------------------------
  FormatHjsonFiles(input_api, output_api, results, modified_hjson_files)

  # ---------------------------------------------------------------------------
  # Unittest:
  # ---------------------------------------------------------------------------
  test_dirs_to_check, test_file_patterns_to_check = TestFilePatternsToCheck(
      on_commit, crossbench_test_path)
  for test_dir_to_check in test_dirs_to_check:
    # Skip potentially empty dirs
    if test_dir_to_check.name == "__pycache__":
      continue
    # End-to-end tests require custom setup and are not suited for presubmits.
    if "end2end" in test_dir_to_check.parts:
      continue
    tests += input_api.canned_checks.GetUnitTestsInDirectory(
        input_api,
        output_api,
        directory=test_dir_to_check,
        env=testing_env,
        files_to_check=test_file_patterns_to_check,
        skip_shebang_check=True,
        run_on_python2=False)

  # ---------------------------------------------------------------------------
  # Run all test
  # ---------------------------------------------------------------------------
  results += input_api.RunTests(tests)
  return results


def SortImports(input_api, output_api, results, modified_py_files):
  for py_file in (modified_py_files or []):
    if GlobalSkipChecks(input_api, py_file):
      continue
    full_py_path = pathlib.Path(
      input_api.change.RepositoryRoot()) / py_file
    original_contents = input_api.ReadFile(str(full_py_path), "r")
    subprocess.run([
              input_api.python_executable,
              "-m", "isort",
              full_py_path], check=True)
    formatted_contents = input_api.ReadFile(str(full_py_path), "r")
    if original_contents != formatted_contents:
      results.append(
        output_api.PresubmitPromptWarning(
            "Unsorted python imports in file:",
            items=[str(full_py_path)],
            long_text="Please update your commit with the formatted file."))


def FormatHjsonFiles(input_api, output_api, results, modified_hjson_files):
  for hjson_file in (modified_hjson_files or []):
    full_hjson_path = pathlib.Path(
      input_api.change.RepositoryRoot()) / hjson_file

    try:
      formatted_contents: str = FormatHjsonFile(input_api, full_hjson_path)
    except ValueError as e:
      results.append(
        output_api.PresubmitPromptWarning(
            "Malformed hjson file:",
            items=[str(full_hjson_path)],
            long_text=str(e)))
      continue

    original_contents = input_api.ReadFile(str(full_hjson_path), "r")
    if original_contents != formatted_contents:
      full_hjson_path.write_text(formatted_contents)
      results.append(
        output_api.PresubmitPromptWarning(
            "Unformatted hjson file:",
            items=[str(full_hjson_path)],
            long_text="Please update your commit with the formatted file."))


def ModifiedFiles(input_api,
                  on_commit: bool,
                  filename_pattern="*.py") -> Optional[list[str]]:
  if on_commit:
    return None
  files = [file.AbsoluteLocalPath() for file in input_api.AffectedFiles()]
  files_to_check = []
  for file_path in files:
    if not input_api.fnmatch.fnmatch(file_path, filename_pattern):
      continue
    if GlobalSkipChecks(input_api, file_path):
      continue
    if not input_api.os_path.exists(file_path):
      continue
    file_path = input_api.os_path.relpath(file_path,
                                          input_api.PresubmitLocalPath())
    files_to_check.append(file_path)
  return files_to_check


def PylintFilePatternsToCheck(on_commit, modified_py_files) -> list[str]:
  if on_commit:
    # Test all files on commit
    return [r"^[^\.]+\.py$"]
  # By default, the pylint canned check lints all Python files together to
  # check for potential problems between dependencies. This is slow to run
  # across all of crossbench (>2 min), so only lint affected files.
  return [re.escape(file) for file in modified_py_files]


def MypyFilesToCheck(input_api, on_commit, modified_py_files) -> list[str]:
  root_path = pathlib.Path(input_api.PresubmitLocalPath())
  mypy_files_to_check = {"PRESUBMIT.py"}
  crossbench_path = root_path / "crossbench"
  if on_commit:
    mypy_files_to_check.add(str(crossbench_path))
  else:
    mypy_files_to_check.update(modified_py_files)
  # TODO: enable mypy on all tests
  result = []
  for file in mypy_files_to_check:
    if file.startswith("tests/"):
      continue
    if GlobalSkipChecks(input_api, file):
      continue
    result.append(file)
  return result


def GetNodeExecutable(input_api) -> str:
  node_base: pathlib.Path = pathlib.Path(
      input_api.change.RepositoryRoot()) / "third_party" / "node"

  node_bin = ""

  if input_api.platform == "linux":
    node_bin = str(node_base / "linux" / "node-linux-x64" / "bin" / "node")
  if input_api.platform == "win32":
    node_bin = str(node_base / "win" / "node.exe")
  if input_api.platform == "darwin":
    if platform.machine() == "arm64":
      node_bin = str(node_base / "mac_arm64" / "node-darwin-arm64" / "bin" /
                     "node")
    else:
      node_bin = str(node_base / "mac" / "node-darwin-x64" / "bin" / "node")

  if not node_bin:
    raise NotImplementedError(f"{input_api.platform} {platform.machine()} "
                              "is not a supported platform.")

  return node_bin


def FormatHjsonFile(input_api, hjson_file: pathlib.Path) -> str:
  node_bin = GetNodeExecutable(input_api)

  hjson_js_bin = str(
      pathlib.Path(input_api.change.RepositoryRoot()) / "third_party" /
      "hjson_js" / "bin" / "hjson")

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


def TestFilePatternsToCheck(on_commit, crossbench_test_path):
  # Only run test_cli to speed up the presubmit checks
  if on_commit:
    test_dirs_to_check: Iterable[pathlib.Path] = crossbench_test_path.glob("**")
    test_files_to_check = [r".*test_.*\.py$"]
  else:
    # Only check a small subset on upload
    test_dirs_to_check = [crossbench_test_path / "cli"]
    test_files_to_check = [r".*test_cli_fast_.*\.py$"]
  return test_dirs_to_check, test_files_to_check


def CheckChangeOnUpload(input_api, output_api):
  return CheckChange(input_api, output_api, on_commit=False)


def CheckChangeOnCommit(input_api, output_api):
  return CheckChange(input_api, output_api, on_commit=True)
