# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import dataclasses
import enum
import functools
import re
from typing import Any, Final, Iterable, Optional, Self

from typing_extensions import override


@dataclasses.dataclass
class _BrowserVersionChannelMixin:
  label: str
  index: int


@functools.total_ordering
class BrowserVersionChannel(_BrowserVersionChannelMixin, enum.Enum):
  # Explicit channel enums:
  LTS = ("lts", 0)
  STABLE = ("stable", 1)
  BETA = ("beta", 2)
  ALPHA = ("alpha", 3)
  PRE_ALPHA = ("pre-alpha", 4)
  # Use as sentinel if the channel can be ignored:
  ANY = ("any", 5)

  def __str__(self) -> str:
    return self.label

  def __lt__(self, other: Any) -> bool:
    if not isinstance(other, BrowserVersionChannel):
      raise TypeError("BrowserVersionChannel can not be compared to {other}")
    return self.index < other.index

  def __hash__(self) -> int:
    return hash(self.name)

  def matches(self, other: BrowserVersionChannel) -> bool:
    if BrowserVersionChannel.ANY in (self, other):
      return True
    return self == other


class BrowserVersionParseError(ValueError):

  def __init__(self, name: str, msg: str, version: str) -> None:
    self._version = version
    super().__init__(f"Invalid {name} {repr(version)}: {msg}")


class PartialBrowserVersionError(ValueError):

  def __init__(self, version: BrowserVersion):
    self.version = version
    msg = f"Got partial browser version: {repr(version)}"
    super().__init__(msg)


class BrowserVersionNoChannelError(ValueError):
  pass


_VERSION_DIGITS_ONLY_RE: re.Pattern[str] = re.compile(r"\d+(\.\d+)*")


@functools.total_ordering
class BrowserVersion(abc.ABC):

  _MAX_PART_VALUE: Final[int] = 0xFFFF

  _parts: tuple[int, ...]
  _channel: BrowserVersionChannel
  _version_str: str

  @classmethod
  def parse_unique(cls, value: str) -> Self:
    """Parse a unique version identifier for a browser.
    Unlike the parse() method, this should only parse input values that can
    be unambiguously associated with a specific BrowserVersion."""
    if _VERSION_DIGITS_ONLY_RE.fullmatch(str(value)):
      raise cls.parse_error(
          "Ambiguous version, missing browser specific prefix or suffix", value)
    return cls.parse(value)

  @classmethod
  def parse(cls,
            value: str,
            channel: Optional[BrowserVersionChannel] = None) -> Self:
    (parts, parsed_channel, version_str) = cls._parse(value)
    parts = cls._validate_parts(parts, value)
    return cls(parts, channel or parsed_channel, version_str)

  @classmethod
  def _validate_parts(cls, parts: Iterable[int], value: str) -> tuple[int, ...]:
    if parts is None:
      raise cls.parse_error("Invalid version format", value)
    parts_tpl = tuple(parts)
    for part in parts_tpl:
      if part < 0:
        raise cls.parse_error("Version parts must be positive", value)
    return parts_tpl

  @classmethod
  def is_valid_unique(cls, value: str) -> bool:
    try:
      cls.parse_unique(value)
      return True
    except BrowserVersionParseError:
      return False

  @classmethod
  @abc.abstractmethod
  def _parse(
      cls,
      full_version: str) -> tuple[tuple[int, ...], BrowserVersionChannel, str]:
    pass

  @classmethod
  def parse_error(cls, msg: str, version: str) -> BrowserVersionParseError:
    return BrowserVersionParseError(cls.__name__, msg, version)

  @classmethod
  def any(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls(parts, BrowserVersionChannel.ANY, version_str)

  @classmethod
  def lts(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls(parts, BrowserVersionChannel.LTS, version_str)

  @classmethod
  def stable(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls(parts, BrowserVersionChannel.STABLE, version_str)

  @classmethod
  def beta(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls(parts, BrowserVersionChannel.BETA, version_str)

  @classmethod
  def alpha(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls(parts, BrowserVersionChannel.ALPHA, version_str)

  @classmethod
  def pre_alpha(cls, parts: Iterable[int], version_str: str = "") -> Self:
    return cls(parts, BrowserVersionChannel.PRE_ALPHA, version_str)

  def __init__(self,
               parts: Iterable[int],
               channel: BrowserVersionChannel = BrowserVersionChannel.STABLE,
               version_str: str = "") -> None:
    self._parts = self._validate_parts(parts, version_str or repr(parts))
    self._channel = channel
    self._version_str = version_str

  @property
  def parts(self) -> tuple[int, ...]:
    return self._parts

  @property
  def version_str(self) -> str:
    return self._version_str

  @property
  def parts_str(self) -> str:
    return ".".join(map(str, self._parts))

  def comparable_parts(self, padded_len) -> tuple[int, ...]:
    if self.is_complete:
      return self._parts
    padding = (self._MAX_PART_VALUE,) * (padded_len - len(self._parts))
    return self._parts + padding

  @property
  def is_complete(self) -> bool:
    return self.has_complete_parts and self.has_channel

  @property
  @abc.abstractmethod
  def has_complete_parts(self) -> bool:
    pass

  @property
  def is_unknown(self) -> bool:
    # Only True for UnknownBrowserVersion
    return False

  @property
  def is_channel_version(self) -> bool:
    return not self._parts and self.has_channel

  @property
  def major(self) -> int:
    if not self._parts:
      raise PartialBrowserVersionError(self)
    return self._parts[0]

  @property
  def minor(self) -> int:
    if len(self._parts) <= 1:
      raise PartialBrowserVersionError(self)
    return self._parts[1]

  @property
  def channel(self) -> BrowserVersionChannel:
    if not self.has_channel:
      raise BrowserVersionNoChannelError(
          f"BrowserVersion {self} has no channel")
    return self._channel

  def matches_channel(self, channel: BrowserVersionChannel) -> bool:
    return self._channel.matches(channel)

  @property
  def has_channel(self) -> bool:
    return self._channel is not BrowserVersionChannel.ANY

  @property
  def is_lts(self) -> bool:
    return self._channel == BrowserVersionChannel.LTS

  @property
  def is_stable(self) -> bool:
    return self._channel == BrowserVersionChannel.STABLE

  @property
  def is_beta(self) -> bool:
    return self._channel == BrowserVersionChannel.BETA

  @property
  def is_alpha(self) -> bool:
    return self._channel == BrowserVersionChannel.ALPHA

  @property
  def is_pre_alpha(self) -> bool:
    return self._channel == BrowserVersionChannel.PRE_ALPHA

  @property
  def channel_name(self) -> str:
    if not self.has_channel:
      return "any"
    return self._channel_name(self._channel)

  @abc.abstractmethod
  def _channel_name(self, channel: BrowserVersionChannel) -> str:
    pass

  @property
  def key(self) -> tuple[tuple[int, ...], BrowserVersionChannel]:
    return (self._parts, self._channel)

  def with_channel(self, channel: BrowserVersionChannel) -> Self:
    if self.channel == channel:
      return self
    return type(self)(self.parts, channel, self.version_str)  # pytype: disable=not-instantiable

  def __str__(self) -> str:
    if not self._version_str:
      if not self._parts:
        return self.channel_name
      return f"{self.parts_str} {self.channel_name}"
    return f"{self._version_str} {self.channel_name}"

  def __repr__(self) -> str:
    return (
        f"{self.__class__.__name__}"
        f"({self.parts_str}, {self.channel_name}, {repr(self._version_str)})")

  def is_compatible_type(self, other: BrowserVersion) -> bool:
    return isinstance(other, type(self)) or isinstance(self, type(other))

  def __eq__(self, other: Any) -> bool:
    if not self.is_compatible_type(other):
      return False
    return self.key == other.key

  def __le__(self, other: Any) -> bool:
    if not self.is_compatible_type(other):
      raise TypeError("Cannot compare versions from unrelated browsers: "
                      f"{type(self).__name__} vs. "
                      f"{type(other).__name__}.")
    if self.is_channel_version and other.is_channel_version:
      return self._channel <= other._channel
    if self.is_channel_version:
      raise ValueError(f"Cannot compare channel {self} against {other}")
    if other.is_channel_version:
      raise ValueError(f"Cannot compare {self} against channel {other}")
    return self.key <= other.key

  def contains(self, other: BrowserVersion) -> bool:
    if not isinstance(other, type(self)):
      raise TypeError("Cannot compare versions from different browsers: "
                      f"{self} vs. {other}.")
    if self == other:
      return True
    if self.has_channel and other.has_channel:
      if self.channel != other.channel:
        return False
    # A less precise version (e.g. channel or partial version) can never be
    # part of a more complete version.
    other_parts = other.parts
    common_part_len = min(len(self._parts), len(other_parts))
    if common_part_len < len(self._parts):
      return False
    return self._parts[:common_part_len] == other_parts[:common_part_len]


class UnknownBrowserVersion(BrowserVersion):
  """Sentinel helper object for initializing version variables before
  knowing which exact browser/version is used."""

  def __init__(self,
               parts: tuple[int, ...] = (),
               channel: BrowserVersionChannel = BrowserVersionChannel.ANY,
               version_str: str = "unknown") -> None:
    super().__init__(parts, BrowserVersionChannel.ANY, version_str)

  @classmethod
  @override
  def _parse(
      cls,
      full_version: str) -> tuple[tuple[int, ...], BrowserVersionChannel, str]:
    raise RuntimeError("UnknownBrowserVersion does not support parsing")

  @override
  def _channel_name(self, channel: BrowserVersionChannel) -> str:
    return "unknown"

  @property
  @override
  def has_complete_parts(self) -> bool:
    return False

  @property
  @override
  def is_unknown(self) -> bool:
    return True
