# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
  from crossbench.runner.run import Run


class ActionRunnerListener:
  """Default empty ActionRunnerListener implementation."""

  def handle_error(self, run: Run, e: Exception) -> None:
    pass

  def handle_page_run(self, run: Run) -> None:
    pass

  def handle_new_tab(self, run: Run) -> None:
    pass
