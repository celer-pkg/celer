# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
from typing import TYPE_CHECKING, cast

from typing_extensions import override

from crossbench.parse import NumberParser
from crossbench.plt.port_manager import PortManager
from crossbench.plt.ssh import SshPlatformMixin

if TYPE_CHECKING:
  import subprocess

  from crossbench.plt.base import Platform
  from crossbench.plt.types import CmdArg, ListCmdArgs


class SshPortManager(PortManager):
  PORT_FORWARDING_TIMEOUT = dt.timedelta(seconds=10)

  def __init__(self, platform: Platform) -> None:
    assert isinstance(platform, SshPlatformMixin)
    super().__init__(platform)
    self._forward_popens: dict[int, subprocess.Popen] = {}
    self._reverse_forward_popens: dict[int, subprocess.Popen] = {}

  def _build_ssh_cmd(self, *args: CmdArg, shell: bool = False) -> ListCmdArgs:
    return cast(SshPlatformMixin, self.platform).build_ssh_cmd(
        *args, shell=shell)

  @property
  def host_platform(self) -> Platform:
    return self._platform.host_platform

  @override
  def forward(self, local_port: int, remote_port: int) -> int:
    local_port, remote_port = self._validate_forwarding_ports(
        local_port, remote_port)
    self._forward_popens[local_port] = self.host_platform.popen(
        *self._build_ssh_cmd("-NL", f"{local_port}:localhost:{remote_port}"))
    self.host_platform.wait_for_port(local_port, self.PORT_FORWARDING_TIMEOUT)
    logging.debug("Forwarded Remote Port: %s:%s <= %s:%s", self.host_platform,
                  local_port, self, remote_port)
    return local_port

  def _validate_forwarding_ports(self, local_port: int,
                                 remote_port: int) -> tuple[int, int]:
    local_port = NumberParser.positive_zero_int(local_port, "local_port")
    remote_port = NumberParser.port_number(remote_port, "remote_port")
    if not local_port:
      local_port = self.host_platform.get_free_port()
    if local_port in self._forward_popens:
      raise RuntimeError(f"Cannot forward local port {local_port} twice.")
    return local_port, remote_port

  @override
  def stop_forward(self, local_port: int) -> None:
    self._forward_popens.pop(local_port).terminate()

  @override
  def reverse_forward(self, remote_port: int, local_port: int) -> int:
    # TODO: this should likely match with adb, where we support 0
    # for auto-allocating a remote_port
    remote_port, local_port = self._validate_reverse_forwarding_ports(
        remote_port, local_port)
    self._reverse_forward_popens[remote_port] = self.host_platform.popen(
        *self._build_ssh_cmd("-NR", f"{remote_port}:localhost:{local_port}"))
    self.platform.wait_for_port(remote_port, self.PORT_FORWARDING_TIMEOUT)
    logging.debug("Forwarded Local Port: %s:%s => %s:%s", self.host_platform,
                  local_port, self, remote_port)
    return remote_port

  def _validate_reverse_forwarding_ports(self, remote_port: int,
                                         local_port: int) -> tuple[int, int]:
    remote_port = NumberParser.port_number(remote_port, "remote_port")
    local_port = NumberParser.positive_zero_int(local_port, "local_port")
    if not local_port:
      local_port = self.host_platform.get_free_port()
    if remote_port in self._reverse_forward_popens:
      raise RuntimeError(f"Cannot forward remote port {remote_port} twice.")
    return remote_port, local_port

  @override
  def stop_reverse_forward(self, remote_port: int) -> None:
    self._reverse_forward_popens.pop(remote_port).terminate()
