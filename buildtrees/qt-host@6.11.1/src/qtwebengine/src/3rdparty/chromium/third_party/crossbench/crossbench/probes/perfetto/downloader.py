# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Final, Mapping, Optional

from crossbench import exception, plt

if TYPE_CHECKING:
  import crossbench.path as pth

_BASE_STORAGE_URL = (
    "https://commondatastorage.googleapis.com/perfetto-luci-artifacts")

# Copied from perfetto sources:
# https://crsrc.org/c/third_party/perfetto/python/perfetto/prebuilts/manifests/tracebox.py
PLATFORM_LOOKUP: Final[Mapping[tuple[str, str], str]] = {
    ("linux", "x64"): "linux-amd64",
    ("linux", "arm64"): "linux-arm64",
    ("linux", "arm32"): "linux-arm",
    ("chromeos_ssh", "x64"): "linux-amd64",
    ("chromeos_ssh", "arm64"): "linux-arm64",
    ("chromeos_ssh", "arm32"): "linux-arm",
    ("macos", "x64"): "mac-amd64",
    ("macos", "arm64"): "mac-arm64",
    ("android", "arm32"): "android-arm",
    ("android", "arm64"): "android-arm64",
    ("android", "ia32"): "android-x86",
    ("android", "x64"): "android-x64",
}


class PerfettoToolDownloader:

  def __init__(self,
               tool: str,
               version: str = "v51.2",
               platform: Optional[plt.Platform] = None) -> None:
    self._version = version
    self._tool = tool
    self._platform = platform or plt.PLATFORM

  @property
  def version(self) -> str:
    return self._version

  @property
  def url(self) -> str:
    # TODO: use new platform.lookup helper.
    platform_name = PLATFORM_LOOKUP[self._platform.key]
    return f"{_BASE_STORAGE_URL}/{self._version}/{platform_name}/{self._tool}"

  @property
  def path(self) -> pth.AnyPath:
    out_dir = self._platform.cache_dir("perfetto")
    version_dir = out_dir / self._version
    result_path = version_dir / self._tool
    return result_path

  def download(self) -> pth.AnyPath:
    result_path = self.path
    if self._platform.exists(result_path):
      return result_path
    with exception.annotate(f"Downloading {self._tool} binary"):
      self._platform.mkdir(result_path.parent, parents=True, exist_ok=True)
      self._platform.download_to(self.url, result_path)
      self._platform.chmod(result_path, 0o755)
    with exception.annotate(f"Validate {self._tool} binary"):
      version_str = self._platform.sh_stdout(result_path, "--version")
      if self.version not in version_str:
        raise RuntimeError(f"{self._tool} has a different version, "
                           f"expected {self.version}, got: {version_str}")

    return result_path
