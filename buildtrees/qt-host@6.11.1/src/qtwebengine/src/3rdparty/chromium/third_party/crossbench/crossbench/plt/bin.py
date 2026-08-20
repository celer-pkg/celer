# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
from typing import TYPE_CHECKING, Iterable, Optional, TypeAlias

from typing_extensions import override

from crossbench import path as pth

if TYPE_CHECKING:
  from crossbench.plt.base import Platform
  BinaryLookup: TypeAlias = pth.AnyPathLike | Iterable[pth.AnyPathLike]



class BinaryNotFoundError(RuntimeError):

  def __init__(self, binary: Binary, platform: Platform) -> None:
    self.binary = binary
    self.platform = platform
    super().__init__(self._create_message())

  def _create_message(self) -> str:
    return (f"Could not find binary '{self.binary}' on {self.platform}. "
            f"Please install {self.binary.name} or use the "
            f"--bin-{self.binary.name} "
            "command line flag to manually specify a path.")


class UnsupportedPlatformError(BinaryNotFoundError):

  def __init__(self, binary: Binary, platform: Platform, expected: str) -> None:
    self.expected_platform_name: str = expected
    super().__init__(binary, platform)

  @override
  def _create_message(self) -> str:
    return (f"Could not find binary '{self.binary}' on {self.platform}. "
            f"Only supported on {self.expected_platform_name}")


class Binary:
  """A binary abstraction for multiple platforms.
  Use this implementation to define binaries that exist on multiple platforms.
  For platform-specific binaries use subclasses of Binary."""

  def __init__(self,
               name: str,
               default: Optional[BinaryLookup] = None,
               posix: Optional[BinaryLookup] = None,
               linux: Optional[BinaryLookup] = None,
               android: Optional[BinaryLookup] = None,
               macos: Optional[BinaryLookup] = None,
               win: Optional[BinaryLookup] = None,
               chromeos: Optional[BinaryLookup] = None) -> None:
    self._name = name
    self._default = self._convert(default)
    self._posix = self._convert(posix)
    self._linux = self._convert(linux)
    self._android = self._convert(android)
    self._macos = self._convert(macos)
    self._win = self._convert(win)
    self._validate_win()
    self._chromeos = self._convert(chromeos)
    if not any((chromeos, default, posix, linux, android, macos, win)):
      raise ValueError("At least one platform binary must be provided")

  def _convert(self,
               paths: Optional[BinaryLookup] = None) -> tuple[pth.AnyPath, ...]:
    if paths is None:
      return tuple()
    if isinstance(paths, str):
      path: str = paths
      if not path:
        raise ValueError("Got unexpected empty string as binary path")
      paths = [path]
    elif isinstance(paths, pth.AnyPath):
      paths = [paths]
    return tuple(pth.AnyPath(path) for path in paths)

  def _validate_win(self) -> None:
    for path in self._win:
      if path.suffix != ".exe":
        raise ValueError(f"Windows binary {path} should have '.exe' suffix")

  @property
  def name(self) -> str:
    return self._name

  def __str__(self) -> str:
    return self._name

  @functools.cache  # pylint: disable=method-cache-max-size-none
  def resolve_cached(self, platform: Platform) -> pth.AnyPath:
    return self.resolve(platform)

  def resolve(self, platform: Platform) -> pth.AnyPath:
    self._validate_platform(platform)
    for binary in self.platform_path(platform):
      binary_path = platform.path(binary)
      if result := platform.search_binary(binary_path):
        return result
    raise BinaryNotFoundError(self, platform)

  def platform_path(self, platform: Platform) -> tuple[pth.AnyPath, ...]:
    if self._chromeos and platform.is_chromeos:
      return self._chromeos
    if self._linux and platform.is_linux:
      return self._linux
    if self._android and platform.is_android:
      return self._android
    if self._macos and platform.is_macos:
      return self._macos
    if self._posix and platform.is_posix:
      return self._posix
    if platform.is_win:
      if self._win:
        return self._win
      if self._default:
        return self._win_default()
    return self._default

  def _win_default(self) -> tuple[pth.AnyPath, ...]:
    return tuple(
        default if default.suffix == ".exe" else default.with_suffix(".exe")
        for default in self._default)

  def _validate_platform(self, platform: Platform) -> None:
    pass


class PosixBinary(Binary):

  def __init__(self, name: pth.AnyPathLike) -> None:
    super().__init__(pth.AnyPosixPath(name).name, posix=name)

  @override
  def _validate_platform(self, platform: Platform) -> None:
    if not platform.is_posix:
      raise UnsupportedPlatformError(self, platform, "posix")


class MacOsBinary(Binary):

  def __init__(self, name: pth.AnyPathLike) -> None:
    super().__init__(pth.AnyPosixPath(name).name, macos=name)

  @override
  def _validate_platform(self, platform: Platform) -> None:
    if not platform.is_macos:
      raise UnsupportedPlatformError(self, platform, "macos")


class LinuxBinary(Binary):

  def __init__(self, name: pth.AnyPathLike) -> None:
    super().__init__(pth.AnyPosixPath(name).name, linux=name)

  @override
  def _validate_platform(self, platform: Platform) -> None:
    if not platform.is_posix:
      raise UnsupportedPlatformError(self, platform, "linux")


class AndroidBinary(Binary):

  def __init__(self, name: pth.AnyPathLike) -> None:
    super().__init__(pth.AnyPosixPath(name).name, android=name)

  @override
  def _validate_platform(self, platform: Platform) -> None:
    if not platform.is_android:
      raise UnsupportedPlatformError(self, platform, "android")


class WinBinary(Binary):

  def __init__(self, name: pth.AnyPathLike) -> None:
    super().__init__(pth.AnyWindowsPath(name).name, win=name)

  @override
  def _validate_platform(self, platform: Platform) -> None:
    if not platform.is_win:
      raise UnsupportedPlatformError(self, platform, "windows")


class ChromeOSBinary(Binary):

  def __init__(self, name: pth.AnyPathLike) -> None:
    super().__init__(pth.AnyPosixPath(name).name, chromeos=name)

  @override
  def _validate_platform(self, platform: Platform) -> None:
    if not platform.is_chromeos:
      raise UnsupportedPlatformError(self, platform, "chromeos")


class Binaries:
  ADB = Binary("adb", default="adb", win="adb.exe")
  CPIO = LinuxBinary("cpio")
  FFMPEG = Binary("ffmpeg", posix="ffmpeg")
  GCERTSTATUS = Binary("gcertstatus", posix="gcertstatus")
  GO = Binary("go", posix="go")
  GSUTIL = Binary("gsutil", posix="gsutil")
  LSCPU = LinuxBinary("lscpu")
  MONTAGE = Binary("montage", posix="montage")
  ON_AC_POWER = LinuxBinary("on_ac_power")
  PERF = LinuxBinary("perf")
  PPROF = LinuxBinary("pprof")
  PYTHON3 = Binary("python3", default="python3", win="python3.exe")
  RPM2CPIO = LinuxBinary("rpm2cpio")
  SIMPLEPERF = AndroidBinary("simpleperf")
  XCTRACE = MacOsBinary("xctrace")
  CHROMEDRIVER = Binary(
      "chromedriver",
      chromeos="/usr/local/chromedriver/chromedriver",
      linux="chromedriver")


class Browsers:
  SAFARI = MacOsBinary("Safari.app")
  SAFARI_TECH_PREVIEW = MacOsBinary("Safari Technology Preview.app")
  FIREFOX_STABLE = Binary(
      "firefox stable",
      macos="Firefox.app",
      linux="firefox",
      win="Mozilla Firefox/firefox.exe")
  FIREFOX_DEV = Binary(
      "firefox developer edition",
      macos="Firefox Developer Edition.app",
      linux="firefox-developer-edition",
      win="Firefox Developer Edition/firefox.exe")
  FIREFOX_NIGHTLY = Binary(
      "Firefox nightly",
      macos="Firefox Nightly.app",
      linux=["firefox-nightly", "firefox-trunk"],
      win="Firefox Nightly/firefox.exe")
