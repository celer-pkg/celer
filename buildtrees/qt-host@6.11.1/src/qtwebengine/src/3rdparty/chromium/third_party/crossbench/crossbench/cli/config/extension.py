# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
import logging
import re
import zipfile
from typing import TYPE_CHECKING, Optional, Self

from typing_extensions import override

from crossbench.config import ConfigObject
from crossbench.helper import url_helper
from crossbench.parse import PathParser

if TYPE_CHECKING:
  from crossbench.path import LocalPath
  from crossbench.plt.base import Platform

EXTENSION_ID_PATTERN: re.Pattern = re.compile(r"^[a-p]{32}$")


@dataclasses.dataclass(frozen=True)
class ExtensionConfig(ConfigObject):
  VALID_EXTENSIONS = (".crx",)
  crx: Optional[LocalPath] = None
  id: Optional[str] = None
  unpacked: Optional[LocalPath] = None

  @classmethod
  @override
  def maybe_valid_path(cls, path: LocalPath) -> LocalPath | None:
    if super().maybe_valid_path(path):
      return path
    manifest_path = path / "manifest.json"
    if manifest_path.is_file():
      return path
    return None

  @classmethod
  @override
  def parse_path(cls, path: LocalPath, **kwargs) -> Self:
    if "," in str(path):
      raise argparse.ArgumentTypeError("Extension paths must not contain ','")
    if path.is_file():
      assert path.suffix == ".crx", (
          f"Extension files must be crx: {repr(str(path))}")
      return cls(crx=path, id=None, unpacked=None)
    # Extension is unpacked in a directory.
    manifest_path = path / "manifest.json"
    assert manifest_path.exists(), (
        f"Extension dirs must contain a manifest.json: {repr(str(path))}")
    return cls(crx=None, id=None, unpacked=path)

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    if EXTENSION_ID_PATTERN.match(value):
      return cls(crx=None, id=value, unpacked=None)

    path = PathParser.existing_path(value, name="extension path")
    return cls.parse_path(path)

  @override
  def validate(self) -> None:
    super().validate()

    set_count = sum(
        prop is not None for prop in [self.crx, self.id, self.unpacked])
    if set_count != 1:
      raise ValueError("Only 1 of crx, id, unpacked should be set")

  def get_unpacked(self, version_str: str, tmp_dir: LocalPath,
                   host_platform: Platform) -> LocalPath:
    if self.unpacked:
      return self.unpacked
    if self.crx:
      logging.info("Extracting extension %s", self.crx)
      unpacked = tmp_dir / self.crx.stem
      unpacked.mkdir()
      with zipfile.ZipFile(self.crx) as zip_ref:
        zip_ref.extractall(unpacked)
      return unpacked
    if self.id:
      crx_cache = host_platform.local_cache_dir(
          f"extension_{self.id}") / f"{version_str}.crx"
      if not crx_cache.exists():
        logging.info("Downloading extension %s", self.id)
        crx_url = url_helper.update_url_query(
            "https://clients2.google.com/service/update2/crx", {
                "response": "redirect",
                "prodversion": version_str,
                "acceptformat": "crx2,crx3",
                "x": f"id={self.id}&uc"
            })
        response = url_helper.get(crx_url)
        response.raise_for_status()
        crx_cache.write_bytes(response.content)
      else:
        logging.info("Loading extension from cache %s", crx_cache)

      unpacked = tmp_dir / self.id
      unpacked.mkdir()
      with zipfile.ZipFile(crx_cache) as zip_ref:
        zip_ref.extractall(unpacked)
      return unpacked
    raise RuntimeError("Unsupported ExtensionConfig type")
