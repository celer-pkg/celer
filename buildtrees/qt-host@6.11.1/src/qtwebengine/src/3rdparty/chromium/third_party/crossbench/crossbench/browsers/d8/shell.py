# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import enum
import logging
import queue
import shlex
import subprocess
import threading
import time
from typing import TYPE_CHECKING, Final, Optional, Sequence

from crossbench.helper.state import BaseState, StateMachine

if TYPE_CHECKING:
  import crossbench.path as pth
  from crossbench import plt

PROMPT: Final[str] = "d8> "
READ_LEN: Final[int] = len(PROMPT)
DEFAULT_TIMEOUT = dt.timedelta(seconds=10)


class BackgroundReader(threading.Thread):

  def __init__(self, stream, read_len: int) -> None:
    super().__init__()
    self.print_output: bool = False
    self.daemon = True
    self._queue: queue.Queue[str] = queue.Queue()
    self._stream = stream
    self._read_len: int = read_len

  def run(self) -> None:
    while True:
      data = self._stream.readline(self._read_len)
      if data:
        if self.print_output:
          print(data, end="")
        self._queue.put(data)

  def get(self, timeout: Optional[float] = None) -> str:
    return self._queue.get(timeout=timeout)


@enum.unique
class State(BaseState):
  INITIAL = enum.auto()
  WAIT_FOR_INPUT = enum.auto()
  WAIT_FOR_OUTPUT = enum.auto()


class D8Shell:
  # pylint: disable=redefined-builtin

  def __init__(self,
               platform: plt.Platform,
               d8_bin: pth.LocalPath,
               flags: Sequence[str] = tuple(),
               cwd: Optional[pth.LocalPath] = None):
    self._state = StateMachine(State.INITIAL)
    self._platform = platform
    assert platform.is_local, (
        f"D8 only works on local platforms, but got {platform}")
    self._d8_bin: pth.LocalPath = d8_bin
    self._flags: Sequence[str] = flags
    self._cwd: pth.LocalPath | None = cwd
    self._poll_interval: float = 0.01
    cmd = [str(d8_bin), *flags]
    logging.debug("SHELL: %s", shlex.join(map(str, cmd)))
    logging.debug("CWD: %s", cwd)
    self._process = subprocess.Popen(  # pylint: disable=consider-using-with
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        cwd=self._cwd,
        encoding="utf-8",
        bufsize=1  # Line buffering
    )
    self._state.transition(State.INITIAL, to=State.WAIT_FOR_OUTPUT)
    if stdin := self._process.stdin:
      self._stdin = stdin
    else:
      raise RuntimeError("Could not start d8 with active stdin")
    if stdout := self._process.stdout:
      self._reader = BackgroundReader(stdout, READ_LEN)
    else:
      raise RuntimeError("Could not start d8 with active stdout")
    self._reader.start()
    self._version: str = self.read().strip()

  @property
  def version(self) -> str:
    return self._version

  @property
  def pid(self) -> int:
    return self._process.pid

  def quit(self) -> None:
    try:
      self.write("quit()")
    finally:
      self._platform.terminate(self._process)

  def read(self, timeout: Optional[dt.timedelta] = None) -> str:
    self._state.expect(State.WAIT_FOR_OUTPUT)
    if timeout is None:
      timeout = DEFAULT_TIMEOUT
    time_left: float = timeout.total_seconds()
    expected_end_time = time.time() + time_left

    buffer: list[str] = []
    while time_left > 0:
      try:
        data = self._reader.get(self._poll_interval)
        if data == PROMPT:
          break
        buffer.append(data)
      except queue.Empty:
        pass
      time_left = expected_end_time - time.time()

    if time_left > 0:
      self._state.transition(State.WAIT_FOR_OUTPUT, to=State.WAIT_FOR_INPUT)
      return ("".join(buffer))[:-1]
    raise TimeoutError(f"D8 timed out after {timeout.total_seconds()}s")

  def write(self, cmd: str, print_output: bool = False) -> None:
    self._state.expect(State.WAIT_FOR_INPUT)
    logging.debug("D8 CMD: %s", cmd)
    self._reader.print_output = print_output
    self._stdin.write(cmd)
    self._stdin.write("\n")
    self._stdin.flush()
    self._state.transition(State.WAIT_FOR_INPUT, to=State.WAIT_FOR_OUTPUT)

  def execute(self,
              cmd: str,
              eval: bool = False,
              print_output: bool = False,
              timeout: Optional[dt.timedelta] = None) -> str:
    if eval:
      cmd = f"eval({repr(cmd)})"
    self.write(cmd, print_output=print_output)
    return self.read(timeout)

  def load(self, file: pth.LocalPath) -> str:
    if not file.exists():
      raise RuntimeError(f"{file} does not exist")
    logging.debug("D8 load: %s", file)
    return self.execute(f"load({repr(str(file))})")
