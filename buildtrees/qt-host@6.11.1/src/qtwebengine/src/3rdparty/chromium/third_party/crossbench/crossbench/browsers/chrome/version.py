# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import re
from typing import Iterable, Optional, Self

from typing_extensions import override

from crossbench.browsers.chromium.version import ChromiumVersion


class ChromeVersion(ChromiumVersion):

  _PREFIX_RE = re.compile(
      r"(?:google )?chr(?:ome(?: for testing)?)?[- ]?"
      r"(?:latest)?[- ]?"
      rf"(?:{ChromiumVersion._CHANNEL_RE.pattern})?[- ]?m?", re.I)

  @classmethod
  @override
  def _validate_prefix(cls, prefix: Optional[str]) -> bool:
    if not prefix:
      return True
    prefix = prefix.lower()
    if prefix.strip() == "m":
      return True
    return (bool(cls._PREFIX_RE.fullmatch(prefix)) or
            super()._validate_prefix(prefix))

  @classmethod
  @override
  def _validate_suffix(cls, suffix: Optional[str]) -> bool:
    if suffix and "(Official Build)" in suffix:
      return True
    return super()._validate_suffix(suffix)

  @classmethod
  def dev(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls.alpha(parts, version_str)

  @classmethod
  def canary(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls.pre_alpha(parts, version_str)
