# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import re
from typing import TYPE_CHECKING, Final

from typing_extensions import override

from crossbench.browsers.version import BrowserVersion, BrowserVersionChannel

if TYPE_CHECKING:
  VersionParseResult = tuple[tuple[int, ...], BrowserVersionChannel, str]

class SafariVersion(BrowserVersion):
  _MIN_MAJOR_PARTS_LEN: Final[int] = 3
  _MIN_PARTS_LEN: Final[int] = 3
  _SIMPLE_VERSION_RE = re.compile(
      r"(?P<name>Safari(?: Technology Preview)?) "
      r"(?P<parts>(?:[\d.]+)+)", re.I)
  _COMPLEX_VERSION_RE = re.compile(r"[^\d]*"
                                   r"(?P<major_parts>[\d.]+)"
                                   #  r"[^(0-9]+ "
                                   r".*\("
                                   r"(?P<version>(Release (?P<release>\d+), )?"
                                   r"(?P<parts>[\d.]+))"
                                   r"\).*")

  @classmethod
  @override
  def _parse(cls, full_version: str) -> VersionParseResult:
    if "Safari" in full_version:
      full_version = full_version.strip()
      if matches := cls._SIMPLE_VERSION_RE.fullmatch(full_version):
        return cls._parse_simple_version(full_version, matches)
      if matches := cls._COMPLEX_VERSION_RE.fullmatch(full_version):
        return cls._parse_complex_version(full_version, matches)
    raise cls.parse_error("Could not extract version number", full_version)

  @classmethod
  def _parse_complex_version(cls, full_version: str,
                             matches) -> VersionParseResult:
    version_str = matches["version"]
    parts_str = matches["parts"]
    major_parts_str = matches["major_parts"]
    assert version_str and parts_str and major_parts_str
    channel = cls._parse_channel(full_version)
    major_parts = tuple(map(int, major_parts_str.split(".")))
    if len(major_parts) < cls._MIN_MAJOR_PARTS_LEN:
      major_parts += (0,) * (cls._MIN_MAJOR_PARTS_LEN - len(major_parts))
    major_parts = major_parts[:cls._MIN_MAJOR_PARTS_LEN]
    release = 0
    if release_str := matches["release"]:
      release = int(release_str)
    parts = cls._parse_parts(full_version, parts_str)
    parts = major_parts + (release,) + parts
    return parts, channel, f"{major_parts_str} ({version_str})"

  @classmethod
  def _parse_simple_version(cls, full_version: str,
                            matches) -> VersionParseResult:
    channel: BrowserVersionChannel = cls._parse_channel(full_version)
    parts = cls._parse_parts(full_version, matches["parts"])
    parts += (0,)
    return parts, channel, full_version

  @classmethod
  def _parse_parts(cls, full_version: str, parts_str: str) -> tuple[int, ...]:
    try:
      parts = tuple(map(int, parts_str.split(".")))
    except ValueError as e:
      raise cls.parse_error("Could not parse version number parts.",
                            full_version) from e
    if len(parts) < cls._MIN_PARTS_LEN:
      raise cls.parse_error("Invalid number of version number parts",
                            full_version)
    return parts

  @classmethod
  def _parse_channel(cls, full_version: str) -> BrowserVersionChannel:
    if "Safari Technology Preview" in full_version:
      return BrowserVersionChannel.BETA
    if " any" in full_version.lower():
      return BrowserVersionChannel.ANY
    return BrowserVersionChannel.STABLE

  @property
  @override
  def has_complete_parts(self) -> bool:
    return len(self.parts) >= self._MIN_PARTS_LEN

  @property
  def is_tech_preview(self) -> bool:
    return self.channel == BrowserVersionChannel.BETA

  @property
  def release(self) -> int:
    return self._parts[self._MIN_MAJOR_PARTS_LEN]

  @override
  def _channel_name(self, channel: BrowserVersionChannel) -> str:
    if channel == BrowserVersionChannel.STABLE:
      return "stable"
    if channel == BrowserVersionChannel.BETA:
      return "technology preview"
    raise ValueError(f"Unsupported channel: {channel}")

  @property
  @override
  def key(self) -> tuple[tuple[int, ...], BrowserVersionChannel]:
    return (self.comparable_parts(self._MIN_PARTS_LEN), self._channel)
