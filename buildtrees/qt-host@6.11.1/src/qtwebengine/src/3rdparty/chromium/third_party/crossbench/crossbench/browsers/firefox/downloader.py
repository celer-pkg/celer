# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import os
import shutil
from typing import TYPE_CHECKING, Final, Iterable, Optional, Type

from typing_extensions import override

from crossbench.browsers.downloader import DMGArchiveHelper, Downloader
from crossbench.browsers.firefox.version import FirefoxVersion
from crossbench.helper import url_helper

if TYPE_CHECKING:
  from crossbench.browsers.version import BrowserVersion
  from crossbench.path import AnyPathLike, LocalPath
  from crossbench.plt.base import Platform


_PLATFORM_NAME_LOOKUP: Final[dict[tuple[str, str], str]] = {
    ("win", "ia32"): "win32",
    ("win", "x64"): "win64",
    ("win", "arm64"): "win-aarch64",
    ("linux", "x64"): "linux-x86-64",
    ("linux", "ia32"): "linux-i686",
    ("macos", "x64"): "mac",
    ("macos", "arm64"): "mac"
}


class FirefoxDownloader(Downloader):
  # TODO: support nightly versions as well
  STORAGE_URL: str = "https://ftp.mozilla.org/pub/firefox/releases/"

  @classmethod
  @override
  def _get_loader_cls(cls,
                      browser_platform: Platform) -> Type[FirefoxDownloader]:
    if browser_platform.is_macos:
      return FirefoxDownloaderMacOS
    if browser_platform.is_linux:
      return FirefoxDownloaderLinux
    if browser_platform.is_win:
      return FirefoxDownloaderWin
    raise ValueError("Downloading Firefox is not supported "
                     f"{browser_platform.name} {browser_platform.machine}")

  @classmethod
  @override
  def is_valid_version(cls, path_or_identifier: str) -> bool:
    return FirefoxVersion.is_valid_unique(path_or_identifier)

  @classmethod
  def _is_valid(cls, path_or_identifier: AnyPathLike,
                browser_platform: Platform) -> bool:
    if cls.is_valid_version(str(path_or_identifier)):
      return True
    path = browser_platform.path(path_or_identifier)
    return (browser_platform.exists(path) and
            path.name.endswith(cls.ARCHIVE_SUFFIX))

  def __init__(self, version_identifier: str | LocalPath, browser_type: str,
               platform_name: str, browser_platform: Platform) -> None:
    assert not browser_type
    assert not platform_name
    firefox_platform_name = _PLATFORM_NAME_LOOKUP.get(browser_platform.key)
    if not firefox_platform_name:
      raise ValueError(
          "Unsupported macOS architecture for downloading Firefox: "
          f"got={browser_platform.machine}")
    super().__init__(version_identifier, "firefox", firefox_platform_name,
                     browser_platform)

  @override
  def _parse_version(self, version_identifier: str) -> BrowserVersion:
    return FirefoxVersion.parse(version_identifier)

  @override
  def _requested_version_validation(self) -> None:
    pass

  @override
  def _find_archive_url(self) -> tuple[BrowserVersion, Optional[str]]:
    # Quick probe for complete versions
    if self.requested_version.is_complete:
      return self._find_exact_archive_url()
    raise NotImplementedError("Only full-release versions supported.")

  def _find_exact_archive_url(self) -> tuple[BrowserVersion, Optional[str]]:
    folder_url = (
        f"{self.STORAGE_URL}{self.requested_version.parts_str}/mac/en-GB")
    return tuple(self._archive_urls(folder_url, self.requested_version))[0]

  @override
  def _download_archive(self, archive_url: str, tmp_dir: LocalPath) -> None:
    self._browser_platform.download_to(
        archive_url, tmp_dir / f"archive.{self.ARCHIVE_SUFFIX}")
    archive_candidates = list(tmp_dir.glob("*"))
    assert len(archive_candidates) == 1, (
        f"Download tmp dir contains more than one file: {tmp_dir}"
        f"{archive_candidates}")
    candidate = archive_candidates[0]
    assert not self._archive_path.exists(), (
        f"Archive was already downloaded: {self._archive_path}")
    shutil.move(os.fspath(candidate), os.fspath(self._archive_path))

  @abc.abstractmethod
  @override
  def _install_archive(self, archive_path: LocalPath) -> None:
    pass


class FirefoxDownloaderLinux(FirefoxDownloader):
  ARCHIVE_SUFFIX: str = ".tar.bz2"

  @classmethod
  @override
  def is_valid(cls, path_or_identifier: AnyPathLike,
               browser_platform: Platform) -> bool:
    return cls._is_valid(path_or_identifier, browser_platform)

  @override
  def _installed_app_path(self) -> LocalPath:
    # TODO: support local vs remote
    return self._extracted_path() / "firefox-bin"

  @override
  def _archive_urls(
      self, folder_url: str,
      version: BrowserVersion) -> Iterable[tuple[BrowserVersion, str]]:
    return ((version, f"{folder_url}/firefox-{version.parts_str}.tar.bz2"),)

  @override
  def _install_archive(self, archive_path: LocalPath) -> None:
    raise NotImplementedError("Missing linux support")


class FirefoxDownloaderMacOS(FirefoxDownloader):
  ARCHIVE_SUFFIX: str = ".dmg"
  MIN_MAC_ARM64_MILESTONE: Final[int] = 84

  @classmethod
  @override
  def is_valid(cls, path_or_identifier: AnyPathLike,
               browser_platform: Platform) -> bool:
    return cls._is_valid(path_or_identifier, browser_platform)

  @override
  def _requested_version_validation(self) -> None:
    major_version: int = self.requested_version.major
    if (self._browser_platform.is_macos and self._browser_platform.is_arm64 and
        major_version < self.MIN_MAC_ARM64_MILESTONE):
      raise ValueError(
          "Native Mac arm64/m1 Firefox version is available with v84, "
          f"but requested {major_version}.")

  @override
  def _download_archive(self, archive_url: str, tmp_dir: LocalPath) -> None:
    assert self._browser_platform.is_macos
    if self._browser_platform.is_arm64 and (self.requested_version
                                            < self.MIN_MAC_ARM64_MILESTONE):
      raise ValueError(
          "Firefox Arm64 Apple Silicon is only available starting with "
          f"{self.MIN_MAC_ARM64_MILESTONE}, "
          f"but requested {self.requested_version} is too old.")
    super()._download_archive(archive_url, tmp_dir)

  @override
  def _archive_urls(
      self, folder_url: str,
      version: BrowserVersion) -> Iterable[tuple[BrowserVersion, str]]:
    archive_name = url_helper.quote(f"firefox {version.parts_str}.dmg")
    return ((version, f"{folder_url}/{archive_name}"),)

  @override
  def _extracted_path(self) -> LocalPath:
    # TODO: support local vs remote
    return self._installed_app_path()

  @override
  def _installed_app_path(self) -> LocalPath:
    return self._out_dir / f"Firefox {self.requested_version}.app"

  @override
  def _install_archive(self, archive_path: LocalPath) -> None:
    extracted_path = self._extracted_path()
    DMGArchiveHelper.extract(self.host_platform, archive_path, extracted_path)
    assert extracted_path.exists(), (
        f"Could not extract {archive_path} into {extracted_path}")


class FirefoxDownloaderWin(FirefoxDownloader):

  @classmethod
  @override
  def is_valid(cls, path_or_identifier: AnyPathLike,
               browser_platform: Platform) -> bool:
    return False

  @override
  def _install_archive(self, archive_path: LocalPath) -> None:
    raise NotImplementedError("Missing windows support")
