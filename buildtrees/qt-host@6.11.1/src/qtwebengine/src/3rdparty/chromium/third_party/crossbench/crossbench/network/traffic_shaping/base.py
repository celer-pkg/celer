# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import contextlib
from typing import TYPE_CHECKING, Iterator, TypeVar

from crossbench.flags.base import Flags

if TYPE_CHECKING:
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.network.base import Network
  from crossbench.plt.base import Platform
  from crossbench.runner.groups.session import BrowserSessionRunGroup


TrafficShaperT = TypeVar("TrafficShaperT", bound="TrafficShaper")

class TrafficShaper(abc.ABC):

  def __init__(self, browser_platform: Platform) -> None:
    self._browser_platform = browser_platform
    self._is_running = False

  @property
  def browser_platform(self) -> Platform:
    return self._browser_platform

  @property
  def host_platform(self) -> Platform:
    return self._browser_platform.host_platform

  @property
  def is_live(self) -> bool:
    return False

  @property
  def is_running(self) -> bool:
    return self._is_running

  def extra_flags(self, browser_attributes: BrowserAttributes) -> Flags:
    del browser_attributes
    assert self.is_running, "TrafficShaper is not running."
    return Flags()

  @contextlib.contextmanager
  def open(self: TrafficShaperT, network: Network,
           session: BrowserSessionRunGroup) -> Iterator[TrafficShaperT]:
    del network, session
    assert not self._is_running, "Cannot start network more than once."
    self._is_running = True
    try:
      yield self
    finally:
      self._is_running = False

  @contextlib.contextmanager
  def pause(self):
    """Temporarily pause traffic shaping if supported."""
    yield None
