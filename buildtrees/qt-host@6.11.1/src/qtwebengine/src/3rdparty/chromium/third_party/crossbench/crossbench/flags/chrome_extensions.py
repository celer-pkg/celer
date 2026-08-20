# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum
from typing import Any, Final, Iterable, Optional

from ordered_set import OrderedSet

from crossbench.flags.base import Freezable


@enum.unique
class ExtensionsMode(enum.StrEnum):
  DISABLED = "disabled"
  DEFAULT = "default"
  ENABLED = "enabled"
  ENABLED_SELECTIVE = "enabled_selective"


class ChromeExtensions(Freezable):

  DISABLE_FLAG: Final[str] = "--disable-extensions"
  LOAD_FLAG: Final[str] = "--load-extension"
  DISABLE_EXCEPT_FLAG: Final[str] = "--disable-extensions-except"

  ENABLE_FLAGS: Final[tuple[str, ...]] = (LOAD_FLAG, DISABLE_EXCEPT_FLAG)
  FLAGS: Final[tuple[str, ...]] = ENABLE_FLAGS + (DISABLE_FLAG,)

  def __init__(self, extensions: Optional[Iterable[str]] = None) -> None:
    super().__init__()
    self._mode: ExtensionsMode = ExtensionsMode.DEFAULT
    self._extensions: OrderedSet[str] = OrderedSet(extensions or tuple())

  def disable(self) -> None:
    self.assert_not_frozen()
    if (self._mode not in (ExtensionsMode.DEFAULT,
                           ExtensionsMode.DISABLED)) or self.extensions:
      raise ValueError(
          f"Cannot disable previously enabled extensions: {self.extensions}")
    self._mode = ExtensionsMode.DISABLED

  @property
  def extensions(self) -> tuple[str, ...]:
    return tuple(self._extensions)

  @property
  def extensions_str(self) -> str:
    return ",".join(self._extensions)

  @property
  def disabled(self) -> bool:
    return self._mode == ExtensionsMode.DISABLED

  @property
  def enabled(self) -> bool:
    return self._mode in (ExtensionsMode.ENABLED,
                          ExtensionsMode.ENABLED_SELECTIVE)

  @property
  def mode(self) -> str:
    return str(self._mode)

  def add(self, extension: str, should_override: bool = False) -> None:
    self.assert_not_frozen()
    if not extension:
      raise ValueError("Cannot add empty extension")
    if should_override:
      self._mode = ExtensionsMode.DEFAULT
    else:
      if self._mode == ExtensionsMode.DISABLED:
        raise ValueError(f"Extensions are disabled, cannot add {extension}")
      if self._mode == ExtensionsMode.ENABLED_SELECTIVE:
        raise ValueError(
            f"Cannot enable additional extension {repr(extension)}, "
            f"currently they are restricted to {self.extensions_str}")
    assert self._mode in (ExtensionsMode.DEFAULT, ExtensionsMode.ENABLED)
    self._mode = ExtensionsMode.ENABLED
    self._extensions.add(extension)

  def enable(self,
             extensions: str | None,
             selective: bool = False,
             should_override: bool = False) -> None:
    self.assert_not_frozen()
    if not extensions:
      raise ValueError("Cannot enable empty extensions")
    for extension in extensions.split(","):
      self.add(extension, should_override=should_override)
    if selective:
      assert self._mode in (ExtensionsMode.DEFAULT, ExtensionsMode.ENABLED)
      self._mode = ExtensionsMode.ENABLED_SELECTIVE

  def merge(self, other: ChromeExtensions) -> None:
    if not isinstance(other, ChromeExtensions):
      raise TypeError
    if self.mode != other.mode:
      raise ValueError("Cannot merge extensions with different modes: "
                       f"{self.mode} and {other.mode}")
    for extension in other.extensions:
      self.add(extension, should_override=True)

  def __getitem__(self, key: Any) -> Any:
    if key == self.DISABLE_FLAG:
      return None
    if key == self.LOAD_FLAG:
      return None
    if key == self.DISABLE_EXCEPT_FLAG:
      return None
    raise KeyError(f"Unsupported extension flag: {key}")

  def set(self,
          flag_name: str,
          flag_value: Optional[str] = None,
          should_override: bool = False) -> None:
    self.assert_not_frozen()
    self._verify_flag(flag_name, flag_value)
    match flag_name:
      case self.DISABLE_FLAG:
        self.disable()
      case self.LOAD_FLAG:
        self.enable(flag_value, should_override=should_override)
      case self.DISABLE_EXCEPT_FLAG:
        self.enable(flag_value, selective=True, should_override=should_override)
      case _:
        raise ValueError(f"Unsupported extension flag: {flag_name}")

  def _verify_flag(self, flag_name: str, value: Optional[str]) -> Optional[str]:
    if flag_name == self.DISABLE_FLAG:
      if value:
        raise ValueError(f"{flag_name} expects no value, but got {repr(value)}")
      enable_flag, _ = self.item()
      if enable_flag:
        raise ValueError(
            f"Existing {enable_flag} conflicts with {flag_name}. "
            "Cannot explicitly enable and disable at the same time.")
      return None

    if flag_name not in self.ENABLE_FLAGS:
      raise ValueError(f"Unsupported extensions flag {flag_name}")
    if not value:
      raise ValueError(f"{flag_name} expects a value, but got {repr(value)}")
    if self.disabled:
      raise ValueError(
          f"Existing --disable-extensions conflicts with {flag_name}. "
          "Cannot explicitly disable and enable at the same time.")
    enable_flag, enabled_extensions = self.item()
    if enable_flag and (enable_flag != flag_name or
                        enabled_extensions != value):
      raise ValueError("Got previously enabled extensions "
                       f"{enable_flag}={enabled_extensions}, "
                       "which conflicts with nwe "
                       f"{flag_name}={value}.")
    return value

  def items(self) -> Iterable[tuple[str, str | None]]:
    flag, value = self.item()
    if flag:
      yield flag, value

  def item(self) -> tuple[str | None, str | None]:
    match self._mode:
      case ExtensionsMode.DEFAULT:
        return None, None
      case ExtensionsMode.DISABLED:
        return (self.DISABLE_FLAG, None)
      case ExtensionsMode.ENABLED:
        return (self.LOAD_FLAG, self.extensions_str)
      case ExtensionsMode.ENABLED_SELECTIVE:
        return (self.DISABLE_EXCEPT_FLAG, self.extensions_str)
    raise RuntimeError(f"Invalid extensions state: {self._mode}")

  def __str__(self) -> str:
    flag, value = self.item()
    if not flag:
      return ""
    if value is None:
      return flag
    return f"{flag}={value}"

  def __bool__(self):
    return bool(self.extensions) or self._mode is not ExtensionsMode.DEFAULT
