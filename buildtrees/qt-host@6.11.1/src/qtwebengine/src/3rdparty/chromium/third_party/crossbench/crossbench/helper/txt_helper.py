# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import textwrap
from typing import Iterable, Type


def wrap_lines(body: str, width: int = 80, indent: str = "") -> Iterable[str]:
  for line in body.splitlines():
    if len(line) <= width:
      yield f"{indent}{line}"
      continue
    for split in textwrap.wrap(line, width):
      yield f"{indent}{split}"


def type_name(t: Type) -> str:
  module = t.__module__
  if not module:
    return t.__qualname__
  return f"{module}.{t.__qualname__}"
