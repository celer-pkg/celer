# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import re
from typing import Optional, Self

from typing_extensions import override

from crossbench.flags.base import Flags


class JSFlags(Flags):
  """Custom flags implementation for V8 flags (--js-flags in chrome)

  Additionally to the base Flag implementation it asserts that bool flags
  with the --no-.../--no... prefix are not contradicting each other.
  """
  _NO_PREFIX = "--no"
  _NAME_RE = re.compile(r"--[a-zA-Z_][a-zA-Z0-9_-]+")

  # We allow two forms:
  # - space separated: --foo="1" --bar  --baz='2'  --boo=3
  # - comma separated: --foo="1",--bar ,--baz='2', --boo=3
  _VALUE_PATTERN = (r"('(?P<value_single_quotes>[^',]+)')|"
                    r"(\"(?P<value_double_quotes>[^\",]+)\")|"
                    r"(?P<value_no_quotes>[^'\", =]+)")
  _END_OR_SEPARATOR_PATTERN = r"(\s*[,\s]\s*|$)"
  _PARSE_RE = re.compile(fr"(?P<name>{_NAME_RE.pattern})"
                         fr"((?P<equal>=)({_VALUE_PATTERN})?)?"
                         fr"{_END_OR_SEPARATOR_PATTERN}")

  @classmethod
  @override
  def parse_str(cls, raw_flags: str) -> Self:
    return cls._parse_str(raw_flags, "--js-flags")

  @override
  def _set(self,
           flag_name: str,
           flag_value: Optional[str] = None,
           should_override: bool = False) -> None:
    self._validate_js_flag_name(flag_name)
    if flag_value is not None:
      self._validate_js_flag_value(flag_name, flag_value)
    self._check_negated_flag(flag_name, should_override)
    super()._set(flag_name, flag_value, should_override)

  def _validate_js_flag_value(self, flag_name: str, flag_value: str) -> None:
    if not isinstance(flag_value, str):
      raise TypeError("JSFlag value must be str, "
                      f"but got {type(flag_value)}: {repr(flag_value)}")
    if "," in flag_value:
      raise ValueError(
          "--js-flags: Comma in V8 flag value, flag escaping for chrome's "
          f"--js-flags might not work: {flag_name}={repr(flag_value)}")
    if self._WHITE_SPACE_RE.search(flag_value):
      raise ValueError("--js-flags: V8 flag-values cannot contain whitespaces:"
                       f"{flag_name}={repr(flag_value)}")

  def _validate_js_flag_name(self, flag_name: str) -> None:
    if not flag_name.startswith("--"):
      raise ValueError("--js-flags: Only long-form flag names allowed, "
                       f"but got {repr(flag_name)}")
    if not self._NAME_RE.fullmatch(flag_name):
      raise ValueError(f"--js-flags: Invalid flag name {repr(flag_name)}. \n"
                       "Check invalid characters in the V8 flag name?")

  def _check_negated_flag(self, flag_name: str, should_override: bool) -> None:
    if flag_name.startswith(self._NO_PREFIX):
      enabled = flag_name[len(self._NO_PREFIX):]
      # Check for --no-foo form
      if enabled.startswith("-"):
        enabled = enabled[1:]
      enabled = "--" + enabled
      if should_override:
        del self[enabled]
      elif enabled in self:
        raise ValueError(
            f"Conflicting flag {flag_name}, "
            f"it has already been enabled by {repr(self._describe(enabled))}")
    else:
      # --foo => --no-foo
      disabled = f"--no-{flag_name[2:]}"
      if disabled not in self:
        # Try compact version: --foo => --nofoo
        disabled = f"--no{flag_name[2:]}"
        if disabled not in self:
          return
      if should_override:
        del self[disabled]
      else:
        raise ValueError(f"Conflicting flag {flag_name}, "
                         "it has previously been disabled by "
                         f"{repr(self._describe(flag_name))}")

  def __str__(self) -> str:
    return ",".join(self)
