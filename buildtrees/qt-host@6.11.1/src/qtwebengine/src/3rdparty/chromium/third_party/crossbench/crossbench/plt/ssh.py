# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import TYPE_CHECKING, Mapping, Optional

from crossbench.plt.port_manager import PortManager
from crossbench.plt.remote import RemotePlatformMixin

if TYPE_CHECKING:
  import subprocess

  from crossbench.plt.base import Platform
  from crossbench.plt.types import CmdArg, ListCmdArgs


class SshPortManager(PortManager):
  pass

class SshPlatformMixin(RemotePlatformMixin, metaclass=abc.ABCMeta):

  def __init__(self, host_platform: Platform, host: str, port: int,
               ssh_port: int, ssh_user: str) -> None:
    super().__init__(host_platform)
    self._host = host
    self._port = port
    self._ssh_port = ssh_port
    self._ssh_user = ssh_user

  @abc.abstractmethod
  def _create_port_manager(self) -> PortManager:
    pass

  @property
  def host(self) -> str:
    return self._host

  @property
  def port(self) -> int:
    return self._port

  @property
  def ssh_user(self) -> str:
    return self._ssh_user

  @property
  def ssh_port(self) -> int:
    return self._ssh_port

  @property
  def is_remote_ssh(self) -> bool:
    return True

  @abc.abstractmethod
  def build_ssh_cmd(self, *args: CmdArg, shell: bool = False) -> ListCmdArgs:
    pass

  def sh_stdout_bytes(self,
                      *args: CmdArg,
                      shell: bool = False,
                      quiet: bool = False,
                      stdin=None,
                      env: Optional[Mapping[str, str]] = None,
                      check: bool = True) -> bytes:
    ssh_cmd: ListCmdArgs = self.build_ssh_cmd(*args, shell=shell)
    return self._host_platform.sh_stdout_bytes(
        *ssh_cmd, shell=False, quiet=quiet, stdin=stdin, env=env, check=check)

  def sh(self,
         *args: CmdArg,
         shell: bool = False,
         capture_output: bool = False,
         stdout=None,
         stderr=None,
         stdin=None,
         env: Optional[Mapping[str, str]] = None,
         quiet: bool = False,
         check: bool = True) -> subprocess.CompletedProcess:
    ssh_cmd: ListCmdArgs = self.build_ssh_cmd(*args, shell=shell)
    return self._host_platform.sh(
        *ssh_cmd,
        shell=shell,
        capture_output=capture_output,
        stdout=stdout,
        stderr=stderr,
        stdin=stdin,
        env=env,
        quiet=quiet,
        check=check)

  def popen(self,
            *args: CmdArg,
            bufsize: int = -1,
            shell: bool = False,
            stdout=None,
            stderr=None,
            stdin=None,
            env: Optional[Mapping[str, str]] = None,
            quiet: bool = False) -> subprocess.Popen:
    ssh_cmd: ListCmdArgs = self.build_ssh_cmd(*args, shell=shell)
    return self._host_platform.popen(
        *ssh_cmd,
        shell=False,
        bufsize=bufsize,
        stdout=stdout,
        stderr=stderr,
        stdin=stdin,
        env=env,
        quiet=quiet)
