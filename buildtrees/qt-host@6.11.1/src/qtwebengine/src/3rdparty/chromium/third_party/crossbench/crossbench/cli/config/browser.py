# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
import logging
import os
import re
from typing import Any, Optional, Self, cast

from typing_extensions import override

import crossbench.browsers.all as all_browsers
from crossbench import path as pth
from crossbench import plt
from crossbench.browsers.chrome.downloader import ChromeDownloader
from crossbench.browsers.firefox.downloader import FirefoxDownloader
from crossbench.cli.config.driver import DriverConfig
from crossbench.cli.config.driver_type import BrowserDriverType
from crossbench.cli.config.env import ENV_CONFIG_PRESETS, EnvConfig
from crossbench.cli.config.extension import ExtensionConfig
from crossbench.cli.config.network import NetworkConfig
from crossbench.cli.config.network_speed import NetworkSpeedPreset
from crossbench.config import ConfigObject, ConfigParser
from crossbench.parse import NumberParser, ObjectParser, PathParser

SUPPORTED_EMBEDDER = ("googlequicksearchbox",)
SUPPORTED_BROWSER = ("chromium", "chrome", "safari", "edge", "firefox",
                     "d8") + SUPPORTED_EMBEDDER

# Split inputs like:
# - "/out/x64.release/chrome"
# - "/out/x64.release/chrome:4G"
# - "C:\out\x64.release\chrome"
# - "C:\out\x64.release\chrome:4G"
# - "applescript:/out/x64.release/chrome"
# - "applescript:/out/x64.release/chrome:4G"
# - "selenium:C:\out\x64.release\chrome"
# - "selenium:C:\out\x64.release\chrome:4G"
NETWORK_PRESETS: str = "|".join(
    re.escape(preset.value) for preset in NetworkSpeedPreset)  # pytype: disable=missing-parameter
ENV_PRESETS: str = "|".join(re.escape(preset) for preset in ENV_CONFIG_PRESETS)

SHORT_FORM_RE: re.Pattern[str] = re.compile(
    r"((?P<driver>\w{3,}):)??"
    r"(?P<path>([A-Z]:[/\\])?[^:]+)"
    f"(:(?P<network>{NETWORK_PRESETS}))?"
    f"(:(?P<env>{ENV_PRESETS}))?")
ANDROID_PACKAGE_RE: re.Pattern[str] = re.compile(r"[a-z]+(\.[a-z]+){2,}")
VERSION_FOR_RANGE_RE: re.Pattern[str] = re.compile(
    r"(?P<prefix>[^\d]*)(?P<milestone>\d+)")


@dataclasses.dataclass(frozen=True)
class BrowserConfig(ConfigObject):
  browser: pth.AnyPathLike
  driver: DriverConfig = DriverConfig.default()
  # Make network optional since --network provides a global default and we do
  # want to have the option to explicitly specify the default network in a
  # browser config.
  network: NetworkConfig | None = None
  env: EnvConfig | None = None

  cache_dir: pth.AnyPath | None = None
  clear_cache: bool | None = None
  extensions: tuple[ExtensionConfig, ...] = tuple()

  def __post_init__(self) -> None:
    if not self.browser:
      raise ValueError(f"{type(self).__name__}.browser cannot be None.")
    if not self.driver:
      raise ValueError(f"{type(self).__name__}.driver cannot be None.")

  @classmethod
  def default(cls) -> Self:
    return cls(
        all_browsers.Chrome.stable_path(plt.PLATFORM), DriverConfig.default())

  @classmethod
  @override
  def parse_any_path(cls, path: pth.LocalPath, **kwargs) -> Self:
    if cls.is_supported_browser_path(path):
      return cls(path)
    return super().parse_any_path(path, **kwargs)

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    if not value:
      raise argparse.ArgumentTypeError("Cannot parse empty string")
    path: pth.AnyPathLike | None = None
    driver = DriverConfig.default()
    network: NetworkConfig | None = None
    env: EnvConfig | None = None
    if ":" not in value or cls.is_path_like(value):
      # Variant: $PATH_OR_IDENTIFIER
      path = cls._parse_path_or_identifier(value)
    else:
      # Variant: ${DRIVER_TYPE}:${PATH_OR_IDENTIFIER}:${NETWORK}
      driver, path, network, env = cls._parse_inline_short_form(value)
    assert path, "Invalid path"
    return cls(path, driver, network, env)

  @classmethod
  def parse_with_range(cls, value: Any) -> tuple[Self, ...]:
    if isinstance(value, str):
      return cls._parse_with_range(value)
    return (cls.parse(value),)

  @classmethod
  def _parse_with_range(cls, value: str) -> tuple[Self, ...]:
    if not value:
      raise argparse.ArgumentTypeError("Cannot parse empty string")
    parts = value.split("...", maxsplit=1)
    start_version: str = parts.pop(0)
    if not parts:
      return (cls.parse(start_version),)
    limit_version = parts[0]

    start_match = VERSION_FOR_RANGE_RE.fullmatch(start_version)
    if not start_match:
      raise argparse.ArgumentTypeError(
          f"Start of a browser range {repr(value)} must end in digits, "
          f"but got {repr(start_version)}")
    limit_match = VERSION_FOR_RANGE_RE.fullmatch(limit_version)
    if not limit_match:
      raise argparse.ArgumentTypeError(
          f"Upper limit of a browser range {repr(value)} must end in digits, "
          f"but got {repr(limit_version)}")

    start_prefix = start_match["prefix"]
    limit_prefix = limit_match["prefix"]
    if limit_prefix and not start_prefix.endswith(limit_prefix):
      raise argparse.ArgumentTypeError(
          f"Browser version range start prefix {repr(start_prefix)} must match "
          f"limit prefix {repr(limit_prefix)}: {repr(value)}")

    start_milestone: int = NumberParser.positive_int(
        start_match["milestone"], "browser version range start milestone")
    limit_milestone: int = NumberParser.positive_int(
        limit_match["milestone"], "browser version range limit milestone")
    if start_milestone > limit_milestone:
      raise argparse.ArgumentTypeError(
          f"Browser version limit must be larger than start: {repr(value)}")

    count = limit_milestone - start_milestone
    logging.info("Creating %d intermediate browser versions from %s", count,
                 value)
    versions = []
    for milestone in range(start_milestone, limit_milestone + 1):
      version_str = f"{start_prefix}{milestone}"
      versions.append(cls.parse(version_str))
    return tuple(versions)

  @classmethod
  def _parse_path_or_identifier(
      cls,
      maybe_path_or_identifier: str,
      driver_type: Optional[BrowserDriverType] = None,
      driver: Optional[DriverConfig] = None) -> pth.AnyPathLike:
    if not maybe_path_or_identifier:
      raise argparse.ArgumentTypeError("Got empty browser identifier.")
    if not driver_type:
      if driver:
        driver_type = driver.type
      else:
        driver_type = BrowserDriverType.default()
    identifier = maybe_path_or_identifier.lower()
    path = None
    if cls.is_path_like(maybe_path_or_identifier):
      if cls._is_downloadable_identifier(maybe_path_or_identifier):
        return maybe_path_or_identifier
      # Assume a path since short-names never contain back-/slashes.
      if driver_type.is_remote_browser:
        path = PathParser.path(maybe_path_or_identifier)
      else:
        path = cls.resolve_path(
            PathParser.existing_path(maybe_path_or_identifier))
    else:
      if ":" in maybe_path_or_identifier:
        raise argparse.ArgumentTypeError(
            "Got unexpected short-form string "
            f"{repr(maybe_path_or_identifier)}. \n"
            "  - Use a complex browser config with separate "
            "'browser' and 'driver' attributes, or\n"
            "  - Use the short-form directly on the parent config attribute: \n"
            f"   {{my-browser: '{maybe_path_or_identifier}'}}")
      if maybe_path := cls._try_parse_short_name(identifier, driver_type):
        return maybe_path
      if cls._is_downloadable_identifier(maybe_path_or_identifier):
        return maybe_path_or_identifier
      if driver_type == BrowserDriverType.ANDROID:
        if ANDROID_PACKAGE_RE.fullmatch(maybe_path_or_identifier):
          return pth.AnyPosixPath(maybe_path_or_identifier)
    if not path:
      path = pth.try_resolve_existing_path(maybe_path_or_identifier)
      if not path:
        raise argparse.ArgumentTypeError(
            f"Unknown browser path or short name: '{maybe_path_or_identifier}'")
    if cls.is_supported_browser_path(path):
      return path
    raise argparse.ArgumentTypeError(f"Unsupported browser path='{path}'")

  @classmethod
  def _is_downloadable_identifier(cls, maybe_path_or_identifier: str) -> bool:
    # TODO: handle remote platforms.
    platform = plt.PLATFORM
    if ChromeDownloader.is_valid(maybe_path_or_identifier, platform):
      return True
    if FirefoxDownloader.is_valid(maybe_path_or_identifier, platform):
      return True
    return False

  @classmethod
  def _try_parse_short_name(
      cls, identifier: str,
      driver_type: BrowserDriverType) -> Optional[pth.AnyPath]:
    # We're not using a dict-based lookup here, since not all browsers are
    # available on all platforms
    # TODO: handle remote platforms.
    platform = plt.PLATFORM
    if identifier in ("chrome", "chrome-stable", "chr-stable", "chr"):
      if driver_type == BrowserDriverType.ANDROID:
        return pth.AnyPosixPath("com.android.chrome")
      return all_browsers.Chrome.stable_path(platform)
    if identifier in ("chrome-app"):
      if driver_type == BrowserDriverType.ANDROID:
        return pth.AnyPosixPath("com.google.android.apps.chrome")
    if identifier in ("chrome-beta", "chr-beta"):
      if driver_type == BrowserDriverType.ANDROID:
        return pth.AnyPosixPath("com.chrome.beta")
      return all_browsers.Chrome.beta_path(platform)
    if identifier in ("chrome-dev", "chr-dev"):
      if driver_type == BrowserDriverType.ANDROID:
        return pth.AnyPosixPath("com.chrome.dev")
      return all_browsers.Chrome.dev_path(platform)
    if identifier in ("chrome-canary", "chr-canary"):
      if driver_type == BrowserDriverType.ANDROID:
        return pth.AnyPosixPath("com.chrome.canary")
      return all_browsers.Chrome.canary_path(platform)
    if identifier == "chromium":
      if driver_type == BrowserDriverType.ANDROID:
        return pth.AnyPosixPath("org.chromium.chrome")
      return all_browsers.Chromium.default_path(platform)
    if identifier in ("edge", "edge-stable"):
      return all_browsers.Edge.stable_path(platform)
    if identifier == "edge-beta":
      return all_browsers.Edge.beta_path(platform)
    if identifier == "edge-dev":
      return all_browsers.Edge.dev_path(platform)
    if identifier == "edge-canary":
      return all_browsers.Edge.canary_path(platform)
    if identifier in ("safari", "sf", "safari-stable", "sf-stable"):
      return all_browsers.Safari.default_path(platform)
    if identifier in ("safari-technology-preview", "safari-tech-preview",
                      "safari-tp", "sf-tp", "stp", "tp"):
      return all_browsers.Safari.technology_preview_path(platform)
    if identifier in ("firefox", "firefox-stable", "ff", "ff-stable"):
      return all_browsers.Firefox.default_path(platform)
    if identifier in ("firefox-dev", "firefox-developer-edition", "ff-dev"):
      return all_browsers.Firefox.developer_edition_path(platform)
    if identifier in ("firefox-nightly", "ff-nightly", "ff-trunk"):
      return all_browsers.Firefox.nightly_path(platform)
    if identifier in ("webview", "org.chromium.webview_shell"):
      return pth.AnyPosixPath("org.chromium.webview_shell")
    return None

  @classmethod
  def is_supported_browser_path(cls, path: pth.AnyPath) -> bool:
    path_str = os.fspath(path).lower()
    return any(short_name in path_str for short_name in SUPPORTED_BROWSER)

  @classmethod
  def _parse_inline_short_form(
      cls, value: str
  ) -> tuple[DriverConfig, pth.AnyPathLike, Optional[NetworkConfig],
             Optional[EnvConfig]]:
    assert ":" in value, f"Invalid short config {repr(value)} for {cls}"
    match = SHORT_FORM_RE.fullmatch(value)
    if not match:
      raise argparse.ArgumentTypeError(
          f"Invalid browser short form: '{value}' \n"
          "A browser path/identifier and "
          "at least a driver or network preset have to be present")
    path_or_identifier = match.group("path")
    if not path_or_identifier:
      raise argparse.ArgumentTypeError(
          "Browser short form: missing path or browser identifier.")
    driver = DriverConfig.default()
    if driver_identifier := match.group("driver"):
      driver = cast(DriverConfig, DriverConfig.parse(driver_identifier))
    path: pth.AnyPathLike = cls._parse_path_or_identifier(
        path_or_identifier, driver.type)
    network = None
    if network_identifier := match.group("network"):
      network = NetworkConfig.parse_str(network_identifier)
    env = None
    if env_identifier := match.group("env"):
      env = EnvConfig.parse_str(env_identifier)
    return (driver, path, network, env)

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = ConfigParser(cls)
    parser.add_argument(
        "browser",
        aliases=("path",),
        type=cls._parse_path_or_identifier,
        required=True,
        depends_on=("driver",))
    parser.add_argument(
        "driver", type=DriverConfig, default=DriverConfig.default())
    parser.add_argument("network", type=NetworkConfig)
    parser.add_argument(
        "cache_dir",
        aliases=("browser_cache", "browser_cache_dir"),
        type=PathParser.optional_any_path,
        default=None)
    parser.add_argument(
        "clear_cache",
        aliases=("clear_cache_dir", "clear_browser_cache",
                 "clear_browser_cache_dir"),
        type=ObjectParser.optional_bool,
        default=None)
    parser.add_argument(
        "extensions", type=ExtensionConfig, is_list=True, default=())
    return parser

  @property
  def is_remote(self) -> bool:
    return self.driver.type.is_remote_browser

  @property
  def is_local(self) -> bool:
    return self.driver.type.is_local_browser

  @property
  def path(self) -> pth.AnyPath:
    assert isinstance(self.browser, pth.AnyPath)
    return self.browser

  def get_platform(self) -> plt.Platform:
    return self.driver.get_platform()
