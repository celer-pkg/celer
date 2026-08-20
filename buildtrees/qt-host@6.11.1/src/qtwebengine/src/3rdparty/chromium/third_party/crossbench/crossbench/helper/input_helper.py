# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import threading


def input_with_timeout(
    timeout: dt.timedelta = dt.timedelta(seconds=10), default=None):
  result_container = [default]
  wait = threading.Thread(
      target=_input, args=[
          result_container,
      ])
  wait.daemon = True
  wait.start()
  wait.join(timeout=timeout.total_seconds())
  return result_container[0]


def _input(results_container) -> None:
  try:
    results_container[0] = input()
  except KeyboardInterrupt:
    pass
