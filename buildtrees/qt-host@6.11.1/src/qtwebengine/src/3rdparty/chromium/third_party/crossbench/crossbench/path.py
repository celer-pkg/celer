# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import hashlib
import pathlib
import re
import unicodedata
from typing import Optional, TypeAlias

# A path that can refer to files on a remote platform with potentially
# a different Path flavour (e.g. Win vs Posix).
AnyPath: TypeAlias = pathlib.PurePath
AnyPosixPath: TypeAlias = pathlib.PurePosixPath
AnyWindowsPath: TypeAlias = pathlib.PureWindowsPath

AnyPathLike: TypeAlias = str | AnyPath

# A path that only ever refers to files on the local host / runner platform.
# Not that Path inherits from PurePath, and thus we can use a LocalPath in
# all places a RemotePath is expected.
LocalPath: TypeAlias = pathlib.Path
LocalPosixPath: TypeAlias = pathlib.PosixPath

LocalPathLike: TypeAlias = str | LocalPath

MAX_PART_LEN = 255

_UNSAFE_FILENAME_CHARS_RE: re.Pattern[str] = re.compile(r"[^a-zA-Z0-9+\-_.]")


def safe_filename(name: str, strict_len: bool = False) -> str:
  normalized_name = unicodedata.normalize("NFKD", name)
  ascii_name = normalized_name.encode("ascii", "ignore").decode("ascii")
  safe_name: str = _UNSAFE_FILENAME_CHARS_RE.sub("_", ascii_name)
  if strict_len and len(safe_name) > MAX_PART_LEN:
    raise ValueError(f"Too long file name: {repr(safe_name)}")
  return safe_name[:MAX_PART_LEN]


def try_resolve_existing_path(value: str) -> Optional[LocalPath]:
  if not value:
    return None
  maybe_path = LocalPath(value)
  if maybe_path.exists():
    return maybe_path
  maybe_path = maybe_path.expanduser()
  if maybe_path.exists():
    return maybe_path
  return None


def check_hash(file_path: LocalPath, file_hash: str) -> bool:
  if not file_path.exists():
    return False
  sha1 = hashlib.sha1()
  sha1.update(file_path.read_bytes())
  return sha1.hexdigest() == file_hash
