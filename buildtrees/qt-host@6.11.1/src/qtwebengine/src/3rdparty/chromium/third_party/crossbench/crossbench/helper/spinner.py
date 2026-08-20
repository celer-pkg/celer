# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import sys
import threading
import time
from typing import Iterable

from crossbench.helper import terminal


class Spinner:
  CURSORS = "◐◓◑◒"

  def __init__(self, is_atty: bool, sleep: float, title: str) -> None:
    self._is_running: bool = False
    # Only enable the spinner if the output is an interactive terminal.
    self._is_atty: bool = is_atty
    self._sleep_time_seconds: float = sleep
    self._title: str = title
    self._message: str = ""
    self._cursor: str = " "

  def __enter__(self) -> None:
    if self._is_atty:
      self._is_running = True
      threading.Thread(target=self._spin).start()
    elif self._title:
      # Write single title line.
      self._write_message()

  def __exit__(self, exc_type, exc_value, traceback) -> None:
    self._is_running = False

  def _cursors(self) -> Iterable[str]:
    while True:
      yield from Spinner.CURSORS

  def _spin(self) -> None:
    for cursor in self._cursors():
      if not self._is_running:
        return
      self._cursor = cursor
      self._write_message()
      self._sleep()

  def _sleep(self) -> None:
    time.sleep(self._sleep_time_seconds)

  def write(self, message: str) -> None:
    self._message = message
    self._write_message()

  @property
  def title(self) -> str:
    return self._title

  @title.setter
  def title(self, title: str) -> None:
    self._title = title
    self._write_message()

  def _write_message(self) -> None:
    if self._is_atty:
      self._write_interactive_message()
    else:
      print(f"{self._title}{self._message}")

  def _write_interactive_message(self) -> None:
    stdout = sys.stdout
    stdout.write(f"{terminal.STORE_CURSOR_POS} {self._cursor} "
                 f"{self._title}{self._message}{terminal.CLEAR_END}"
                 f"{terminal.RESTORE_CURSOR_POS}")
    stdout.flush()
