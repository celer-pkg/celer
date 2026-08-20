# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import subprocess
from typing import TYPE_CHECKING

from typing_extensions import override

if TYPE_CHECKING:
  from crossbench.path import AnyPathLike, LocalPath
  from crossbench.plt.base import Platform
  from crossbench.plt.signals import Signals
  from crossbench.plt.types import CmdArg, ListCmdArgs


class RemotePlatformMixin:

  def __init__(self, host_platform: Platform) -> None:
    super().__init__()
    self._host_platform: Platform = host_platform

  @property
  def is_remote(self) -> bool:
    return True

  @property
  def host_platform(self) -> Platform:
    return self._host_platform

  def host_path(self, path: AnyPathLike) -> LocalPath:
    return self._host_platform.local_path(path)

  def build_shell_cmd(self, *args: CmdArg, shell: bool = False) -> ListCmdArgs:
    raise NotImplementedError()


class RemotePopen(subprocess.Popen):
  """
  A wrapper class to represent a process running on a remote platform.

  Allows to send signals to the remote process and gracefully wait for its
  termination.
  """


  def __init__(self,
               platform: Platform,
               args: ListCmdArgs,
               bufsize: int = -1,
               stdout=None,
               stderr=None,
               stdin=None) -> None:
    self._platform: Platform = platform
    assert self._platform.is_remote, (
        f"Cannot create remote process on local platform {self._platform}")
    self._remote_pid: int | None = None
    super().__init__(
        args, bufsize=bufsize, stdout=stdout, stderr=stderr, stdin=stdin)

  def set_remote_pid(self, pid: int) -> None:
    assert self._remote_pid is None, "Should not set remote PID twice"
    self._remote_pid = pid

  @property
  def remote_pid(self) -> int:
    assert self._remote_pid, "remote process has no PID"
    return self._remote_pid

  @override
  def send_signal(self, signal: int | Signals) -> None:
    signal = self._platform.signals(signal)
    self._platform.send_signal(self.remote_pid, signal)

  @override
  def terminate(self) -> None:
    self._platform.terminate(self.remote_pid)

  @override
  def kill(self) -> None:
    self._platform.kill(self.remote_pid)
