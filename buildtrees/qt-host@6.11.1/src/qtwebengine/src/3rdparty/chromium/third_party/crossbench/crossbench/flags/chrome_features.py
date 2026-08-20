# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import Iterable, Iterator, Optional

from ordered_set import OrderedSet
from typing_extensions import override

from crossbench.flags.base import Freezable


class ChromeBaseFeatures(Freezable, abc.ABC):
  ENABLE_FLAG: str = ""
  DISABLE_FLAG: str = ""

  def __init__(self) -> None:
    super().__init__()
    self._enabled: dict[str, str | None] = {}
    self._disabled: OrderedSet[str] = OrderedSet()

  @property
  def is_empty(self) -> bool:
    return len(self._enabled) == 0 and len(self._disabled) == 0

  @property
  def enabled(self) -> dict[str, Optional[str]]:
    return dict(self._enabled)

  @property
  def disabled(self) -> OrderedSet[str]:
    return OrderedSet(self._disabled)

  def _parse_feature(self, feature: str) -> tuple[str, Optional[str]]:
    if not feature:
      raise ValueError("Cannot parse empty feature")
    if "," in feature:
      raise ValueError(f"{repr(feature)} contains multiple features. "
                       "Please split them first.")
    return self._parse_feature_parts(feature)

  @abc.abstractmethod
  def _parse_feature_parts(self, feature: str) -> tuple[str, Optional[str]]:
    pass

  def enable(self, feature: str) -> None:
    name, value = self._parse_feature(feature)
    self._enable(name, value)

  def _enable(self, name: str, value: Optional[str]) -> None:
    self.assert_not_frozen()
    if name in self._disabled:
      raise ValueError(
          f"Cannot enable previously disabled feature={repr(name)}")
    if name in self._enabled:
      prev_value = self._enabled[name]
      if value != prev_value:
        raise ValueError("Cannot set conflicting values "
                         f"({repr(prev_value)}, vs. {repr(value)}) "
                         f"for the same feature={repr(name)}")
    else:
      self._enabled[name] = value

  def disable(self, feature: str) -> None:
    self.assert_not_frozen()
    name, _ = self._parse_feature(feature)
    if name in self._enabled:
      raise ValueError(
          f"Cannot disable previously enabled feature={repr(name)}")
    self._disabled.add(name)

  def clear_enabled(self):
    self.assert_not_frozen()
    self._enabled = {}

  def clear_disabled(self):
    self.assert_not_frozen()
    self._disabled = OrderedSet()

  def update(self, other: ChromeBaseFeatures) -> None:
    if not isinstance(other, type(self)):
      raise TypeError(f"Cannot merge {type(self)} with {type(other)}")
    for disabled in other.disabled:
      self.disable(disabled)
    for name, value in other.enabled.items():
      self._enable(name, value)

  def merge(self, other: ChromeBaseFeatures) -> None:
    self.update(other)

  def items(self) -> Iterable[tuple[str, str]]:
    if self._enabled:
      yield (self.ENABLE_FLAG, self.enabled_str())
    if self._disabled:
      yield (self.DISABLE_FLAG, self.disabled_str())

  def enabled_str(self) -> str:
    return ",".join(
        k if v is None else f"{k}{v}" for k, v in self._enabled.items())

  def disabled_str(self) -> str:
    return ",".join(self._disabled)

  def __iter__(self) -> Iterator[str]:
    for flag_name, features_str in self.items():
      yield f"{flag_name}={features_str}"

  def __bool__(self) -> bool:
    return bool(self._enabled) or bool(self._disabled)

  def __str__(self) -> str:
    return " ".join(self)

  def __contains__(self, feature: str) -> bool:
    return feature in self._disabled or feature in self._enabled


class ChromeFeatures(ChromeBaseFeatures):
  """
  Chrome Features set, throws if features are enabled and disabled at the same
  time.
  Examples:
    --disable-features="MyFeature1"
    --enable-features="MyFeature1,MyFeature2"
    --enable-features="MyFeature1:k1/v1/k2/v2,MyFeature2"
    --enable-features="MyFeature3<Trial2:k1/v1/k2/v2"
  """

  ENABLE_FLAG: str = "--enable-features"
  DISABLE_FLAG: str = "--disable-features"

  @override
  def _parse_feature_parts(self, feature: str) -> tuple[str, Optional[str]]:
    parts = feature.split("<")
    if len(parts) == 2:
      return (parts[0], "<" + parts[1])
    if len(parts) != 1:
      raise ValueError(f"Invalid number of feature parts: {repr(parts)}")
    parts = feature.split(":")
    if len(parts) == 2:
      return (parts[0], ":" + parts[1])
    if len(parts) != 1:
      raise ValueError(f"Invalid number of feature parts: {repr(parts)}")
    return (feature, None)


class ChromeBlinkFeatures(ChromeBaseFeatures):
  """
  Chrome Features set, throws if features are enabled and disabled at the same
  time.
  Examples:
    --disable-blink-features="MyFeature1"
    --enable-blink-features="MyFeature1,MyFeature2"
  """

  ENABLE_FLAG: str = "--enable-blink-features"
  DISABLE_FLAG: str = "--disable-blink-features"

  @override
  def _parse_feature_parts(self, feature: str) -> tuple[str, Optional[str]]:
    if "<" in feature or ":" in feature:
      raise ValueError("blink features do not have params, "
                       f"but found param separator in {repr(feature)}")
    return (feature, None)
