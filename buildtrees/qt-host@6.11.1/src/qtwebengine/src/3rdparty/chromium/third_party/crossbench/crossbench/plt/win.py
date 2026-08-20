# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import functools
import logging
import os
import shutil
from typing import Optional, Type

from typing_extensions import override

from crossbench import path as pth
from crossbench.plt.base import Platform
from crossbench.plt.signals import WinSignals


class WinPlatform(Platform):
  # TODO: support remote platforms
  SEARCH_PATHS = (
      pth.LocalPath("."),
      pth.LocalPath(os.path.expandvars("%ProgramFiles%")),
      pth.LocalPath(os.path.expandvars("%ProgramFiles(x86)%")),
      pth.LocalPath(os.path.expandvars("%APPDATA%")),
      pth.LocalPath(os.path.expandvars("%LOCALAPPDATA%")),
  )

  @property
  def signals(self) -> Type[WinSignals]:
    return WinSignals

  @property
  @override
  def is_win(self) -> bool:
    return True

  @property
  @override
  def name(self) -> str:
    return "win"

  @property
  @override
  def device(self) -> str:
    # TODO: implement
    return ""

  def cmd_stdout(self, *args, **kwargs) -> str:
    cmd = ["cmd", "/c", *args]
    return self.sh_stdout(*cmd, **kwargs)

  def powershell_stdout(self, *args, **kwargs) -> str:
    cmd = ["powershell", "-c", *args]
    return self.sh_stdout(*cmd, **kwargs)


  @functools.cached_property
  @override
  def version(self) -> str:  #pylint: disable=invalid-overridden-method
    return self.cmd_stdout("ver").strip()

  @functools.cached_property
  @override
  def cpu(self) -> str:  #pylint: disable=invalid-overridden-method
    return self.powershell_stdout(
        "Get-CIMInstance -query 'select * from Win32_Processor' | ft Name"
    ).strip().splitlines()[2].strip()

  @functools.lru_cache(maxsize=1)
  @override
  def _raw_machine_arch(self) -> str:
    self.assert_is_local()
    # The method in base class doesn't always give the correct answer,
    # because it uses py_platform.machine, which give the architecture of
    # the Python binary. It is possible to run x64 Python on ARM Windows.
    cpu_caption = self.powershell_stdout(
        "Get-CIMInstance -query 'select * from Win32_Processor' | ft Caption"
    ).strip().splitlines()[2].strip().lower()
    if cpu_caption.startswith("arm"):
      return "arm64" if "64-bit" in cpu_caption else "arm"
    return super()._raw_machine_arch()

  @override
  def uptime(self) -> dt.timedelta:
    """Parse powershell last boot time time-span into a timedelta object.
    Example Output:
    Days              : 14
    Hours             : 2
    Minutes           : 19
    Seconds           : 54
    Milliseconds      : 978
    Ticks             : 12179949789862
    TotalDays         : 14.0971641086366
    TotalHours        : 338.331938607278
    TotalMinutes      : 20299.9163164367
    TotalSeconds      : 1217994.9789862
    TotalMilliseconds : 1217994978.9862
    """
    uptime_output = self.powershell_stdout(
        "(New-TimeSpan -Start ("
        "Get-CimInstance Win32_OperatingSystem).LastBootUpTime"
        ")")
    results = {}
    for line in uptime_output.splitlines():
      line = line.strip()
      if not line:
        continue
      unit, value = line.split(":", maxsplit=1)
      unit = unit.strip()
      value_f: float = float(value)
      results[unit] = value_f
    return dt.timedelta(
        days=results["Days"],
        hours=results["Hours"],
        minutes=results["Minutes"],
        seconds=results["Seconds"],
        milliseconds=results["Milliseconds"])

  @override
  def search_binary(self, app_or_bin: pth.AnyPathLike) -> Optional[pth.AnyPath]:
    self.assert_is_local()
    app_or_bin_path: pth.AnyPath = self.path(app_or_bin)
    if not app_or_bin_path.parts:
      raise ValueError("Got empty path")
    if app_or_bin_path.suffix.lower() not in (".exe", ".bat"):
      raise ValueError("Expected executable path with '.exe' or '.bat' suffix, "
                       f"but got: '{app_or_bin_path.name}'")
    if result_path := self.which(app_or_bin):
      assert self.exists(result_path), f"{result_path} does not exist."
      return result_path
    for path in self.SEARCH_PATHS:
      # Recreate Path object for easier pyfakefs testing
      result_path = self.path(path) / app_or_bin
      if self.exists(result_path):
        return result_path
    return None

  @override
  def app_version(self, app_or_bin: pth.AnyPathLike) -> str:
    app_or_bin = self.path(app_or_bin)
    if not self.exists(app_or_bin):
      raise ValueError(f"Binary {app_or_bin} does not exist.")
    if version := self.sh_stdout(
        "powershell", "-command",
        f"(Get-Item '{app_or_bin}').VersionInfo.ProductVersion").strip():
      name = self.sh_stdout(
          "powershell", "-command",
          f"(Get-Item '{app_or_bin}').VersionInfo.ProductName").strip()
      return f"{name} {version}"
    try:
      # Fall back to command-line tools.
      if version := self.sh_stdout(app_or_bin, "--version").strip():
        return version
    except Exception as e:  # pylint: disable=broad-exception-caught
      logging.debug("Failed to extract binary tool version: %s", e)
    raise ValueError(f"Could not extract version for {app_or_bin}")


  @override
  def symlink_or_copy(self, src: pth.AnyPathLike,
                      dst: pth.AnyPathLike) -> pth.AnyPath:
    """Windows does not support symlinking without admin support.
    Copy files on windows but symlink everywhere else (see base Platform)."""
    self.assert_is_local()
    dst_path = self.path(dst)
    shutil.copy(os.fspath(self.path(src)), os.fspath(dst_path))
    return dst_path
