# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import re
from typing import Final, Optional

from typing_extensions import override

from crossbench.browsers.version import (BrowserVersion, BrowserVersionChannel,
                                         PartialBrowserVersionError)


class ChromiumVersion(BrowserVersion):
  _PARTS_LEN: Final[int] = 4
  _VERSION_RE = re.compile(
      r"(?P<prefix>[^\d]*)"
      r"(?P<version>(?:\d{2,3}(?:\.(?:\d{1,4}|X)){0,3})|latest)? ?"
      r"(?P<suffix>.*)", re.I)
  _VALID_SUFFIX_MATCH = re.compile(r"[^.\d]+", re.I)
  _CHANNEL_LOOKUP: dict[str, BrowserVersionChannel] = {
      "any": BrowserVersionChannel.ANY,
      "extended": BrowserVersionChannel.LTS,
      "stable": BrowserVersionChannel.STABLE,
      "beta": BrowserVersionChannel.BETA,
      "dev": BrowserVersionChannel.ALPHA,
      "canary": BrowserVersionChannel.PRE_ALPHA,
  }
  _CHANNEL_NAME_LOOKUP: dict[BrowserVersionChannel, str] = {
      channel: name for name, channel in _CHANNEL_LOOKUP.items()
  }
  _CHANNEL_RE = re.compile("|".join(_CHANNEL_LOOKUP.keys()), re.I)

  @classmethod
  @override
  def _parse(
      cls,
      full_version: str) -> tuple[tuple[int, ...], BrowserVersionChannel, str]:
    matches = cls._VERSION_RE.fullmatch(full_version.strip(),)
    if not matches:
      raise cls.parse_error("Could not extract version number.", full_version)
    channel_str = cls._parse_channel(full_version)
    version_str = matches["version"]
    if not version_str and not channel_str:
      raise cls.parse_error("Got empty version match.", full_version)
    prefix = matches["prefix"]
    if not cls._validate_prefix(prefix):
      raise cls.parse_error(f"Wrong prefix {repr(prefix)}", full_version)
    suffix = matches["suffix"]
    if not cls._validate_suffix(suffix):
      raise cls.parse_error(f"Wrong suffix {repr(suffix)}", full_version)

    if not version_str:
      return cls._channel_version(channel_str, full_version)
    return cls._numbered_version(version_str, full_version)

  @classmethod
  def _parse_channel(cls, full_version: str) -> str:
    if matches := cls._CHANNEL_RE.search(full_version):
      return matches[0]
    return ""

  @classmethod
  def _channel_version(
      cls, channel_str: str,
      full_version: str) -> tuple[tuple[int, ...], BrowserVersionChannel, str]:
    channel = cls._parse_exact_channel(channel_str, full_version)
    version_str = ""
    return tuple(), channel, version_str

  @classmethod
  def _numbered_version(
      cls, version_str: str,
      full_version: str) -> tuple[tuple[int, ...], BrowserVersionChannel, str]:
    channel: BrowserVersionChannel = cls._parse_default_channel(full_version)

    parts_str = version_str.split(".")
    if len(parts_str) > cls._PARTS_LEN:
      raise cls.parse_error(f"Too many version parts {parts_str}", full_version)
    if len(parts_str) != 1 and len(parts_str) != cls._PARTS_LEN:
      raise cls.parse_error(
          f"Incomplete chrome version number, need {cls._PARTS_LEN} parts",
          full_version)
    # Remove .X from the input version.
    while parts_str[-1] == "X":
      parts_str.pop()
    try:
      parts = tuple(map(int, parts_str))
    except ValueError as e:
      raise cls.parse_error(
          f"Could not parse version parts {repr(version_str)}",
          full_version) from e
    if not parts_str:
      raise cls.parse_error("Need at least one version number part.",
                            full_version)
    if len(parts_str) == 1:
      version_str = f"M{parts_str[0]}"
    else:
      padding = ("X",) * (cls._PARTS_LEN - len(parts))
      version_str = ".".join(map(str, parts + padding))
    return parts, channel, version_str

  @classmethod
  def _validate_prefix(cls, prefix: Optional[str]) -> bool:
    if not prefix:
      return True
    prefix = prefix.lower()
    if prefix.strip() == "m":
      return True
    return "chromium " in prefix or "chromium-" in prefix

  @classmethod
  def _parse_exact_channel(cls, channel_str: str,
                           full_version: str) -> BrowserVersionChannel:
    if channel := cls._CHANNEL_LOOKUP.get(channel_str.lower()):
      return channel
    raise cls.parse_error(f"Unknown channel {repr(channel_str)}", full_version)

  @classmethod
  def _parse_default_channel(cls, full_version: str) -> BrowserVersionChannel:
    version_lower: str = full_version.lower()
    for channel_name, channel_obj in cls._CHANNEL_LOOKUP.items():
      if channel_name in version_lower:
        return channel_obj
    return BrowserVersionChannel.STABLE

  @classmethod
  def _validate_suffix(cls, suffix: Optional[str]) -> bool:
    if not suffix:
      return True
    return bool(cls._VALID_SUFFIX_MATCH.fullmatch(suffix))

  @property
  @override
  def key(self) -> tuple[tuple[int, ...], BrowserVersionChannel]:
    return (self.comparable_parts(self._PARTS_LEN), self._channel)

  @property
  @override
  def has_complete_parts(self) -> bool:
    return len(self.parts) == 4

  @property
  def build(self) -> int:
    if len(self._parts) <= 2:
      raise PartialBrowserVersionError(self)
    return self._parts[2]

  @property
  def patch(self) -> int:
    if len(self._parts) <= 3:
      raise PartialBrowserVersionError(self)
    return self._parts[3]

  @property
  def is_dev(self) -> bool:
    return self.is_alpha

  @property
  def is_canary(self) -> bool:
    return self.is_pre_alpha

  @override
  def _channel_name(self, channel: BrowserVersionChannel) -> str:
    if name := self._CHANNEL_NAME_LOOKUP[channel]:
      return name
    raise ValueError(f"Unsupported channel: {channel}")


class ChromeDriverVersion(ChromiumVersion):
  _EMPTY_COMMIT_HASH: Final = "0000000000000000000000000000000000000000"

  @classmethod
  @override
  def _validate_prefix(cls, prefix: Optional[str]) -> bool:
    if not prefix:
      return False
    return prefix.lower() in ("chromedriver ", "chromedriver-",
                              "microsoft edge webdriver ")

  @classmethod
  @override
  def _parse_default_channel(cls, full_version: str) -> BrowserVersionChannel:
    if cls._EMPTY_COMMIT_HASH in full_version:
      return BrowserVersionChannel.PRE_ALPHA
    return BrowserVersionChannel.STABLE

  @classmethod
  @override
  def _validate_suffix(cls, suffix: Optional[str]) -> bool:
    # TODO: extract commit hash / branch info from newer versions
    return True
