# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import re
from typing import Final, Optional

from typing_extensions import override

from crossbench.browsers.version import BrowserVersion, BrowserVersionChannel


class D8Version(BrowserVersion):
  _PARTS_LEN: Final[int] = 3
  _VERSION_RE = re.compile(
      r"(?P<prefix>V8\D+)"
      r"(?P<parts>(?:\d+(\.\d+)+))"
      r"(?:.*)", re.I)

  @classmethod
  @override
  def _parse(
      cls,
      full_version: str) -> tuple[tuple[int, ...], BrowserVersionChannel, str]:
    matches = cls._VERSION_RE.fullmatch(full_version.strip())
    if not matches:
      raise cls.parse_error("Could not extract version number", full_version)
    prefix = matches["prefix"]
    if not cls._validate_prefix(prefix):
      raise cls.parse_error(f"Wrong prefix {repr(prefix)}", full_version)
    version_parts = matches["parts"]
    assert version_parts
    parts: tuple[int, ...] = tuple(map(int, version_parts.split(".")))
    if len(parts) != cls._PARTS_LEN:
      raise cls.parse_error("Invalid number of version number parts",
                            full_version)
    version_str: str = f"{prefix} {parts}"
    return parts, BrowserVersionChannel.ALPHA, version_str

  @classmethod
  def _validate_prefix(cls, prefix: Optional[str]) -> bool:
    if not prefix:
      return True
    return "V8" in prefix

  @override
  def _channel_name(self, channel: BrowserVersionChannel) -> str:
    if channel == BrowserVersionChannel.ALPHA:
      return "default"
    raise ValueError(f"Unsupported channel: {channel}")

  @property
  @override
  def has_complete_parts(self) -> bool:
    return len(self.parts) == 3

  @property
  @override
  def key(self) -> tuple[tuple[int, ...], BrowserVersionChannel]:
    return (self.comparable_parts(self._PARTS_LEN), self._channel)
