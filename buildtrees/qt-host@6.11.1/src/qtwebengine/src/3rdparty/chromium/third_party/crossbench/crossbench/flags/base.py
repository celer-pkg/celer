# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import collections
import re
from typing import (Any, Iterable, Iterator, Optional, Self, Set, TypeAlias,
                    TypeVar, Union)

from typing_extensions import override


class FrozenFlagsError(RuntimeError):
  pass


class Freezable:

  def __init__(self, *args, **kwargs) -> None:
    self._frozen = False
    super().__init__(*args, **kwargs)

  def __hash__(self) -> int:
    self.freeze()
    return hash(str(self))

  @property
  def is_frozen(self) -> bool:
    return self._frozen

  def freeze(self: Self) -> Self:
    self._frozen = True
    return self

  def assert_not_frozen(self, msg: Optional[str] = None) -> None:
    if not self._frozen:
      return
    if not msg:
      msg = f"Cannot modify frozen {type(self).__name__}"
    raise FrozenFlagsError(msg)



FlagsData: TypeAlias = Union[None, dict[str, str], "Flags",
                             Iterable[str | tuple[str, str | None]]]


class BasicFlags(Freezable, collections.UserDict):
  """Basic implementation for command line flags (similar to Dic[str, str].

  This class is mostly used to make sure command-line flags for browsers
  don't end up having contradicting values.
  """

  _WHITE_SPACE_RE = re.compile(r"\s+")
  _BASIC_FLAG_NAME_RE = re.compile(r"(--?)[^\s=-][^\s=]*")
  # Handles space-separated flags: --foo="1" --bar  --baz='2'  --boo=3
  _VALUE_PATTERN = (r"('(?P<value_single_quotes>[^']*)')|"
                    r"(\"(?P<value_double_quotes>[^\"]*)\")|"
                    r"(?P<value_no_quotes>[^'\" ]+)")
  _END_OR_SEPARATOR_PATTERN = r"(\s*\s\s*|$)"
  _PARSE_RE = re.compile(fr"(?P<name>{_BASIC_FLAG_NAME_RE.pattern})"
                         fr"((?P<equal>=)({_VALUE_PATTERN})?)?"
                         fr"{_END_OR_SEPARATOR_PATTERN}")

  @classmethod
  def split(cls, flag_str: str) -> tuple[str, Optional[str]]:
    if "=" in flag_str:
      flag_name, flag_value = flag_str.split("=", maxsplit=1)
      return (flag_name, flag_value)
    return (flag_str, None)

  @classmethod
  def parse(cls, data: Any) -> Self:
    if isinstance(data, cls):
      return data
    if isinstance(data, str):
      return cls.parse_str(data)
    return cls(data)

  @classmethod
  def parse_str(cls, raw_flags: str) -> Self:
    return cls._parse_str(raw_flags)

  @classmethod
  def _parse_str(cls, raw_flags: str, msg: str = "flag") -> Self:
    raw_flags = raw_flags.strip()
    if not raw_flags:
      return cls()
    flag_parts: list[tuple[str, str | None]] = []
    current_end: int | None = None
    for match in cls._PARSE_RE.finditer(raw_flags):
      if current_end is None:
        if match.start() != 0:
          part = raw_flags[:match.start()]
          raise ValueError(f"Invalid {msg} part at pos=0: {repr(part)}")
      elif current_end != match.start():
        raise ValueError(f"Invalid {msg}: could not consume all data")
      current_end = match.end()

      groups = match.groupdict()
      maybe_flag_name: str | None = groups.get("name")
      if not maybe_flag_name:
        raise ValueError(f"Invalid {msg}: {repr(raw_flags)}")
      # Re-assign since pytype doesn't remove the Optional.
      flag_name: str = maybe_flag_name
      flag_value: str | None = (
          groups.get("value_single_quotes") or
          groups.get("value_double_quotes") or groups.get("value_no_quotes"))
      if groups.get("equal") and not flag_value:
        raise ValueError(f"Invalid {msg}: missing value for {repr(flag_name)}")
      assert flag_name
      flag_parts.append((flag_name, flag_value))

    if current_end != len(raw_flags):
      part = raw_flags[current_end:]
      raise ValueError(
          f"Invalid {msg} part at pos={current_end or 0}: {repr(part)}")
    return cls(flag_parts)

  def __init__(self, initial_data: FlagsData = None) -> None:
    super().__init__(initial_data)

  def __setitem__(self, flag_name: str, flag_value: Optional[str]) -> None:
    return self.set(flag_name, flag_value)

  def set(self,
          flag_name: str,
          flag_value: Optional[str] = None,
          should_override: bool = False) -> None:
    self._set(flag_name, flag_value, should_override)

  def _set(self,
           flag_name: str,
           flag_value: Optional[str] = None,
           should_override: bool = False) -> None:
    self.assert_not_frozen()
    self._validate_flag_name(flag_name)
    if flag_value:
      self._validate_flag_value(flag_name, flag_value)
    self._validate_override(flag_name, flag_value, should_override)
    self.data[flag_name] = flag_value

  def _validate_flag_name(self, flag_name: str) -> None:
    if not flag_name:
      raise ValueError("Cannot set empty flag")
    if self._WHITE_SPACE_RE.search(flag_name):
      raise ValueError(
          f"Flag name cannot contain whitespaces: {repr(flag_name)}")
    if "=" in flag_name:
      raise ValueError(
          f"Flag name contains '=': {repr(flag_name)}, please split")
    if flag_name[0] != "-":
      raise ValueError(
          f"Flag name must begin with a '-', but got {repr(flag_name)}")
    if not self._BASIC_FLAG_NAME_RE.fullmatch(flag_name):
      raise ValueError(
          f"Flag name contains invalid characters: {repr(flag_name)}")

  def _validate_flag_value(self, flag_name: str, flag_value: str) -> None:
    assert flag_value, "Got invalid empty flag_value."
    if not isinstance(flag_value, str):
      raise TypeError(
          f"Expected None or string flag-value for flag {flag_name}, "
          f"but got: {repr(flag_value)}")

  def _validate_override(self, flag_name: str, flag_value: Optional[str],
                         should_override: bool) -> None:
    if should_override or flag_name not in self:
      return
    old_value = self[flag_name]
    if flag_value != old_value:
      raise ValueError(f"Flag {flag_name}={repr(flag_value)} was already set "
                       f"with a different previous value: {repr(old_value)}")

  # pylint: disable=arguments-differ
  def update(  # type: ignore
      self,
      initial_data: FlagsData = None,
      should_override: bool = False) -> None:
    # pylint: disable=arguments-differ
    if initial_data is None:
      return
    if isinstance(initial_data, (Flags, dict)):
      for flag_name, flag_value in initial_data.items():
        self.set(flag_name, flag_value, should_override)
    else:
      for flag_name_or_items in initial_data:
        if isinstance(flag_name_or_items, str):
          self.set(flag_name_or_items, None, should_override)
        else:
          flag_name, flag_value = flag_name_or_items
          self.set(flag_name, flag_value, should_override)

  def merge(self, other: FlagsData) -> None:
    self.update(other)

  def copy(self: Self) -> Self:
    return self.__class__(self)

  def merge_copy(self, other: FlagsData):
    ret = self.copy()
    ret.merge(other)
    return ret

  def filtered(self: Self, flag_names: Iterable[str]) -> Self:
    flag_names_set: Set[str] = set(flag_names)
    filtered_flags = {k: v for k, v in self.items() if k in flag_names_set}
    return self.__class__(filtered_flags)

  def contains_without_value(self, key: str) -> bool:
    return key in self.data and self.data[key] is None

  def _describe(self, flag_name: str) -> str:
    value = self.get(flag_name)
    if value is None:
      return flag_name
    return f"{flag_name}={value}"

  @override
  def items(self) -> Iterable[tuple[str, Optional[str]]]:  # type: ignore
    return self.data.items()

  def to_dict(self) -> dict[str, Optional[str]]:
    return dict(self.items())

  def clear(self):
    self.data.clear()

  def __iter__(self) -> Iterator[str]:
    for k, v in self.items():
      if v is None:
        yield k
      else:
        yield f"{k}={v}"

  def __bool__(self) -> bool:
    return bool(self.data)

  def __repr__(self) -> str:
    dict_repr = repr(self.to_dict())
    return f"{type(self).__name__}({dict_repr})"

  def __str__(self) -> str:
    return " ".join(self)

  def freeze(self: Self) -> Self:
    self.validate()
    super().freeze()
    return self

  def validate(self) -> None:
    pass


class Flags(BasicFlags):
  """
  Subclass with slightly stricter flag name checking.
  Most command-line programs adhere to this.
  """
  _FLAG_NAME_RE = re.compile(r"(--?)[a-zA-Z0-9][a-zA-Z0-9_-]*")
  _PARSE_RE = re.compile(fr"(?P<name>{_FLAG_NAME_RE.pattern})"
                         fr"((?P<equal>=)({BasicFlags._VALUE_PATTERN})?)?"
                         fr"{BasicFlags._END_OR_SEPARATOR_PATTERN}")

  @override
  def _validate_flag_name(self, flag_name: str) -> None:
    super()._validate_flag_name(flag_name)
    if not self._FLAG_NAME_RE.fullmatch(flag_name):
      raise ValueError(
          f"Flag name contains invalid characters: {repr(flag_name)}")


FlagsT = TypeVar("FlagsT", bound=Flags)
