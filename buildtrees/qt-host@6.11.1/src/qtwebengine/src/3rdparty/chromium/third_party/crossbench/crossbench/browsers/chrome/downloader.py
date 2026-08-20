# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import contextlib
import logging
import os
import shutil
import tempfile
import zipfile
from typing import TYPE_CHECKING, Final, Iterable, Optional, Type, cast

from typing_extensions import override

from crossbench import path as pth
from crossbench.browsers.chrome.version import ChromeVersion
from crossbench.browsers.downloader import (DMGArchiveHelper, Downloader,
                                            IncompatibleVersionError,
                                            RPMArchiveHelper)
from crossbench.browsers.version import BrowserVersion, BrowserVersionChannel
from crossbench.helper import url_helper
from crossbench.plt.android_adb import AndroidAdbPlatform
from crossbench.plt.base import SubprocessError

if TYPE_CHECKING:
  from crossbench.plt.android_adb import Adb
  from crossbench.plt.base import Platform


class ChromeDownloader(Downloader):
  STORAGE_URL: str = "gs://chrome-signed/desktop-5c0tCh/"
  VERSION_URL = (
      "https://versionhistory.googleapis.com/v1/"
      "chrome/platforms/{platform}/channels/{channel}/versions?filter={filter}")
  VERSION_URL_PLATFORM_LOOKUP: dict[tuple[str, str], str] = {
      ("win", "arm64"): "win_arm64",
      ("win", "ia32"): "win",
      ("win", "x64"): "win64",
      ("linux", "x64"): "linux",
      ("macos", "x64"): "mac",
      ("macos", "arm64"): "mac_arm64",
      ("android", "arm64"): "android",
  }

  def __init__(self, *args, **kwargs) -> None:
    self._gsutil: pth.AnyPath | None = None
    super().__init__(*args, **kwargs)

  @classmethod
  @override
  def is_valid_version(cls, path_or_identifier: str) -> bool:
    return ChromeVersion.is_valid_unique(path_or_identifier)

  @classmethod
  def _is_valid(cls, path_or_identifier: pth.AnyPathLike,
                browser_platform: Platform) -> bool:
    if cls.is_valid_version(str(path_or_identifier)):
      return True
    path = browser_platform.path(path_or_identifier)
    return (browser_platform.exists(path) and
            path.name.endswith(cls.ARCHIVE_SUFFIX))

  @classmethod
  @override
  def _get_loader_cls(cls,
                      browser_platform: Platform) -> Type[ChromeDownloader]:
    if browser_platform.is_macos:
      return ChromeDownloaderMacOS
    if browser_platform.is_linux:
      return ChromeDownloaderLinux
    if browser_platform.is_win:
      return ChromeDownloaderWin
    if browser_platform.is_android:
      return ChromeDownloaderAndroid
    raise ValueError(
        "Downloading chrome is only supported on linux and macOS, "
        f"but not on {browser_platform.name} {browser_platform.machine}")

  @override
  def _pre_check(self,
                 requested_version: Optional[BrowserVersion] = None) -> None:
    super()._pre_check(requested_version)
    if not requested_version:
      return
    self._gsutil = self.host_platform.which("gsutil")
    if not self._gsutil:
      raise ValueError(
          f"Cannot download chrome version {requested_version}: "
          "please install gsutil.\n"
          "- https://cloud.google.com/storage/docs/gsutil_install\n"
          "- Run 'gcloud auth login' to get access to the archives "
          "(googlers only).")

  @property
  def gsutil(self) -> pth.AnyPath:
    assert self._gsutil, "gsutil not be found."
    return self._gsutil

  @override
  def _requested_version_validation(self) -> None:
    pass

  @override
  def _parse_version(self, version_identifier: str) -> BrowserVersion:
    return ChromeVersion.parse_unique(version_identifier)

  @override
  def _find_archive_url(self) -> tuple[BrowserVersion, Optional[str]]:
    # Quick probe for complete versions
    if self.requested_version.is_complete:
      return self._find_exact_archive_url()
    return self._find_milestone_archive_url()

  def _find_milestone_archive_url(self) -> tuple[BrowserVersion, Optional[str]]:
    platform = self.VERSION_URL_PLATFORM_LOOKUP.get(self._browser_platform.key)
    if not platform:
      raise ValueError(f"Unsupported platform {self._browser_platform}")
    # Version ordering is: stable < beta < dev < canary < canary_asan
    # See https://developer.chrome.com/docs/web-platform/versionhistory/reference#filter
    channel_filter: str = "channel<=canary"
    channel: str = "all"
    requested_channel = BrowserVersionChannel.ANY
    version: BrowserVersion = self.requested_version
    if version.has_channel:
      requested_channel = version.channel
      channel_filter = f"channel={version.channel_name}"
      channel = version.channel_name

    milestone_filter: str = ""
    if not version.is_channel_version:
      milestone: int = version.major
      milestone_filter = f"version>={milestone},version<{milestone+1}"

    url = self.VERSION_URL.format(
        platform=platform,
        channel=channel,
        filter=f"{milestone_filter},{channel_filter}&")
    self.info(f"Listing all versions at {url}")
    version_urls: list[tuple[BrowserVersion, str]] = []
    try:
      response = url_helper.get(url, retry=3, timeout=100)
      raw_infos = response.json()["versions"]
      version_urls = [
          self._create_version_url(
              ChromeVersion(
                  map(int, info["version"].split(".")), requested_channel))
          for info in raw_infos
      ]
    except Exception as e:
      raise ValueError(
          f"Could not find version {version} "
          f"for {self._browser_platform.name} {self._browser_platform.machine} "
      ) from e
    self.info(f"Filtering {len(version_urls)} candidates")
    return self._filter_candidate_urls(version_urls)

  def _create_version_url(
      self, version: BrowserVersion) -> tuple[BrowserVersion, str]:
    # TODO: respect channel
    assert version.has_complete_parts
    return (version,
            f"{self.STORAGE_URL}{version.parts_str}/{self._platform_name}/")

  def _find_exact_archive_url(self) -> tuple[BrowserVersion, Optional[str]]:
    # TODO: respect channel
    version, test_url = self._create_version_url(self.requested_version)
    self.info(f"LIST VERSIONS at {test_url}")
    return self._filter_candidate_urls([(version, test_url)])

  def _filter_candidate_urls(
      self, versions_urls: list[tuple[BrowserVersion, str]]
  ) -> tuple[BrowserVersion, Optional[str]]:
    versions_urls.sort(key=lambda version_url: version_url[0], reverse=True)
    # Iterate from new to old version and and the first one that is older or
    # equal than the requested version.
    access_error: str = ""
    for version, url in versions_urls:
      if not self.requested_version.contains(version):
        logging.debug("Skipping download candidate: %s %s", version, url)
        continue
      # https://crbug.com/409334109: sometimes we get non-canary builds in
      # the archive. Canary versions always end in 0.
      if self.requested_version.is_pre_alpha and version.parts[-1] != 0:
        logging.debug("Skipping non-canary build: %s %s", version, url)
        continue
      for archive_version, archive_url in self._archive_urls(url, version):
        try:
          result = self.host_platform.sh_stdout(self.gsutil, "ls", archive_url)
        except SubprocessError as e:
          logging.debug("gsutil failed: %s", e)
          if stderr := e.stderr:
            stderr_str = stderr.decode("utf-8")
            if "AccessDeniedException" in stderr_str:
              access_error = stderr_str
          continue
        if result:
          return archive_version, archive_url
    if access_error:
      raise ValueError(f"Could not load version: {access_error}")
    return self.requested_version, None

  @override
  def _download_archive(self, archive_url: str, tmp_dir: pth.LocalPath) -> None:
    self.host_platform.sh(self.gsutil, "cp", archive_url, tmp_dir)
    archive_candidates = list(tmp_dir.glob("*"))
    assert len(archive_candidates) == 1, (
        f"Download tmp dir contains more than one file: {tmp_dir}: "
        f"{archive_candidates}")
    candidate = archive_candidates[0]
    assert not self._archive_path.exists(), (
        f"Archive was already downloaded: {self._archive_path}")
    shutil.move(os.fspath(candidate), os.fspath(self._archive_path))


class ChromeDownloaderLinux(ChromeDownloader):
  ARCHIVE_SUFFIX: str = ".rpm"
  CHANNEL_BINARY_LOOKUP: dict[BrowserVersionChannel, str] = {
      BrowserVersionChannel.PRE_ALPHA: "chrome-canary",
      BrowserVersionChannel.ALPHA: "chrome-unstable",
      BrowserVersionChannel.BETA: "chrome-beta",
      BrowserVersionChannel.STABLE: "chrome",
  }

  @classmethod
  @override
  def is_valid(cls, path_or_identifier: pth.AnyPathLike,
               browser_platform: Platform) -> bool:
    return cls._is_valid(path_or_identifier, browser_platform)

  def __init__(self, version_identifier: str | pth.LocalPath, browser_type: str,
               platform_name: str, browser_platform: Platform) -> None:
    assert not browser_type
    if browser_platform.is_linux and browser_platform.is_x64:
      platform_name = "linux64"
    else:
      raise ValueError("Unsupported linux architecture for downloading chrome: "
                       f"got={browser_platform.machine} supported=linux.x64")
    super().__init__(version_identifier, "chrome", platform_name,
                     browser_platform)

  @override
  def _installed_app_path(self) -> pth.LocalPath:
    base_dir: pth.LocalPath = self._extracted_path() / "opt/google"
    version: BrowserVersion = self.requested_version
    if version.has_channel:
      channel_name = self.CHANNEL_BINARY_LOOKUP[version.channel]
      return base_dir / channel_name / "chrome"
    for _, channel_name in self.CHANNEL_BINARY_LOOKUP.items():
      bin_path: pth.LocalPath = base_dir / channel_name / "chrome"
      if bin_path.exists():
        return bin_path
    logging.debug("Could not find binary for %s in %s", self.requested_version,
                  base_dir)
    return pth.LocalPath()

  @override
  def _archive_urls(
      self, folder_url: str,
      version: BrowserVersion) -> Iterable[tuple[BrowserVersion, str]]:
    parts_str = version.parts_str
    parts = version.parts
    stable = (ChromeVersion.stable(parts),
              f"{folder_url}google-chrome-stable-{parts_str}-1.x86_64.rpm")
    if version.is_stable:
      return (stable,)
    beta = (ChromeVersion.beta(parts),
            f"{folder_url}google-chrome-beta-{parts_str}-1.x86_64.rpm")
    if version.is_beta:
      return (beta,)
    dev = (ChromeVersion.dev(parts),
           f"{folder_url}google-chrome-unstable-{parts_str}-1.x86_64.rpm")
    if version.is_alpha:
      return (dev,)
    canary = (ChromeVersion.canary(parts),
              f"{folder_url}google-chrome-canary-{parts_str}-1.x86_64.rpm")
    if version.is_pre_alpha:
      return (canary,)
    return (stable, beta, dev, canary)

  @override
  def _install_archive(self, archive_path: pth.LocalPath) -> None:
    extracted_path = self._extracted_path()
    RPMArchiveHelper.extract(self.host_platform, archive_path, extracted_path)
    assert extracted_path.exists(), (
        f"Could not extract {archive_path} into {extracted_path}")


class ChromeDownloaderMacOS(ChromeDownloader):
  ARCHIVE_SUFFIX: str = ".dmg"
  MIN_MAC_ARM64_MILESTONE: Final[int] = 87

  @classmethod
  @override
  def is_valid(cls, path_or_identifier: pth.AnyPathLike,
               browser_platform: Platform) -> bool:
    return cls._is_valid(path_or_identifier, browser_platform)

  def __init__(self, version_identifier: str | pth.LocalPath, browser_type: str,
               platform_name: str, browser_platform: Platform) -> None:
    assert not browser_type
    assert browser_platform.is_macos, f"{type(self)} can only be used on macOS"
    platform_name = "mac-universal"
    super().__init__(version_identifier, "chrome", platform_name,
                     browser_platform)

  @override
  def _requested_version_validation(self) -> None:
    assert self._browser_platform.is_macos
    if self.requested_version.is_channel_version:
      return
    major_version: int = self.requested_version.major
    if (self._browser_platform.is_arm64 and
        (major_version < self.MIN_MAC_ARM64_MILESTONE)):
      raise ValueError(
          "Chrome Arm64 Apple Silicon is only available starting with M87, "
          f"but requested {self.requested_version} is too old.")

  @override
  def _download_archive(self, archive_url: str, tmp_dir: pth.LocalPath) -> None:
    self._requested_version_validation()
    super()._download_archive(archive_url, tmp_dir)

  @override
  def _archive_urls(
      self, folder_url: str,
      version: BrowserVersion) -> Iterable[tuple[BrowserVersion, str]]:
    # TODO: respect channel
    version_str: str = version.parts_str
    parts = version.parts
    stable = (ChromeVersion.stable(parts),
              f"{folder_url}GoogleChrome-{version_str}.dmg")
    if version.is_stable:
      return (stable,)
    beta = (ChromeVersion.beta(parts),
            f"{folder_url}GoogleChromeBeta-{version_str}.dmg")
    if version.is_beta:
      return (beta,)
    dev = (ChromeVersion.dev(parts),
           f"{folder_url}GoogleChromeDev-{version_str}.dmg")
    if version.is_alpha:
      return (dev,)
    canary = (ChromeVersion.canary(parts),
              f"{folder_url}GoogleChromeCanary-{version_str}.dmg")
    if version.is_pre_alpha:
      return (canary,)
    return (stable, beta, dev, canary)

  @override
  def _extracted_path(self) -> pth.LocalPath:
    # TODO: support local vs remote
    return self._installed_app_path()

  @override
  def _installed_app_path(self) -> pth.LocalPath:
    return self._out_dir / f"Google Chrome {self.requested_version}.app"

  @override
  def _install_archive(self, archive_path: pth.LocalPath) -> None:
    extracted_path = self._extracted_path()
    if archive_path.suffix == ".dmg":
      DMGArchiveHelper.extract(self.host_platform, archive_path, extracted_path)
    else:
      raise ValueError(f"Unknown archive type: {archive_path}")
    assert extracted_path.exists(), (
        f"Could not extract {archive_path} into {extracted_path}")


class ChromeDownloaderAndroid(ChromeDownloader):
  """The android downloader for Chrome pulls .apks and the
  corresponding .apk library and installs both on the attached device."""
  ARCHIVE_SUFFIX: str = ".apks"
  LIBRARY_ARCHIVE_SUFFIX: str = ".lib.apk"
  STORAGE_URL: str = "gs://chrome-signed/android-B0urB0N/"

  MIN_HIGH_ARM_64_MILESTONE: Final[int] = 104
  ARM_32_BUILD: Final[str] = "arm"
  ARM_64_BUILD: Final[str] = "arm_64"
  ARM_64_HIGH_BUILD: Final[str] = "high-arm_64"

  CHANNEL_PACKAGE_LOOKUP: dict[str, tuple[str, BrowserVersionChannel]] = {
      "Beta": (
          "com.chrome.beta",
          BrowserVersionChannel.BETA,
      ),
      "Dev": ("com.chrome.dev", BrowserVersionChannel.ALPHA),
      "Canary": ("com.chrome.canary", BrowserVersionChannel.PRE_ALPHA),
      # Let's check stable last to avoid overriding the default installation
      # if possible.
      "Stable": ("com.android.chrome", BrowserVersionChannel.STABLE),
  }

  @classmethod
  @override
  def is_valid(cls, path_or_identifier: pth.AnyPathLike,
               browser_platform: Platform) -> bool:
    return cls._is_valid(path_or_identifier, browser_platform)

  def __init__(self, version_identifier: str | pth.LocalPath, browser_type: str,
               platform_name: str, browser_platform: Platform) -> None:
    assert not browser_type
    assert browser_platform.is_android, (
        f"{type(self)} can only be used on Android")
    # TODO: support more CPU types
    assert browser_platform.is_arm64, f"{type(self)} only supports arm64"
    # TODO: support low-end arm_64 and high-arm_64 at the same time.
    platform_name = "high-arm_64"
    super().__init__(version_identifier, "chrome", platform_name,
                     browser_platform)

  @property
  def adb(self) -> Adb:
    return cast(AndroidAdbPlatform, self._browser_platform).adb

  @override
  def _pre_check(self,
                 requested_version: Optional[BrowserVersion] = None) -> None:
    super()._pre_check(requested_version)
    assert self._browser_platform.is_android, (
        f"Expected android but got {self._browser_platform}")

  @override
  def _requested_version_validation(self) -> None:
    assert self._browser_platform.is_android
    # TODO: support custom android builds
    if self.requested_version.major < self.MIN_HIGH_ARM_64_MILESTONE:
      self._platform_name = self.ARM_64_BUILD
    else:
      self._platform_name = self.ARM_64_HIGH_BUILD

  @override
  def _installed_app_version(self, app_path: pth.LocalPath) -> BrowserVersion:
    raw_version = self._browser_platform.app_version(app_path)
    channel = BrowserVersionChannel.STABLE
    for value in self.CHANNEL_PACKAGE_LOOKUP.values():
      (package_name, package_channel) = value
      if app_path.name == package_name:
        channel = package_channel
        break
    return ChromeVersion.parse(raw_version, channel)

  @override
  def _archive_urls(
      self, folder_url: str,
      version: BrowserVersion) -> Iterable[tuple[BrowserVersion, str]]:
    prefix: str = f"{folder_url}"
    urls: list[tuple[BrowserVersion, str]] = []
    # TODO: pass in correct sdk_level
    package = self._get_chrome_package(100)
    # TODO: respect version channel
    for channel_name, (_, channel) in self.CHANNEL_PACKAGE_LOOKUP.items():
      channel_version = ChromeVersion(version.parts, channel)
      version_url = (channel_version,
                     f"{prefix}{package}{channel_name}{self.ARCHIVE_SUFFIX}")
      if version.matches_channel(channel_version.channel):
        return (version_url,)
      urls.append(version_url)
    return tuple(urls)

  def _get_chrome_package(self, sdk_level: int) -> str:
    del sdk_level
    # TODO support older SDKs at some point
    # if sdk_level < 19:
    #   raise RuntimeError(
    #       f"Clank can only be installed on >= 19, not {sdk_level}")
    # if sdk_level < 21:
    #   return "Chrome"
    # if sdk_level < 24:
    #   return "ChromeModern"
    # if sdk_level < 29:
    #   return "Monochrome"
    return "TrichromeChromeGoogle6432"

  @override
  def _extracted_path(self) -> pth.LocalPath:
    return self._archive_path

  @override
  def _installed_app_path(self) -> pth.LocalPath:
    for channel, (package_name, _) in self.CHANNEL_PACKAGE_LOOKUP.items():
      if channel in self._archive_url:
        logging.debug("Using package: %s", package_name)
        return pth.LocalPath(package_name)
    package_name, _ = self.CHANNEL_PACKAGE_LOOKUP["Stable"]
    return pth.LocalPath(package_name)

  @override
  def _find_matching_installed_version(self) -> Optional[pth.LocalPath]:
    # TODO: we should use aapt and read the package name directly from
    # the apk: `aapt dump badging <path-to-apk> | grep package:\ name`
    # Iterate over all chrome versions and find any matching release
    installed_packages = self.adb.packages()
    for value in self.CHANNEL_PACKAGE_LOOKUP.values():
      (package_name, package_channel) = value
      if not self.requested_version.matches_channel(package_channel):
        continue
      if package_name not in installed_packages:
        continue
      try:
        package = pth.LocalPath(package_name)
        self._validate_installed(package)
        return package
      except IncompatibleVersionError as e:
        logging.debug("Ignoring installed package %s: %s", package_name, e)
    return None

  @override
  def _download_archive(self, archive_url: str, tmp_dir: pth.LocalPath) -> None:
    super()._download_archive(archive_url, tmp_dir)
    if "TrichromeChromeGoogle" not in archive_url:
      return
    # Download TrichromeLibrary.apk needed by TrichromeChromeGoogle.apks
    with self._prepare_lib_archive_download(archive_url) as (lib_archive_url,
                                                             lib_tmp_dir):
      super()._download_archive(lib_archive_url, lib_tmp_dir)

  @contextlib.contextmanager
  def _prepare_lib_archive_download(self, archive_url: str):
    # Also download the trichrome library (such a mess)
    main_archive_path = self._archive_path
    lib_archive_path = main_archive_path.with_suffix(
        self.LIBRARY_ARCHIVE_SUFFIX)
    if lib_archive_path.exists():
      return
    self._archive_path = lib_archive_path
    lib_url = archive_url.replace("TrichromeChromeGoogle",
                                  "TrichromeLibraryGoogle")
    lib_url = lib_url.replace(self.ARCHIVE_SUFFIX, ".apk")
    with tempfile.TemporaryDirectory(prefix="cb_download_") as tmp_dir_name:
      lib_tmp_dir = pth.LocalPath(tmp_dir_name)
      yield lib_url, lib_tmp_dir
    self._archive_path = main_archive_path

  @override
  def _install_archive(self, archive_path: pth.LocalPath) -> None:
    # TODO: move browser installation to browser startup to allow
    # multiple versions on android in a single crossbench invocation
    package = str(self._installed_app_path())
    self.adb.uninstall(package, missing_ok=True)
    lib_archive_path = archive_path.with_suffix(self.LIBRARY_ARCHIVE_SUFFIX)
    if lib_archive_path.exists():
      self.adb.install(lib_archive_path, allow_downgrade=True, modules="_ALL_")
    self.adb.install(archive_path, allow_downgrade=True, modules="_ALL_")


class ChromeDownloaderWin(ChromeDownloader):
  ARCHIVE_SUFFIX: str = ".zip"
  ARCHIVE_STEM_X64: str = "chrome-win64-clang"
  ARCHIVE_STEM_ARM: str = "chrome-win-arm64-clang"
  STORAGE_URL: str = "gs://chrome-unsigned/desktop-5c0tCh/"
  MIN_WIN_ARM64_MILESTONE: Final[int] = 118

  @classmethod
  @override
  def is_valid(cls, path_or_identifier: pth.AnyPathLike,
               browser_platform: Platform) -> bool:
    return cls._is_valid(path_or_identifier, browser_platform)

  def __init__(self, version_identifier: str | pth.LocalPath, browser_type: str,
               platform_name: str, browser_platform: Platform) -> None:
    assert not browser_type
    assert browser_platform.is_win, f"{type(self)} can only be used on windows"
    self._archive_stem: str
    if browser_platform.is_arm64:
      platform_name = "win-arm64-clang"
      self._archive_stem = self.ARCHIVE_STEM_ARM
    else:
      platform_name = "win64-clang"
      self._archive_stem = self.ARCHIVE_STEM_X64
    super().__init__(version_identifier, "chrome", platform_name,
                     browser_platform)

  @override
  def _requested_version_validation(self) -> None:
    assert self._browser_platform.is_win
    if self._requested_version.is_channel_version:
      return
    major_version: int = self._requested_version.major
    if (self._browser_platform.is_arm64 and
        (major_version < self.MIN_WIN_ARM64_MILESTONE)):
      raise ValueError(
          "Chrome Arm64 for Windows is only available starting with M118, "
          f"but requested {self._requested_version} is too old.")

  @override
  def _download_archive(self, archive_url: str, tmp_dir: pth.LocalPath) -> None:
    self._requested_version_validation()
    super()._download_archive(archive_url, tmp_dir)

  @override
  def _archive_urls(
      self, folder_url: str,
      version: BrowserVersion) -> Iterable[tuple[BrowserVersion, str]]:
    parts = version.parts
    stable = (ChromeVersion.stable(parts),
              f"{folder_url}{self._archive_stem}.zip")
    return (stable,)

  @override
  def _extracted_path(self) -> pth.LocalPath:
    # TODO: support local vs remote
    return self._out_dir / f"Google Chrome {self.requested_version}"

  @override
  def _installed_app_path(self) -> pth.LocalPath:
    return self._extracted_path() / "chrome.exe"

  @override
  def _install_archive(self, archive_path: pth.LocalPath) -> None:
    extracted_path = self._extracted_path()
    tmp_path = self.host_platform.mkdtemp()
    with zipfile.ZipFile(archive_path, "r") as zip_file:
      zip_file.extractall(tmp_path)
    self.host_platform.rename(tmp_path / self._archive_stem, extracted_path)
    assert self.host_platform.is_dir(extracted_path), (
        f"Could not extract {archive_path} into {extracted_path}")
