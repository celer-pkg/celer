# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
import json
import logging
import subprocess
from typing import TYPE_CHECKING, Any, Optional

from typing_extensions import override

from crossbench import path as pth
from crossbench import plt
from crossbench.parse import NumberParser, ObjectParser
from crossbench.plt.linux_ssh import LinuxSshPlatform

if TYPE_CHECKING:
  from crossbench.plt.display_info import DisplayInfo
  from crossbench.plt.types import ListCmdArgs


class ChromeOsSshPlatform(LinuxSshPlatform):

  AUTOLOGIN_PATH = pth.AnyPosixPath("/usr/local/autotest/bin/autologin.py")
  DEVTOOLS_PORT_PATH = pth.AnyPosixPath("/home/chronos/DevToolsActivePort")

  def __init__(self, *args, enable_arc: bool = False, **kwargs) -> None:
    super().__init__(*args, **kwargs)
    self._enable_arc: bool = enable_arc
    self._username: str | None = None
    # `/tmp` on ChromeOS is mounted with `noexec` flag.
    # Instead, we use `/usr/local/tmp`, which allows executions of binaries.
    self._default_tmp_dir = pth.AnyPosixPath("/usr/local/tmp")

  @property
  @override
  def name(self) -> str:
    return "chromeos_ssh"

  @property
  def username(self) -> Optional[str]:
    return self._username

  @property
  def enable_arc(self) -> bool:
    return self._enable_arc

  @property
  @override
  def is_chromeos(self) -> bool:
    return True

  @property
  @override
  def has_display(self) -> bool:
    return True

  def create_debugging_session(self,
                               browser_flags: Optional[tuple[str, ...]] = None,
                               username: Optional[str] = None,
                               password: Optional[str] = None) -> int:
    disable_extensions_flag: str = "--disable-extensions"

    flags_for_session: list[str] = []

    if browser_flags:
      flags_for_session = list(browser_flags)

    try:
      args: ListCmdArgs = [self.AUTOLOGIN_PATH]
      if self.enable_arc:
        if disable_extensions_flag in flags_for_session:
          logging.warning(
              "'%s' is not compatible with ARC."
              " Proceeding without this flag.", disable_extensions_flag)
          flags_for_session.remove(disable_extensions_flag)
        args.append("--arc")
      if username and password:
        self._username = username
        args.extend(("-u", username, "-p", password))
      if flags_for_session:
        args.append("--")
        args.extend(flags_for_session)
      autologin_output = self.sh(
          *args, stdout=subprocess.PIPE,
          stderr=subprocess.STDOUT).stdout.decode("utf-8")
      logging.debug("Autologin Output:")
      logging.debug(autologin_output)
    except plt.SubprocessError as e:
      raise RuntimeError("Autologin failed.") from e
    try:
      dbg_port = self.cat(self.DEVTOOLS_PORT_PATH).splitlines()[0].strip()
    except plt.SubprocessError as e:
      raise RuntimeError("Could not read remote debugging port.") from e
    return int(dbg_port)

  @override
  def screenshot(self, result_path: pth.AnyPath) -> None:
    self.sh("screenshot", result_path)

  @functools.lru_cache(maxsize=1)
  @override
  def system_details(self) -> dict[str, Any]:
    details = super().system_details()

    details.update({
        "ChromeOS": self._parse_lsb_release(),
    })

    return details

  @functools.lru_cache(maxsize=1)
  def display_details(self) -> tuple[DisplayInfo, ...]:
    # TODO(405995421): add refresh rate and potentially support multiple
    # displays.
    return ({"resolution": self.display_resolution(), "refresh_rate": -1},)

  @override
  def display_resolution(self) -> tuple[int, int]:
    display_info_json = self.sh_stdout("cros-health-tool", "telem",
                                       "--category=display")
    display_info = json.loads(display_info_json)
    display_info = ObjectParser.dict(display_info, "display info")
    embedded_display = ObjectParser.dict(display_info.get("embedded_display"))
    resolution_horizontal = NumberParser.positive_int(
        embedded_display.get("resolution_horizontal"), "resolution_horizontal")
    resolution_vertical = NumberParser.positive_int(
        embedded_display.get("resolution_vertical"), "resolution_vertical")
    return (resolution_horizontal, resolution_vertical)

  def _parse_lsb_release(self) -> dict[str, str]:
    # lsb-release has the format:
    # KEY=VALUE
    result = {}
    for line in self.cat("/etc/lsb-release").splitlines():
      if "=" not in line:
        continue
      key, value = line.split("=", 1)
      result[key.strip()] = value.strip()

    return result
