#!vpython3
# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import dataclasses
import importlib
import importlib.metadata
import logging
import os
import re
import shutil
import subprocess
import sys
from contextlib import contextmanager
from pathlib import Path
from typing import Final, Sequence

# List of modules we want to depend on. We will protoc this module and all its
# dependencies.
PROTO_MODULES = [
    "frameworks.base.core.proto.android.server.activitymanagerservice_pb2",
    "frameworks.base.core.proto.android.server.windowmanagerservice_pb2",
    "frameworks.base.core.proto.android.service.battery_pb2",
    "frameworks.proto_logging.stats.enums.os.enums_pb2",
    "protos.perfetto.config.trace_config_pb2",
    "protos.perfetto.trace_summary.file_pb2",
]


@dataclasses.dataclass(frozen=True)
class RepoConfig:
  name: str
  url: str
  sparse_dirs: Sequence[str] = tuple()
  proto_dir: str = ""

  def checkout(self, repos_dir: Path) -> None:
    repo_dir = repos_dir / self.name
    repos_exists = repo_dir.exists()
    repo_dir.mkdir(parents=True, exist_ok=True)
    with change_cwd(repo_dir):
      if not repos_exists:
        sh("git", "init")
        sh("git", "remote", "add", "origin", self.url)
        if self.sparse_dirs:
          sh("git", "config", "core.sparsecheckout", "true")
          with (repo_dir / ".git" / "info" / "sparse-checkout").open(
              "a", encoding="utf8") as sparse_file:
            sparse_file.writelines(self.sparse_dirs)
      sh("git", "fetch", "--filter=blob:none", "--depth", "1", "origin", "main")
      sh("git", "checkout", "main")

  def symlink_proto_src_dir(self, repo_dir: Path, protos_dir: Path) -> None:
    # Make sure the src folder path matches the proto import path.
    # This way we only need a single --proto_path argument for protoc
    target_dir = repo_dir / self.name / self.proto_dir
    proto_dir = protos_dir / self.name
    proto_dir.parent.mkdir(parents=True, exist_ok=True)
    logging.debug(proto_dir, "=>", target_dir)
    proto_dir.symlink_to(target_dir, target_is_directory=True)


ANDROID_FRAMEWORKS_GIT_URL = "https://android.googlesource.com/platform/frameworks/"
REPOS: Sequence[RepoConfig] = (
    RepoConfig("frameworks/base", f"{ANDROID_FRAMEWORKS_GIT_URL}/base",
               ("core/proto",)),
    RepoConfig("frameworks/proto_logging",
               f"{ANDROID_FRAMEWORKS_GIT_URL}/proto_logging"),
    RepoConfig("protos/perfetto", "https://github.com/google/perfetto.git",
               ("protos/perfetto",), "protos/perfetto"))


@contextmanager
def change_cwd(cwd):
  previous_cwd = os.getcwd()
  try:
    os.chdir(str(cwd))
    yield
  finally:
    os.chdir(previous_cwd)


def sh(*args):
  logging.debug("Running: %s", " ".join(map(str, args)))
  subprocess.run(args, check=True)


def compile_proto(protoc_bin: str, protos_dir: Path, proto_file: Path) -> None:
  # TODO: check that the proto file is inside one of the repos.
  logging.info("Compiling %s", str(proto_file))
  sh(protoc_bin, "--python_out=.", f"--proto_path={protos_dir}", proto_file)


# Patterns to parse import errors when a module has not been compiled yet.
MISSING_MODULE_RE = re.compile("^No module named '([^']*)'")
MISSING_FILE_RE = re.compile("^cannot import name '([^']*)_pb2' from '([^']*)'")


def compile_imports(out_dir: Path, protos_dir: Path, protoc_bin: str,
                    module_name: str) -> None:
  while True:
    try:
      importlib.invalidate_caches()
      importlib.import_module(module_name)
      break
    except ImportError as err:
      compile_missing_import(out_dir, protos_dir, protoc_bin, err)


def compile_missing_import(out_dir: Path, protos_dir: Path, protoc_bin: str,
                           err: ImportError) -> None:
  err_str = str(err)
  if module_missing := MISSING_MODULE_RE.match(err_str):
    compile_missing_module(out_dir, protos_dir, protoc_bin, module_missing[1])
    return

  if file_missing := MISSING_FILE_RE.match(err_str):
    compile_missing_file(protos_dir, protoc_bin, file_missing)
    return
  raise err


PB2_SUFFIX: Final[str] = "_pb2"


def compile_missing_module(out_dir: Path, protos_dir: Path, protoc_bin: str,
                           module_name: str) -> None:
  maybe_module_dir = Path(module_name.replace(".", "/"))
  logging.debug(str(maybe_module_dir))
  if maybe_module_dir.name.endswith(PB2_SUFFIX):
    file_name = f"{maybe_module_dir.name[:-len(PB2_SUFFIX)]}.proto"
    compile_proto(protoc_bin, protos_dir, maybe_module_dir.with_name(file_name))
    return
  logging.debug("Making folder %s", repr(maybe_module_dir))
  dir_path: Path = out_dir / maybe_module_dir
  dir_path.mkdir()
  # this folder is a module
  (dir_path / "__init__.py").touch(exist_ok=True)


def compile_missing_file(protos_dir: Path, protoc_bin: str,
                         file_missing: re.Match) -> None:
  proto_module_dir = Path(file_missing[2].replace(".", "/"))
  proto_file = proto_module_dir / f"{file_missing[1]}.proto"
  compile_proto(protoc_bin, protos_dir, proto_file)


ROOT_MODULE_PROLOGUE: Final[str] = '''# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated by protoc/compile.py
from protoc.sys_path import protoc_in_sys_path

with protoc_in_sys_path():
'''


def write_root_module(root_dir: Path) -> None:
  with (root_dir / "__init__.py").open("w", encoding="utf8") as root_module:
    root_module.write(ROOT_MODULE_PROLOGUE)
    for module_name in PROTO_MODULES:
      module_parts = module_name.split(".")
      from_name = ".".join(module_parts[0:-1])
      import_name = module_parts[-1]
      root_module.write(f"  from {from_name} import {import_name}\n")


VERSION_RE = re.compile(r".* (\d+)\.(\d+)$")


def check_protoc_version(protoc_bin: str) -> None:
  # Parse version "29" from "libprotoc 29.3":
  protoc_version_str = subprocess.check_output((protoc_bin, "--version"),
                                               text=True)
  version_match = VERSION_RE.match(protoc_version_str)
  if not version_match:
    raise ValueError(
        f"Invalid protoc version string: {repr(protoc_version_str)}")
  protoc_version = version_match[1]
  # parse version "29" from "5.29.3":
  pkg_version_str = importlib.metadata.version("protobuf")
  pkg_version = pkg_version_str.split(".")[1]
  if protoc_version != pkg_version:
    logging.debug("protoc version: %s, protobuf module version: %s",
                  protoc_version, pkg_version)
    raise ValueError(
        f"protoc={repr(protoc_version_str)} and "
        f"protobuf={repr(pkg_version)} versions must match.")


ROOT_DIR = Path(__file__).parent.resolve()
OUT_DIR = ROOT_DIR / "gen"
TMP_DIR = ROOT_DIR / ".tmp"
# Contains the temporarily checked out repositories.
REPOS_DIR = TMP_DIR / "repos"
# Contains symlink to proto modules inside the repositories.
PROTOS_DIR = TMP_DIR / "protos"

HELP = """
Script to generate Android proto parsing files.

Does a sparse checkout of the required Android repos, and calls protoc on the
minimum required proto files to have working implementations of the types
listed in PROTO_MODULES below.

Also generates __init__.py that exports the types listed in PROTO_MODULES, for
easier importing into Crossbench.
"""


def main():
  parser = argparse.ArgumentParser(description=HELP)
  parser.add_argument("--version", action="version", version="%(prog)s 1.0")
  parser.add_argument(
      "--protoc", default="protoc", help="Override default protoc version")
  parser.add_argument(
      "-v", "--verbose", action="store_true", help="Enable verbose logging")
  args = parser.parse_args()

  if args.verbose:
    logging.basicConfig(level=logging.DEBUG)
  else:
    logging.basicConfig(level=logging.INFO)

  protoc_bin = args.protoc

  check_protoc_version(protoc_bin)

  if TMP_DIR.exists():
    shutil.rmtree(TMP_DIR)
  if OUT_DIR.exists():
    shutil.rmtree(OUT_DIR)
  OUT_DIR.mkdir(parents=True, exist_ok=True)
  REPOS_DIR.mkdir(parents=True, exist_ok=True)
  PROTOS_DIR.mkdir(parents=True, exist_ok=True)

  # Check out a fresh copy of all repos.
  for repo in REPOS:
    repo.checkout(REPOS_DIR)
    repo.symlink_proto_src_dir(REPOS_DIR, PROTOS_DIR)

  # Put python_out dir in the python search path, so that once we protoc a file,
  # python will find it.
  sys.path.insert(0, str(OUT_DIR))

  with change_cwd(OUT_DIR):
    for module in PROTO_MODULES:
      logging.debug("Compiling all dependencies for %s", module)
      compile_imports(OUT_DIR, PROTOS_DIR, protoc_bin, module)

  write_root_module(ROOT_DIR)

  if TMP_DIR.exists():
    shutil.rmtree(TMP_DIR)


if not __name__ == "__main__":
  raise RuntimeError("protoc should not be imported. Execute it as a script")

main()
