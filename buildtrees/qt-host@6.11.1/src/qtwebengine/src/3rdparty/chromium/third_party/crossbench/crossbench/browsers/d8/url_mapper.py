# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import TYPE_CHECKING, Final

import crossbench.path as pth
from crossbench.network.local_file_server import LocalFileNetwork

if TYPE_CHECKING:
  from crossbench.browsers.d8.d8 import D8
  from crossbench.network.base import Network
  from crossbench.runner.groups.session import BrowserSessionRunGroup

MOCK_DIR: Final[pth.LocalPath] = pth.LocalPath(__file__).parent / "mock"


class D8URLMapper:

  @classmethod
  def create(cls, d8: D8, session: BrowserSessionRunGroup) -> D8URLMapper:
    benchmark_name: str = session.benchmark.NAME
    if "jetstream" in benchmark_name.lower():
      return JetStreamURLMapper(d8)
    raise ValueError(f"D8: Unsupported benchmark: {benchmark_name}")

  def __init__(self, d8: D8):
    self._d8 = d8
    network: Network = d8.network
    assert isinstance(
        network, LocalFileNetwork), (f"Expected LocalFileNetwork got {network}")
    self._network: LocalFileNetwork = network

  @property
  def path(self) -> pth.LocalPath:
    return self._network.path

  @property
  def setup_file(self) -> pth.LocalPath | None:
    return None

  @abc.abstractmethod
  def lookup(self, url: str) -> pth.LocalPath | None:
    pass


class DummyURLMapper(D8URLMapper):
  def lookup(self, url: str) -> pth.LocalPath | None:
    return None


class JetStreamURLMapper(D8URLMapper):

  def __init__(self, d8: D8):
    super().__init__(d8)
    self._driver_js: pth.LocalPath = self.path / "JetStreamDriver.js"
    if not self._driver_js.is_file():
      self._driver_js  = self.path / "driver.js"
    if not self._driver_js.is_file():
      raise ValueError(f"{self._driver_js} does not exist.")

  @property
  def setup_file(self) -> pth.LocalPath:
    return MOCK_DIR / "jetstream.js"

  def lookup(self, url: str) -> pth.LocalPath:
    return self._driver_js
