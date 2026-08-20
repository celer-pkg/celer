# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
import datetime as dt
import logging
from typing import TYPE_CHECKING, Any, ClassVar, Optional, Self, Sequence

from typing_extensions import override

from crossbench import exception
from crossbench import path as pth
from crossbench.action_runner.action.click import ClickAction
from crossbench.action_runner.action.enums import ReadyState
from crossbench.action_runner.action.get import GetAction
from crossbench.action_runner.action.position import PositionConfig
from crossbench.action_runner.action.wait import WaitAction
from crossbench.benchmarks.loading.config.blocks import ActionBlock
from crossbench.benchmarks.loading.config.page import PageConfig
from crossbench.benchmarks.loading.input_source import InputSource
from crossbench.cli.config.secrets import Secrets
from crossbench.config import ConfigObject
from crossbench.parse import DurationParseError, DurationParser, ObjectParser

if TYPE_CHECKING:
  import urllib.parse as urlparse

  from crossbench.action_runner.action.action import Action


@dataclasses.dataclass(frozen=True)
class PagesConfig(ConfigObject):
  pages: tuple[PageConfig, ...] = ()
  secrets: Secrets | None = None

  @override
  def validate(self) -> None:
    super().validate()
    for index, page in enumerate(self.pages):
      assert isinstance(page, PageConfig), (
          f"pages[{index}] is not a PageConfig but {type(page).__name__}")

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    """
    Variant 1: Full inline hjson:
      { ... }
    Variant 2: Simple comma-separate config:
      value = URL, [DURATION], ...
    """
    value = ObjectParser.non_empty_str(value)
    values: list[str] = []
    previous_part: str | None = None
    for part in value.strip().split(","):
      part = ObjectParser.non_empty_str(part, "url or duration")
      try:
        DurationParser.positive_duration(part)
        if not previous_part:
          raise argparse.ArgumentTypeError(
              "Duration can only follow after url. "
              f"Current value: {repr(part)}")
        values[-1] = f"{previous_part},{part}"
        previous_part = None
      except DurationParseError:
        # part is likely a URL
        previous_part = part
        values.append(part)
    return cls.parse_sequence(values)

  @classmethod
  def parse_any_url(cls, url: urlparse.ParseResult, **kwargs) -> Self:
    # We might get comma-separate URL lists here.
    return cls.parse_str(url.geturl(), **kwargs)

  @classmethod
  def parse_any_path(cls, path: pth.LocalPath, **kwargs) -> Self:
    # Make sure we get errors for invalid files.
    return cls.parse_config_path(path, **kwargs)

  @classmethod
  def parse_other(cls, value: Any, **kwargs) -> Self:
    if isinstance(value, (list, tuple)):
      return cls.parse_sequence(value, **kwargs)
    return super().parse_other(value, **kwargs)

  @classmethod
  def parse_sequence(cls, values: Sequence[str]) -> Self:
    """
    Variant a): List of comma-separate URLs
      [ "URL,[DURATION]", ... ]
    """
    # TODO: support parsing a list of PageConfig dicts
    if not values:
      raise argparse.ArgumentTypeError("Got empty page list.")
    ObjectParser.non_empty_sequence(values, "page list")
    pages: list[PageConfig] = []
    for index, single_line_config in enumerate(values):
      with exception.annotate_argparsing(
          f"Parsing pages[{index}]: {repr(single_line_config)}"):
        pages.append(PageConfig.parse_str(single_line_config))
    return cls(pages=tuple(pages))

  @classmethod
  @override
  def parse_dict(cls, config: dict, **kwargs) -> Self:
    """
    Variant a):
      { "pages": { "LABEL": PAGE_CONFIG }, "secrets": { ... } }
    """
    with exception.annotate_argparsing("Parsing stories"):
      if "pages" not in config:
        raise argparse.ArgumentTypeError(
            "Config does not provide a 'pages' dict.")
      secrets: Secrets | None = None
      if secrets_data := config.get("secrets"):
        secrets = Secrets.parse(secrets_data)
      pages_config = ObjectParser.non_empty_dict(config["pages"], "pages")
      with exception.annotate_argparsing("Parsing config 'pages'"):
        pages = cls._parse_pages(pages_config, secrets)
        return cls(pages, secrets)
    raise exception.UnreachableError()

  @classmethod
  def _parse_pages(cls,
                   data: dict[str, Any],
                   secrets: Optional[Secrets] = None) -> tuple[PageConfig, ...]:
    pages = []
    for name, page_config in data.items():
      with exception.annotate_argparsing(f"Parsing story ...['{name}']"):
        # TODO: fix secrets on the inner page and on the outer pages config
        page = PageConfig.parse(page_config, label=name, secrets=secrets)
        pages.append(page)
    return tuple(pages)

  def __eq__(self, value: object) -> bool:
    if not isinstance(value, PagesConfig):
      return False
    return self.pages == value.pages and self.secrets == value.secrets


class DevToolsRecorderPagesConfig(PagesConfig):

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    raise NotImplementedError()

  @classmethod
  @override
  def parse_dict(cls, config: dict[str, Any], **kwargs) -> Self:
    config = ObjectParser.non_empty_dict(config)
    with exception.annotate_argparsing("Loading DevTools recording file"):
      title = ObjectParser.non_empty_str(config["title"], "title")
      actions = cls._parse_steps(config["steps"])
      # Use default block
      blocks = (ActionBlock(actions=actions),)
      pages = (PageConfig(label=title, blocks=blocks),)
      return cls(pages)
    raise exception.UnreachableError()

  @classmethod
  def _parse_steps(cls, steps: list[dict[str, Any]]) -> tuple[Action, ...]:
    actions: list[Action] = []
    for step in steps:
      if maybe_actions := cls.parse_step(step):
        actions.extend(maybe_actions)
        # TODO(cbruni): make this configurable
        actions.append(WaitAction(duration=dt.timedelta(seconds=1)))
    return tuple(actions)

  @classmethod
  def parse_step(cls, step: dict[str, Any]) -> list[Action]:
    step_type: str = step["type"]
    default_timeout = dt.timedelta(seconds=10)
    if step_type == "navigate":
      return [cls._parse_navigate_step(step, default_timeout)]
    if step_type == "click":
      return [cls._parse_click_step(step, default_timeout)]
    if step_type == "setViewport":
      # Resizing is ignored for now.
      return []
    raise ValueError(f"Unsupported step: {step_type}")

  @classmethod
  def _parse_navigate_step(cls, step: dict[str, Any],
                           default_timeout: dt.timedelta) -> Action:
    del default_timeout
    return GetAction(  # type: ignore
        step["url"], ready_state=ReadyState.COMPLETE)

  @classmethod
  def _parse_click_step(cls, step: dict[str, Any],
                        default_timeout: dt.timedelta) -> Action:
    selector = cls._parse_selectors(step["selectors"])
    return ClickAction(
        InputSource.JS,
        position=PositionConfig.from_selector(
            selector=selector, scroll_into_view=True),
        timeout=default_timeout)

  @classmethod
  def _parse_selectors(cls, selectors: list[list[str]]) -> str:
    xpath: str | None = None
    aria: str | None = None
    text: str | None = None
    css: str | None = None
    # Detect all single-element selectors first.
    for selector_list in selectors:
      if len(selector_list) != 1:
        continue
      selector_candidate = selector_list[0]
      if not aria and selector_candidate.startswith("aria/"):
        aria = selector_candidate
      elif not xpath and selector_candidate.startswith("xpath//"):
        xpath = selector_candidate
      elif not text and selector_candidate.startswith("css/"):
        css = selector_candidate
      elif not text and selector_candidate.startswith("text/"):
        text = selector_candidate
      elif not text and selector_candidate.startswith("pierce/"):
        # not supported yet.
        pass
      else:
        css = f"css/{selector_candidate}"

    if xpath:
      assert xpath.startswith("xpath/")
      return xpath
    if css:
      _, css = css.split("css/", maxsplit=1)
      return css
    if aria:
      _, aria = aria.split("aria/", maxsplit=1)
      return f"[aria-label={repr(aria)}]"
    if text:
      _, text = text.split("text/", maxsplit=1)
      return f"xpath///*[text()={repr(text)}]"

    raise ValueError("Need at least one single element xpath or aria "
                     "selector for click action")


class ListPagesConfig(PagesConfig):

  VALID_EXTENSIONS: ClassVar[tuple[str, ...]] = (".txt", ".list")

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    raise argparse.ArgumentTypeError(
        f"URL list file {repr(value)} does not exist.")

  @classmethod
  @override
  def parse_path(cls, path: pth.LocalPath, **kwargs) -> Self:
    assert not kwargs, f"{cls.__name__} does not support extra kwargs"
    pages: list[PageConfig] = []
    with exception.annotate_argparsing(f"Loading Pages list file: {path.name}"):
      line: int = 0
      with path.open() as f:
        for single_line_config in f.readlines():
          with exception.annotate_argparsing(f"Parsing line {line}"):
            line += 1
            single_line_config = single_line_config.strip()
            if not single_line_config:
              logging.warning("Skipping empty line %s", line)
              continue
            pages.append(PageConfig.parse(single_line_config))
    return cls(pages=tuple(pages))

  @classmethod
  @override
  def parse_dict(cls, config: dict, **kwargs) -> Self:
    config = ObjectParser.non_empty_dict(config, "pages")
    with exception.annotate_argparsing("Parsing scenarios / pages"):
      if "pages" not in config:
        raise argparse.ArgumentTypeError(
            "Config does not provide a 'pages' dict.")
      pages = config["pages"]
      if isinstance(pages, str):
        pages = [pages]
      if not isinstance(pages, (list, tuple)):
        raise argparse.ArgumentTypeError(
            f"Expected list/tuple for pages, but got {type(pages)}")
      return cls.parse_sequence(pages)
    raise exception.UnreachableError()
