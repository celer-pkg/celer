# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
import enum
from typing import TYPE_CHECKING, Any, ClassVar, Optional, Self

from typing_extensions import override

from crossbench import exception
from crossbench.cli.config.network_speed import (NetworkSpeedConfig,
                                                 NetworkSpeedPreset)
from crossbench.config import ConfigEnum, ConfigObject, ConfigParser
from crossbench.network.live import LiveNetwork
from crossbench.network.local_file_server import LocalFileNetwork
from crossbench.network.replay.wpr import (LocalWprReplayNetwork,
                                           RemoteWprReplayNetwork)
from crossbench.network.traffic_shaping import ts_proxy
from crossbench.network.traffic_shaping.live import NoTrafficShaper
from crossbench.parse import PathParser

if TYPE_CHECKING:
  import urllib.parse as urlparse

  from crossbench import path as pth
  from crossbench.network.base import Network
  from crossbench.network.traffic_shaping.base import TrafficShaper
  from crossbench.plt.base import Platform

# We're using 'type' here a lot, let's skip the warnings from pylint.
# pylint: disable=redefined-builtin

@enum.unique
class NetworkType(ConfigEnum):
  LIVE = ("live", "Live network.")
  WPR = ("wpr", "Replayed network from a wpr.go archive.")
  LOCAL = ("local", "Serve content from a local http file server.")


@dataclasses.dataclass(frozen=True)
class NetworkConfig(ConfigObject):
  ARCHIVE_EXTENSIONS: ClassVar[tuple[str, ...]] = (".archive", ".wprgo")
  VALID_EXTENSIONS: ClassVar[tuple[str, ...]] = (
      ConfigObject.VALID_EXTENSIONS + ARCHIVE_EXTENSIONS)
  VALID_SCHEMES: ClassVar[tuple[str, ...]] = ("gs",)

  type: NetworkType = NetworkType.LIVE
  speed: NetworkSpeedConfig = NetworkSpeedConfig.default()
  path: pth.LocalPath | None = None
  url: str | None = None
  wpr_go_bin: pth.LocalPath | None = None
  persist_server: bool = False
  run_on_device: bool = False
  skip_injection: bool = False

  @classmethod
  def default(cls, type: Optional[NetworkType] = None) -> Self:
    return cls(type=type or NetworkType.LIVE)

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = ConfigParser(cls, default=cls.default())
    parser.add_argument("type", type=NetworkType, default=NetworkType.LIVE)
    preset_choices = tuple(str(preset) for preset in NetworkSpeedPreset) # pytype: disable=missing-parameter
    parser.add_argument(
        "speed",
        type=NetworkSpeedConfig,
        default=NetworkSpeedConfig.default(),
        help=("Enable traffic shaping using ts_proxy, disabled by default. "
              f"Either full NetworkSpeedConfig or one of {preset_choices}."))
    parser.add_argument(
        "path",
        type=PathParser.existing_path,
        help=("Path to a local directory for 'local' file server network, "
              "or path to a archive.wprgo for a 'wpr' replay network"))
    parser.add_argument("url", type=str)
    parser.add_argument(
        "wpr_go_bin",
        type=PathParser.existing_file_path,
        help=("Location of the wpr.go binary or source, "
              "used for WPR replay network. "
              "If not specified, a default lookup in known locations is used."))
    parser.add_argument("persist_server", type=bool, default=False)
    parser.add_argument(
        "run_on_device",
        type=bool,
        default=False,
        help=("For 'wpr' network only: switch to enable running on-device "
              "to reduce delays caused by traffic forwarding over adb."))
    parser.add_argument(
        "skip_injection",
        type=bool,
        default=False,
        help=("Don't inject the deterministic.js script into every response "
              "in WPR replay mode. Makes WPR response timings more stable."))
    return parser

  @classmethod
  def help(cls) -> str:
    return cls.config_parser().help

  @classmethod
  def parse_wpr(cls, value: Any) -> Self:
    config = cls.parse(value)
    if config.type != NetworkType.WPR:
      raise argparse.ArgumentTypeError(f"Expected wpr, but got {config.type}")
    return config

  @classmethod
  def parse_local(cls, value: Any) -> Self:
    config = cls.parse(value, type=NetworkType.LOCAL)
    if config.type != NetworkType.LOCAL:
      raise argparse.ArgumentTypeError(
          f"Expected local file server, but got {config.type}. ")
    return config

  @classmethod
  @override
  def parse_str(  # pylint: disable=arguments-differ
      cls,
      value: str,
      type: Optional[NetworkType] = None) -> Self:
    if not value:
      raise argparse.ArgumentTypeError("Network: Cannot parse empty string")
    if value == "default":
      return cls.default(type)
    if type and type is not NetworkType.LIVE:
      raise argparse.ArgumentTypeError(
          f"Network type mismatch expected LIVE, got {type}")
    return cls.parse_live(value)

  @classmethod
  def parse_live(cls, value: Any) -> Self:
    with exception.annotate_argparsing("Live network with speed config"):
      speed = NetworkSpeedConfig.parse(value)
      return cls(NetworkType.LIVE, speed)
    raise exception.UnreachableError()

  @classmethod
  def parse_url(cls,
                url: urlparse.ParseResult,
                type: Optional[NetworkType] = None,
                **kwargs) -> Self:
    cls.expect_no_extra_kwargs(kwargs)
    if type and type is not NetworkType.WPR:
      raise argparse.ArgumentTypeError(
          f"Network type mismatch, expected WPR, got {type}")
    assert url.scheme == "gs"
    return cls.parse_wpr_archive_url(url.geturl())

  @classmethod
  @override
  def maybe_valid_path(cls, path: pth.LocalPath) -> pth.LocalPath | None:
    if valid_path := super().maybe_valid_path(path):
      return valid_path
    # for local file server
    if path.is_dir():
      return path
    return None

  @classmethod
  def parse_path(cls, path: pth.LocalPath, **kwargs) -> Self:
    if path.suffix in cls.ARCHIVE_EXTENSIONS:
      return cls.parse_wpr_archive_path(path)
    if path.is_dir():
      return cls(NetworkType.LOCAL, path=path)
    return super().parse_path(path, **kwargs)

  @classmethod
  def parse_path_like(cls, original_value: str, path: pth.LocalPath,
                      **kwargs) -> Self:
    del original_value
    return cls.parse_any_path(path, **kwargs)

  @classmethod
  def parse_wpr_archive_path(cls, path: pth.LocalPath) -> Self:
    path = PathParser.non_empty_file_path(path, "wpr.go archive")
    return cls(type=NetworkType.WPR, path=path)

  @classmethod
  def parse_wpr_archive_url(cls, url: str) -> Self:
    return cls(type=NetworkType.WPR, url=url)

  @override
  def validate(self) -> None:
    if not self.type:
      raise argparse.ArgumentTypeError("Missing NetworkConfig.type.")
    if not self.speed and isinstance(self.speed, NetworkSpeedConfig):
      raise argparse.ArgumentTypeError("Missing NetworkConfig.speed.")
    if self.type == NetworkType.LIVE:
      if self.path:
        raise argparse.ArgumentTypeError(
            "NetworkConfig path cannot be used with type=live")
    elif self.type is NetworkType.WPR:
      if not self.path and not self.url:
        raise argparse.ArgumentTypeError(
            "NetworkConfig with type=replay requires "
            "a valid wpr.go archive path or download url.")
      if self.path and self.url:
        raise argparse.ArgumentTypeError(
            "NetworkConfig with type=replay requires "
            "either archive path or download url but not both.")
    elif self.type is NetworkType.LOCAL:
      if not self.path:
        raise argparse.ArgumentTypeError(
            "NetworkConfig with type=local requires "
            "a valid local dir path to serve files.")
      PathParser.non_empty_dir_path(self.path, "local-serve dir")
    if self.wpr_go_bin and self.type is not NetworkType.WPR:
      raise argparse.ArgumentTypeError(
          "wpr_go_bin can only be used for the WPR replay network")
    if self.persist_server and self.type is not NetworkType.WPR:
      # TODO: support file server as well
      raise argparse.ArgumentTypeError(
          "persist_server can only be used for the WPR replay network")
    if self.run_on_device and self.type is not NetworkType.WPR:
      raise argparse.ArgumentTypeError(
          "run_on_device can only be used for the WPR replay network")
    if self.skip_injection and self.type is not NetworkType.WPR:
      raise argparse.ArgumentTypeError(
          "skip_injection can only be used for the WPR replay network")

  def create(self, browser_platform: Platform) -> Network:
    with exception.annotate_argparsing(
        f"Setting up {self.type} network for {browser_platform}"):
      traffic_shaper = self._create_traffic_shaper(browser_platform)
      if self.type is NetworkType.LIVE:
        return LiveNetwork(traffic_shaper, browser_platform)
      if self.type is NetworkType.LOCAL:
        assert self.path
        return LocalFileNetwork(self.path, self.url, traffic_shaper,
                                browser_platform)
      if self.type is NetworkType.WPR:
        if self.run_on_device and browser_platform.is_remote:
          if not RemoteWprReplayNetwork.is_compatible(browser_platform):
            raise ValueError(
                f"run_on_device is unsupported on {browser_platform}")
          return RemoteWprReplayNetwork(
              self.url or str(self.path),
              traffic_shaper,
              self.wpr_go_bin,
              browser_platform,
              self.persist_server,
              inject_deterministic_script=not self.skip_injection)
        return LocalWprReplayNetwork(
            self.url or str(self.path),
            traffic_shaper,
            self.wpr_go_bin,
            browser_platform,
            self.persist_server,
            inject_deterministic_script=not self.skip_injection)
    raise ValueError(f"Unknown network type {self.type}")

  def _create_traffic_shaper(self, browser_platform: Platform) -> TrafficShaper:
    if self.speed.is_live:
      return NoTrafficShaper(browser_platform)
    return ts_proxy.TsProxyTrafficShaper(browser_platform, self.speed.ts_proxy,
                                         self.speed.rtt_ms, self.speed.in_kbps,
                                         self.speed.out_kbps, self.speed.window)
