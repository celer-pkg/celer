# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
import functools
import re
from typing import TYPE_CHECKING, Any, Optional

from typing_extensions import override

from crossbench.plt.device_info import DeviceInfo
from crossbench.plt.macos import MacOSPlatform

if TYPE_CHECKING:
  from crossbench.plt.base import CPUFreqInfo, Platform
  from crossbench.plt.display_info import DisplayInfo

pattern: re.Pattern[str] = re.compile(
    r"(?P<name>[^\(\)]+) \((?P<version>[0-9\.]+)\) (- Connecting )?"
    r"\((?P<udid>[0-9A-Z-]+)\)")


@dataclasses.dataclass(frozen=True)
class IOSDeviceInfo(DeviceInfo):
  version: str = ""

  @property
  def udid(self) -> str:
    return self.device_id

  def __str__(self) -> str:
    return f"{self.name} ({self.version}) ({self.udid})"


def ios_devices(platform: Platform,
                show_all: bool = False) -> dict[str, IOSDeviceInfo]:
  output = platform.sh_stdout("xcrun", "xctrace", "list", "devices")
  category_index = 0
  results: dict[str, IOSDeviceInfo] = {}
  for line in output.splitlines():
    if line.startswith("== "):
      category_index += 1
      continue
    if category_index > 1 and not show_all:
      return results
    for match in pattern.finditer(line):
      device = IOSDeviceInfo(
          match.group("udid"), match.group("name"), match.group("version"))
      if device.udid in results:
        raise ValueError("Invalid UDID")
      results[device.udid] = device
  return results


# TODO: consider using some abstract MacOS base class.
# TODO: consider using https://github.com/facebook/idb
# TODO: implement mocked methods
# TODO: Follow remove-posix pattern and redirect all shell commands to the
#       host platform.
class IOSPlatform(MacOSPlatform):

  def __init__(self,
               host_platform: Platform,
               device_identifier: Optional[str] = None) -> None:
    assert not host_platform.is_remote, (
        "ios on remote platform is not supported yet")
    self._host_platform: Platform = host_platform
    super().__init__()
    self._device: IOSDeviceInfo = self._find_ios_device(device_identifier)

  def _find_ios_device(
      self, device_identifier: Optional[str] = None) -> IOSDeviceInfo:
    devices: dict[str, IOSDeviceInfo] = ios_devices(self._host_platform)
    if not devices:
      raise ValueError("No devices attached.")
    if not device_identifier:
      if len(devices) != 1:
        raise ValueError(
            f"Too many devices attached, please specify one of: {devices}")
      return list(devices.values())[0]
    if device := devices.get(device_identifier):
      return device
    matches: list[IOSDeviceInfo] = []
    for device in devices.values():
      if device_identifier in device.name:
        matches.append(device)
    if not matches:
      raise ValueError(
          f"No matching device for device identifier: {device_identifier}, "
          f"choices are {devices}")
    if len(matches) > 1:
      raise ValueError(
          f"Found {len(matches)} devices matching: '{device_identifier}'.\n"
          f"Choices: {matches}")
    return matches[0]

  @property
  def udid(self) -> str:
    return self._device.udid

  @property
  @override
  def name(self) -> str:
    return "ios"

  @property
  @override
  def device(self) -> str:
    return self._device.name

  @property
  @override
  def cpu(self) -> str:
    return "ios-arm64"

  @property
  @override
  def version(self) -> str:
    return self._device.version

  @functools.lru_cache(maxsize=1)
  @override
  def cpu_details(self) -> dict[str, Any]:
    # TODO: Implement properly (i.e. remove all n/a values)
    return {
        "info": self.cpu,
        "physical cores": self.cpu_cores(logical=False),
        "logical cores": self.cpu_cores(logical=True),
        "usage": "n/a",
        "total usage": "n/a",
        "system load": "n/a",
        "min frequency": "n/a",
        "max frequency": "n/a",
        "current frequency": "n/a",
    }

  @override
  def cpu_cores(self, logical: bool) -> int:  #type: ignore[override]
    return 0

  @override
  def _cpu_freq(self) -> Optional[CPUFreqInfo]:
    return None

  def get_relative_cpu_speed(self) -> float:
    return 1.0

  def display_details(self) -> tuple[DisplayInfo, ...]:  #type: ignore[override]
    return tuple()

  @override
  def _macos_system_details(self) -> dict[str, Any]:
    return {}
