# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
import datetime as dt
import functools
import os
import re
from typing import TYPE_CHECKING, Any, ClassVar, Iterator, Optional, Type

from typing_extensions import override

from crossbench import path as pth
from crossbench.parse import NumberParser
from crossbench.plt.base import SubprocessError
from crossbench.plt.posix import PosixPlatform
from crossbench.plt.process_meminfo import ProcessMeminfo
from crossbench.plt.remote import RemotePlatformMixin
from crossbench.plt.signals import LinuxSignals

if TYPE_CHECKING:
  from crossbench.plt.display_info import DisplayInfo


SCRIPTS_DIR = pth.LocalPath(__file__).parent / "remote_scripts"

@dataclasses.dataclass
class XrandrDisplayInfo:
  RESOLUTION_RE: ClassVar[re.Pattern] = re.compile(
      r"(?P<resX>[0-9]+)x(?P<resY>[0-9]+)")
  REFRESH_RATE_RE: ClassVar[re.Pattern] = re.compile(r"(?P<freq>[0-9.]+)\*")

  header: str
  resolutions: list[str] = dataclasses.field(default_factory=list)

  def is_connected(self) -> bool:
    return "disconnected" not in self.header

  def resolution(self) -> tuple[int, int] | None:
    if match := self.RESOLUTION_RE.search(self.header):
      return (NumberParser.positive_int(match.group("resX")),
              NumberParser.positive_int(match.group("resY")))
    return None

  def refresh_rate(self) -> float:
    for resolution in self.resolutions:
      # The current refresh ret is marked with a `*`:
      if match := self.REFRESH_RATE_RE.search(resolution):
        return NumberParser.positive_float(match.group("freq"))
    return -1


def parse_display_xrandr(xrandr_str: str) -> Iterator[DisplayInfo]:
  """ Parse xrandr output:
  Screen 0: minimum 64 x 64, current 1728 x 946, maximum 32767 x 32767
  DUMMY0 connected primary 1728x946+0+0 456mm x 249mm
    1024x768      60.00  
    1024x576      59.90
    CRD_78       120.00* 
    ...
  DUMMY1 disconnected
    1600x1200_60  60.00
    ...
  """
  display_infos: list[XrandrDisplayInfo] = []
  current_info: XrandrDisplayInfo | None = None
  # Group display info and resolution entries:
  for line in xrandr_str.splitlines():
    if "connected" in line:
      current_info = XrandrDisplayInfo(line)
      display_infos.append(current_info)
    if current_info and line.startswith(" "):
      current_info.resolutions.append(line.strip())
  # Filter by connected displays and extract the resolution.
  for display_info in display_infos:
    if not display_info.is_connected():
      continue
    if resolution := display_info.resolution():
      yield {
          "resolution": resolution,
          "refresh_rate": display_info.refresh_rate(),
      }


class LinuxPlatform(PosixPlatform):
  SEARCH_PATHS: tuple[pth.AnyPath, ...] = (
      pth.AnyPosixPath("."),
      pth.AnyPosixPath("/usr/local/sbin"),
      pth.AnyPosixPath("/usr/local/bin"),
      pth.AnyPosixPath("/usr/sbin"),
      pth.AnyPosixPath("/usr/bin"),
      pth.AnyPosixPath("/sbin"),
      pth.AnyPosixPath("/bin"),
      pth.AnyPosixPath("/opt/google"),
  )

  @property
  @override
  def is_linux(self) -> bool:
    return True

  @property
  @override
  def name(self) -> str:
    return "linux"

  @property
  def signals(self) -> Type[LinuxSignals]:
    return LinuxSignals

  def check_system_monitoring(self, disable: bool = False) -> bool:
    return True

  @functools.cached_property
  @override
  def device(self) -> str:  #pylint: disable=invalid-overridden-method
    try:
      id_dir = self.path("/sys/devices/virtual/dmi/id")
      vendor = self.cat(id_dir / "sys_vendor").strip()
      product = self.cat(id_dir / "product_name").strip()
      return f"{vendor} {product}"
    except (FileNotFoundError, SubprocessError):
      return "UNKNOWN"

  @property
  @override
  def has_display(self) -> bool:
    return "DISPLAY" in os.environ

  @property
  @override
  def is_battery_powered(self) -> bool:
    if self.is_local:
      return super().is_battery_powered
    if on_ac_power := self.which("on_ac_power"):
      return self.sh(on_ac_power, check=False).returncode == 1
    return False

  @functools.lru_cache(maxsize=1)
  @override
  def system_details(self) -> dict[str, Any]:
    details = super().system_details()
    for info_bin in ("lscpu", "inxi"):
      if info_bin_path := self.which(info_bin):
        details[info_bin] = self.sh_stdout(info_bin_path)
    return details

  def search_binary(self, app_or_bin: pth.AnyPathLike) -> Optional[pth.AnyPath]:
    app_or_bin_path: pth.AnyPath = self.path(app_or_bin)
    if not app_or_bin_path.parts:
      raise ValueError("Got empty path")
    if result_path := self.which(app_or_bin_path):
      if not self.exists(result_path):
        raise RuntimeError(f"{result_path} does not exist.")
      return result_path
    for path in self.SEARCH_PATHS:
      # Recreate Path object for easier pyfakefs testing
      result_path = self.path(path) / app_or_bin_path
      if self.exists(result_path):
        return result_path
    return None

  def screenshot(self, result_path: pth.AnyPath) -> None:
    # TODO: maybe use imagemagick's 'import' as more portable alternative
    self.sh("gnome-screenshot", "--file", result_path)

  @functools.lru_cache(maxsize=1)
  def display_details(self) -> tuple[DisplayInfo, ...]:
    if not self.has_display:
      return tuple()
    if xrandr_str := self.sh_stdout("xrandr"):
      return tuple(parse_display_xrandr(xrandr_str))
    return tuple()

  _MEMINFO_SCRIPT_PROCESS_PATTERN = re.compile(r"==== process (\d+) ====")
  _MEMINFO_SCRIPT_SMAPS_HEADER_PATTERN = re.compile(r"==== smaps_rollup ====")
  _SMAPS_ROLLUP_PATTERN = re.compile(
      r".*Rss:\s+(?P<rss_total>\d+) kB.*"
      r"Pss:\s+(?P<pss_total>\d+) kB.*"
      r"Swap:\s+(?P<swap_total>\d+)",
      flags=re.DOTALL)

  @override
  def process_meminfo(
      self, process_name: str, timeout: dt.timedelta = dt.timedelta(seconds=10)
  ) -> list[ProcessMeminfo]:
    del timeout

    script = (SCRIPTS_DIR / "meminfo.sh").read_text()

    with self.NamedTemporaryFile() as script_file:
      self.write_text(script_file, script)
      # Script outputs the following format repeated per process:
      # ==== process <pid> ====
      # <proc/cmdline>
      # ==== smaps_rollup ====
      # <proc/smaps_rollup>
      output = self.sh_stdout("bash", str(script_file), process_name)
      processes = self._MEMINFO_SCRIPT_PROCESS_PATTERN.split(output)[1:]
      # processes even indices are pids, the odd indices after is the output for
      # that pid.
      meminfos: list[ProcessMeminfo] = []
      for i in range(0, len(processes), 2):
        pid = int(processes[i])
        [cmdline, smaps_rollup
        ] = self._MEMINFO_SCRIPT_SMAPS_HEADER_PATTERN.split(processes[i + 1])
        match = self._SMAPS_ROLLUP_PATTERN.search(smaps_rollup)
        assert match
        meminfos.append(
            ProcessMeminfo(
                pid=pid,
                name=cmdline.strip(),
                pss_total=int(match["pss_total"]),
                rss_total=int(match["rss_total"]),
                swap_total=int(match["swap_total"])))
      return meminfos


class RemoteLinuxPlatform(RemotePlatformMixin, LinuxPlatform):
  pass
