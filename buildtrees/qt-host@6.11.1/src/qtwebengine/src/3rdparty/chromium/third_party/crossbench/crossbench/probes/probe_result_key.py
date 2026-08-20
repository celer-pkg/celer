# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import typing


class ProbeResultKey(typing.Protocol):

  @property
  def name(self) -> str:
    raise NotImplementedError()
