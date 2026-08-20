# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import functools
from typing import TYPE_CHECKING, Optional, Type

from typing_extensions import override

from crossbench.action_runner.action.action import (ACTION_TIMEOUT, Action,
                                                    ActionT)
from crossbench.parse import NumberParser, ObjectParser

if TYPE_CHECKING:
  import datetime as dt
  import re

  from crossbench.config import ConfigParser
  from crossbench.types import JsonDict


class BaseTabAction(Action, metaclass=abc.ABCMeta):

  @classmethod
  @override
  @functools.lru_cache(maxsize=1)
  def config_parser(cls: Type[ActionT]) -> ConfigParser[ActionT]:
    parser = super().config_parser()
    parser.add_argument(
        "tab_index",
        type=NumberParser.any_int,
        help=(
            "The index of the tab. Tabs are indexed in creation "
            "order. Negative values are allowed, e.g. -1 is the most recently "
            "opened tab."))
    parser.add_argument(
        "relative_tab_index",
        type=NumberParser.any_int,
        help=("The index of the tab, relative to the current tab. -1 means the"
              "tab created just before this one."))
    parser.add_argument("title", type=ObjectParser.regexp)
    parser.add_argument("url", type=ObjectParser.regexp)
    return parser

  def __init__(self,
               tab_index: Optional[int] = None,
               relative_tab_index: Optional[int] = None,
               title: Optional[re.Pattern] = None,
               url: Optional[re.Pattern] = None,
               timeout: dt.timedelta = ACTION_TIMEOUT,
               index: int = 0) -> None:
    self._tab_index = tab_index
    self._title = title
    self._url = url
    self._relative_tab_index = relative_tab_index
    super().__init__(timeout, index)

  @property
  def title(self) -> Optional[re.Pattern]:
    return self._title

  @property
  def url(self) -> Optional[re.Pattern]:
    return self._url

  @property
  def tab_index(self) -> Optional[int]:
    return self._tab_index

  @property
  def relative_tab_index(self) -> Optional[int]:
    return self._relative_tab_index

  @override
  def to_json(self) -> JsonDict:
    details = super().to_json()
    if self._tab_index:
      details["tab_index"] = self._tab_index
    if self._title:
      details["title"] = str(self._title.pattern)
    if self._url:
      details["url"] = str(self._url.pattern)
    if self._relative_tab_index is not None:
      details["relative_tab_index"] = self._relative_tab_index
    return details
