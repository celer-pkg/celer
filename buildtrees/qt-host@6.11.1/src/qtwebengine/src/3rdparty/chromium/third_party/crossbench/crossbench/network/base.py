# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import contextlib
from typing import TYPE_CHECKING, Iterator, Optional, TypeVar

from crossbench import plt
from crossbench.network.traffic_shaping.live import NoTrafficShaper

if TYPE_CHECKING:
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.flags.base import Flags
  from crossbench.network.traffic_shaping.base import TrafficShaper
  from crossbench.runner.groups.session import BrowserSessionRunGroup


NetworkT = TypeVar("NetworkT", bound="Network")

class Network(abc.ABC):

  def __init__(self,
               traffic_shaper: Optional[TrafficShaper] = None,
               browser_platform: Optional[plt.Platform] = None) -> None:
    browser_platform = browser_platform or plt.PLATFORM
    self._traffic_shaper = traffic_shaper or NoTrafficShaper(browser_platform)
    self._browser_platform = browser_platform
    self._host_platform = browser_platform.host_platform
    self._is_running: bool = False

  @property
  def traffic_shaper(self) -> TrafficShaper:
    return self._traffic_shaper

  @property
  def browser_platform(self) -> plt.Platform:
    return self._browser_platform

  @property
  def host_platform(self) -> plt.Platform:
    return self._host_platform

  @property
  def is_running(self) -> bool:
    return self._is_running

  @property
  def is_live(self) -> bool:
    """Return True if the network is the default live/direct connection, as
    opposed to a replay network or local file server."""
    return False

  @property
  def is_wpr(self) -> bool:
    """Return True if the network is the replay network."""
    return False

  @property
  def is_local_file_server(self) -> bool:
    """Return True if the network is the local file server network."""
    return False

  @property
  def http_port(self) -> Optional[int]:
    """HTTP port for non-live server-based networks."""
    return None

  @property
  def https_port(self) -> Optional[int]:
    """HTTPS port for non-live server-based networks."""
    return None

  @property
  def host(self) -> Optional[str]:
    """Host for non-live server-based networks."""
    return None

  def extra_flags(self, browser_attributes: BrowserAttributes) -> Flags:
    assert self.is_running, "Network is not running."
    return self.traffic_shaper.extra_flags(browser_attributes)

  @contextlib.contextmanager
  def open(self: NetworkT,
           session: BrowserSessionRunGroup) -> Iterator[NetworkT]:
    del session
    assert not self._is_running, "Cannot start network more than once."
    self._is_running = True
    try:
      # Activate PortForwarder so we properly clean up pending ports in case of
      # errors.
      with self.browser_platform.ports.nested():
        yield self
    finally:
      self._is_running = False
