# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
import os
from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from crossbench.path import LocalPath


class ChangeCWD:

  def __init__(self, destination: LocalPath) -> None:
    self.new_dir = destination
    self.prev_dir: str | None = None

  def __enter__(self) -> None:
    self.prev_dir = os.getcwd()
    os.chdir(self.new_dir)
    logging.debug("CWD=%s", self.new_dir)

  def __exit__(self, exc_type, exc_value, exc_traceback) -> None:
    assert self.prev_dir, "ChangeCWD was not entered correctly."
    os.chdir(self.prev_dir)
