# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Any, Self

from typing_extensions import override

from crossbench import exception
from crossbench.config import ConfigObject
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  from crossbench.types import JsonDict


class Replacements(ConfigObject):

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    raise ValueError("Cannot parse replacements from string")

  @classmethod
  @override
  def parse_dict(cls, config: dict[str, Any], **kwargs) -> Self:
    dict_value = ObjectParser.dict(config)
    for replace_key, replace_value in dict_value.items():
      with exception.annotate_argparsing(
          f"Parsing ...[{repr(replace_key)}] = {repr(config)}"):
        ObjectParser.non_empty_str(replace_key, "replacement key")
        ObjectParser.not_none(replace_value, "replacement value")
    return cls(config)

  def __init__(self, replacements: dict[str, Any]) -> None:
    self._replacements = replacements

  def apply(self, raw_value: str) -> str:
    final_value: str = raw_value

    if self._replacements:
      for key, value in self._replacements.items():
        final_value = final_value.replace(key, str(value))

    return final_value

  def to_json(self) -> JsonDict:
    return self._replacements
