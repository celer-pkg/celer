# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
import datetime as dt
import logging
from typing import (TYPE_CHECKING, Any, ClassVar, Iterator, Optional, Self,
                    Sequence, cast)

from typing_extensions import override

from crossbench import path as pth
from crossbench.action_runner.action.action_type import ActionType
from crossbench.action_runner.action.get import GetAction
from crossbench.benchmarks.loading.config.blocks import (ActionBlock,
                                                         ActionBlockListConfig)
from crossbench.benchmarks.loading.config.login.custom import LoginBlock
from crossbench.benchmarks.loading.page.live import PAGES
from crossbench.benchmarks.loading.playback_controller import \
    PlaybackController
from crossbench.cli.config.secrets import Secrets
from crossbench.config import ConfigObject, ConfigParser
from crossbench.parse import DurationParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.action_runner.action.action import Action


@dataclasses.dataclass(frozen=True)
class PageConfig(ConfigObject):
  VALID_SCHEMES: ClassVar[tuple[str, ...]] = ObjectParser.COMMON_URL_SCHEMES

  label: str | None = None
  playback: PlaybackController | None = None
  secrets: Secrets = Secrets()
  login: LoginBlock | None = None
  setup: ActionBlock | None = None
  blocks: tuple[ActionBlock, ...] = tuple()
  teardown: ActionBlock | None = None

  @classmethod
  def parse_other(cls, value: Any, **kwargs) -> Self:
    if isinstance(value, (list, tuple)):
      return cls.parse_sequence(value, **kwargs)
    return super().parse_other(value)

  @classmethod
  @override
  def parse_str(  # pylint: disable=arguments-differ
      cls,
      value: str,
      label: Optional[str] = None) -> Self:
    """
    Simple comma-separated string with optional duration:
      value = URL,[DURATION]
    """
    parts = value.rsplit(",", maxsplit=1)
    duration = dt.timedelta()
    raw_url: str = parts[0]
    if raw_url in PAGES:
      url = PAGES[raw_url].url
      label = label or raw_url
    else:
      url = ObjectParser.fuzzy_url_str(raw_url)
    if len(parts) == 2:
      duration = DurationParser.positive_duration(parts[1])
    return cls.from_url(label, url, duration)

  @classmethod
  def parse_sequence(cls,
                     value: Sequence[Any],
                     label: Optional[str] = None,
                     secrets: Optional[Secrets] = None) -> Self:
    value = ObjectParser.non_empty_sequence(value, "story actions or blocks")
    blocks = ActionBlockListConfig.parse_sequence(value)
    if label is not None:
      label = ObjectParser.non_empty_str(label, "label")
    secrets = secrets or Secrets()
    return cls(label, secrets=secrets, blocks=blocks.blocks)

  @classmethod
  @override
  def parse_dict(  # pylint: disable=arguments-differ
      cls,
      config: dict[str, Any],
      label: Optional[str] = None,
      secrets: Optional[Secrets] = None,
      **kwargs) -> Self:
    config = ObjectParser.non_empty_dict(config, "story actions or blocks")
    page_config = cls.config_parser().parse(
        config, label=label, secrets=secrets, **kwargs)
    return page_config

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = ConfigParser(cls)
    parser.add_argument("label", type=ObjectParser.non_empty_str)
    parser.add_argument("playback", type=PlaybackController.parse)
    parser.add_argument("secrets", type=Secrets, default=Secrets())
    parser.add_argument("login", type=LoginBlock)
    parser.add_argument("setup", type=ActionBlock)
    parser.add_argument(
        "blocks",
        aliases=("actions", "url", "urls"),
        type=ActionBlockListConfig)
    parser.add_argument("teardown", type=ActionBlock)
    return parser

  @classmethod
  def from_url(cls,
               label: Optional[str],
               url: str,
               duration: dt.timedelta = dt.timedelta()) -> Self:
    blocks = (ActionBlock.from_url(url, duration),)
    return cls(label=label, blocks=blocks)

  def actions(self) -> Iterator[Action]:
    for block in self.blocks:
      yield from block

  @property
  def duration(self) -> dt.timedelta:
    return sum((action.duration for action in self.actions()), dt.timedelta())

  @property
  def any_label(self) -> str:
    return self.label or self.url_label

  @property
  def url_label(self) -> str:
    url = ObjectParser.url(self.first_url)
    if url.scheme == "about":
      return url.path
    if url.scheme == "file":
      return pth.LocalPath(url.path).name
    if hostname := url.hostname:
      if hostname.startswith("www."):
        return hostname[len("www."):]
      return hostname
    return str(url)

  @property
  def first_url(self) -> str:
    for action in self.actions():
      if action.TYPE == ActionType.GET:
        return cast(GetAction, action).url
    logging.debug("PageConfig: No GET action with an URL found.")
    return ""
