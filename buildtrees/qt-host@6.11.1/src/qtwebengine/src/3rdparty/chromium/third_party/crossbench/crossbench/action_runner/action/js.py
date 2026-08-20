# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
import logging
from typing import TYPE_CHECKING, Optional, Type

from typing_extensions import override

from crossbench.action_runner.action.action import (ACTION_TIMEOUT, Action,
                                                    ActionT)
from crossbench.action_runner.action.action_type import ActionType
from crossbench.parse import ObjectParser, PathParser
from crossbench.replacements import Replacements

if TYPE_CHECKING:
  import datetime as dt

  from crossbench import path as pth
  from crossbench.action_runner.base import ActionRunner
  from crossbench.config import ConfigParser
  from crossbench.runner.run import Run
  from crossbench.types import JsonDict

class JsAction(Action):
  TYPE: ActionType = ActionType.JS

  @classmethod
  @override
  @functools.cache
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument("script", type=ObjectParser.non_empty_str)
    parser.add_argument(
        "script_path", aliases=("path",), type=PathParser.existing_file_path)
    parser.add_argument("replacements", aliases=("replace",), type=Replacements)
    return parser

  def __init__(self,
               script: Optional[str],
               script_path: Optional[pth.LocalPath],
               replacements: Optional[Replacements] = None,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._original_script = script
    self._script_path = script_path
    self._final_script = ""
    self._replacements = replacements
    if bool(script) == bool(script_path):
      raise ValueError(
          f"One of {self}.script or {self}.script_path, but not both, "
          "have to specified. ")
    if script:
      self._final_script = script
    elif script_path:
      self._final_script = script_path.read_text()
      logging.debug("Loading script from %s: %s", script_path, script)
      # TODO: Support argument injection into shared file script.
    if replacements:
      self._final_script = replacements.apply(self._final_script)
    super().__init__(timeout, index)

  @property
  def script(self) -> str:
    return self._final_script

  @override
  def run_with(self, run: Run, action_runner: ActionRunner) -> None:
    action_runner.js(run, self)

  @override
  def validate(self) -> None:
    super().validate()
    if not self.script:
      raise ValueError(
          f"{self}.script is missing or the provided script file is empty.")

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    if self._original_script:
      details["script"] = self._original_script
    if self._script_path:
      details["script_path"] = str(self._script_path)
    if self._replacements:
      details["replacements"] = self._replacements.to_json()
    return details
