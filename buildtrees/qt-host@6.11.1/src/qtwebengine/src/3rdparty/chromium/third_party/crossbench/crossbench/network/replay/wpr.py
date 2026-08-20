# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import contextlib
import dataclasses
import logging
from typing import TYPE_CHECKING, Final, Iterator, Mapping, Optional, TypeVar

from typing_extensions import override

from crossbench.helper.path_finder import WprGoToolFinder
from crossbench.network.replay.base import GS_PREFIX, ReplayNetwork
from crossbench.network.replay.web_page_replay import WprReplayServer
from crossbench.path import check_hash
from crossbench.plt import PLATFORM, Platform

if TYPE_CHECKING:
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.flags.base import Flags
  from crossbench.network.base import TrafficShaper
  from crossbench.path import AnyPath, LocalPath
  from crossbench.runner.groups.session import BrowserSessionRunGroup


# use value for pylint
assert GS_PREFIX

WPR_BASE_URL = "gs://chromium-telemetry/binary_dependencies"


@dataclasses.dataclass
class WPRCloudBinary:
  file_hash: str

  @property
  def url(self) -> str:
    return f"{WPR_BASE_URL}/wpr_go_{self.file_hash}"


# See third_party/catapult/telemetry/telemetry/binary_dependencies.json
WPR_PREBUILT_LOOKUP: Final[Mapping[tuple[str, str], WPRCloudBinary]] = {
    ("android", "arm64"):
        WPRCloudBinary("8f422f75ae74113ccc12234bf2d1368074754fcb"),
    ("android", "arm32"):
        WPRCloudBinary("f0aa37ad758ec972816ee65f446b99bdbd74746b"),
    ("android", "x64"):
        WPRCloudBinary("864c50726a8cb5637339ccf2a074ec4b5f413753"),
    # On arm64 ChromeOS, use the same binary as arm64 Linux.
    ("chromeos_ssh", "arm64"):
        WPRCloudBinary("8f422f75ae74113ccc12234bf2d1368074754fcb"),
    # On x64 ChromeOS, use the same binary as x64 Linux.
    ("chromeos_ssh", "x64"):
        WPRCloudBinary("864c50726a8cb5637339ccf2a074ec4b5f413753"),
    ("linux", "x64"):
        WPRCloudBinary("864c50726a8cb5637339ccf2a074ec4b5f413753"),
    ("macos", "arm64"):
        WPRCloudBinary("a245938846180631dbc9806e90147e3cfbc927fc"),
    ("macos", "x64"):
        WPRCloudBinary("613419bc52b357419e7bd7a1158fe257a1b73e97"),
    ("win", "x64"):
        WPRCloudBinary("6f67a1c2284bfe2c36824ceecb5b0f456cdd191c"),
}


WprReplayNetworkT = TypeVar("WprReplayNetworkT", bound="WprReplayNetwork")

class WprReplayNetwork(ReplayNetwork):

  def __init__(self,
               archive: LocalPath | str,
               traffic_shaper: Optional[TrafficShaper] = None,
               wpr_go_bin: Optional[LocalPath] = None,
               browser_platform: Platform = PLATFORM,
               persist_server: bool = False,
               inject_deterministic_script: bool = True) -> None:
    super().__init__(archive, traffic_shaper, browser_platform)
    self._server: WprReplayServer | None = None
    self._tmp_dir: AnyPath | None = None
    self._persist_server = persist_server
    self._inject_deterministic_script = inject_deterministic_script
    self._ensure_wpr_go(wpr_go_bin)

  @override
  def extra_flags(self, browser_attributes: BrowserAttributes) -> Flags:
    assert self.is_running, "Extra network flags are not valid"
    assert self._server
    if not browser_attributes.is_chromium_based:
      raise ValueError(
          "Only chromium-based browsers are supported for wpr replay.")
    # TODO: make ports configurable.
    extra_flags = super().extra_flags(browser_attributes)
    # TODO: read this from wpr_public_hash.txt like in the recorder probe
    extra_flags["--ignore-certificate-errors-spki-list"] = (
        "PhrPvGIaAMmd29hj8BCZOq096yj7uMpRNHpn5PDxI6I=,"
        "2HcXCSKKJS0lEXLQEWhpHUfGuojiU0tiT5gOF9LP6IQ=")
    if self._traffic_shaper.is_live:
      # Only remap ports if we're not using the SOCKS proxy from the traffic
      # shaper.
      extra_flags["--host-resolver-rules"] = (
          f"MAP *:80 {self.host}:{self.http_port},"
          f"MAP *:443 {self.host}:{self.https_port},"
          "EXCLUDE localhost")

    return extra_flags

  @abc.abstractmethod
  def _ensure_wpr_go(self, wpr_go_bin: Optional[LocalPath] = None):
    pass

  @abc.abstractmethod
  def _create_server(self, log_dir: LocalPath) -> WprReplayServer:
    pass

  @contextlib.contextmanager
  @override
  def open(self: WprReplayNetworkT,
           session: BrowserSessionRunGroup) -> Iterator[WprReplayNetworkT]:
    with super().open(session):
      yield self

  def _ensure_server_started(self, session: BrowserSessionRunGroup) -> None:
    log_dir = session.browser_dir if self._persist_server else session.out_dir
    if not self._server or not self._persist_server:
      self._server = self._create_server(log_dir)
      logging.debug("Starting WPR server")
      self._server.start()
    else:
      # TODO: reset wpr server state for reuse
      logging.debug("WPR server already started")

  @contextlib.contextmanager
  @override
  def _open_replay_server(self, session: BrowserSessionRunGroup):
    self._ensure_server_started(session)

    try:
      yield self
    finally:
      if not self._persist_server and self._server:
        self._server.stop()

  @property
  @override
  def http_port(self) -> int:
    assert self._server, "WPR is not running"
    return self._server.http_port

  @property
  @override
  def https_port(self) -> int:
    assert self._server, "WPR is not running"
    return self._server.https_port

  @property
  @override
  def host(self) -> str:
    assert self._server, "WPR is not running"
    return self._server.host

  @property
  def inject_deterministic_script(self) -> bool:
    return self._inject_deterministic_script

  def __str__(self) -> str:
    return f"WPR(archive={self.archive_path}, speed={self.traffic_shaper})"


class LocalWprReplayNetwork(WprReplayNetwork):

  @override
  def _ensure_wpr_go(self, wpr_go_bin: Optional[LocalPath] = None) -> None:
    if not wpr_go_bin:
      if local_wpr_go := WprGoToolFinder(self.host_platform).local_path:
        wpr_go_bin = local_wpr_go
    if not wpr_go_bin:
      raise RuntimeError(
          f"Could not find wpr.go binary on {self.host_platform}")
    if wpr_go_bin.suffix == ".go" and not self.host_platform.which("go"):
      raise ValueError(f"'go' binary not found on {self.host_platform}")
    self._wpr_go_bin: LocalPath = self.host_platform.parse_local_binary_path(
        wpr_go_bin, "wpr.go source")

  @contextlib.contextmanager
  @override
  def open(self: LocalWprReplayNetwork,
           session: BrowserSessionRunGroup) -> Iterator[LocalWprReplayNetwork]:
    with super().open(session):
      with self._forward_ports(session):
        yield self

  @contextlib.contextmanager
  def _forward_ports(self, session: BrowserSessionRunGroup) -> Iterator:
    browser_platform = session.browser_platform
    if not self._traffic_shaper.is_live or not browser_platform.is_remote:
      yield
      return
    http_port = self.http_port
    https_port = self.https_port
    logging.info("REMOTE PORT FORWARDING: %s <= %s", self.host_platform,
                 browser_platform)
    # TODO: make ports configurable
    with browser_platform.ports.nested() as ports:
      ports.reverse_forward(http_port, http_port)
      ports.reverse_forward(https_port, https_port)
      yield
      # port cleanup happens automatically

  @override
  def _create_server(self, log_dir: LocalPath) -> WprReplayServer:
    inject_scripts: list[AnyPath] | None = (
        None if self.inject_deterministic_script else [])
    return WprReplayServer(
        self.archive_path,
        self._wpr_go_bin,
        inject_scripts=inject_scripts,
        log_path=log_dir / "network.wpr.log",
        platform=self.host_platform)


class RemoteWprReplayNetwork(WprReplayNetwork):

  @classmethod
  def is_compatible(cls, platform: Platform) -> bool:
    return platform.is_android or platform.is_chromeos

  @override
  def _ensure_wpr_go(self, wpr_go_bin: Optional[LocalPath] = None) -> None:
    assert RemoteWprReplayNetwork.is_compatible(self.browser_platform)
    if wpr_go_bin:
      if wpr_go_bin.suffix == ".go":
        raise ValueError(f"Can't run .go files on {self.browser_platform}")
    else:
      wpr_go_bin = self._download_prebuilt_wpr()
    self._wpr_go_bin: LocalPath = self.host_platform.parse_local_binary_path(
        wpr_go_bin, "wpr.go binary")

  def _download_prebuilt_wpr(self) -> LocalPath:
    wpr_cloud_binary = WPR_PREBUILT_LOOKUP[self.browser_platform.key]
    local_wpr_go_bin = (
        self.host_platform.local_cache_dir("wpr") /
        str(self.browser_platform.machine) / "wpr_go")
    if not check_hash(local_wpr_go_bin, wpr_cloud_binary.file_hash):
      self.host_platform.download_gcs_file(wpr_cloud_binary.url,
                                           local_wpr_go_bin)
    assert check_hash(local_wpr_go_bin, wpr_cloud_binary.file_hash)

    return local_wpr_go_bin

  @contextlib.contextmanager
  @override
  def open(self: RemoteWprReplayNetwork,
           session: BrowserSessionRunGroup) -> Iterator[RemoteWprReplayNetwork]:
    with self._remote_temp_dir(session):
      with super().open(session):
        yield self

  @contextlib.contextmanager
  def _remote_temp_dir(self, session: BrowserSessionRunGroup) -> Iterator:
    with session.browser_platform.TemporaryDirectory() as tmp_dir:
      self._tmp_dir = tmp_dir
      yield
      self._tmp_dir = None

  def _push_file(self, path: LocalPath) -> AnyPath:
    assert self._tmp_dir is not None
    remote_path = self._tmp_dir / path.name
    self.browser_platform.push(path, remote_path)
    return remote_path

  def _push_required_files(self) -> list[AnyPath]:
    host_platform = self.host_platform
    if local_wpr_go := WprGoToolFinder(host_platform).local_path:
      wpr_root = local_wpr_go.parents[1]
    else:
      raise RuntimeError(f"Could not fine local wpr.go on {host_platform}")

    all_files: list[LocalPath] = [
        self._archive_path, wpr_root / "ecdsa_key.pem",
        wpr_root / "ecdsa_cert.pem", wpr_root / "deterministic.js"
    ]
    remote_files = [self._push_file(path) for path in all_files]

    remote_wpr_go_bin = self._push_file(self._wpr_go_bin)
    self.browser_platform.sh("chmod", "+x", remote_wpr_go_bin)

    return [remote_wpr_go_bin] + remote_files

  @override
  def _create_server(self, log_dir: LocalPath) -> WprReplayServer:
    wpr_go_bin, archive, key_file, cert_file, inject_script = (
        self._push_required_files())
    inject_scripts: list[AnyPath] = ([inject_script] if
                                     self.inject_deterministic_script else [])
    return WprReplayServer(
        archive_path=archive,
        bin_path=wpr_go_bin,
        key_file=key_file,
        cert_file=cert_file,
        inject_scripts=inject_scripts,
        log_path=log_dir / "network.wpr.log",
        platform=self.browser_platform)
