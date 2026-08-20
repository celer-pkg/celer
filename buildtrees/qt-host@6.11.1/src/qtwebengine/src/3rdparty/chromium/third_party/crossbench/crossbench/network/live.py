# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import contextlib
from typing import TYPE_CHECKING, Iterator, TypeVar

from typing_extensions import override

from crossbench.network.base import Network

if TYPE_CHECKING:
  from crossbench.runner.groups.session import BrowserSessionRunGroup

LiveNetworkT = TypeVar("LiveNetworkT", bound="LiveNetwork")

class LiveNetwork(Network):

  @property
  @override
  def is_live(self) -> bool:
    return True

  @contextlib.contextmanager
  @override
  def open(self: LiveNetworkT,
           session: BrowserSessionRunGroup) -> Iterator[LiveNetworkT]:
    with super().open(session):
      with self._traffic_shaper.open(self, session):
        yield self

  def __str__(self) -> str:
    return f"LIVE(speed={self.traffic_shaper})"
