# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import dataclasses
import html
from argparse import ArgumentTypeError
from typing import TYPE_CHECKING, Any

from typing_extensions import override

from crossbench import path as pth
from crossbench.helper import url_helper

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.runner.actions import Actions


@dataclasses.dataclass(frozen=True)
class SplashScreenData:
  is_warmup: bool
  browser: Browser
  run_details: dict


class SplashScreen:
  NONE: SplashScreen
  MINIMAL: SplashScreen
  DETAILED: SplashScreen
  DEFAULT: SplashScreen

  @classmethod
  def parse(cls, value: str) -> SplashScreen:
    if not value or value == "default":
      return cls.DEFAULT
    if value in ("none", "skip"):
      return cls.NONE
    if value == "minimal":
      return cls.MINIMAL
    if value == "detailed":
      return cls.DETAILED
    if value.startswith("http:") or value.startswith("https:"):
      return URLSplashScreen(value)
    maybe_path = pth.LocalPath(value)
    if maybe_path.exists():
      return URLSplashScreen(maybe_path.absolute().as_uri())
    raise ArgumentTypeError(f"Unknown splashscreen: {value}")

  def run(self, action: Actions, info: SplashScreenData) -> None:
    pass


_BLANK_PAGE_HTML = "<html></html>"
_BLANK_PAGE_DATA_URL = (
    f"data:text/html;charset=utf-8,{url_helper.quote(_BLANK_PAGE_HTML)}")

class BaseURLSplashScreen(SplashScreen, metaclass=abc.ABCMeta):

  def __init__(self, timeout: float = 2) -> None:
    super().__init__()
    self._timeout = timeout

  def run(self, action: Actions, info: SplashScreenData) -> None:
    action.show_url(self.get_url(info))
    action.wait(self._timeout)
    action.show_url(_BLANK_PAGE_DATA_URL)

  @abc.abstractmethod
  def get_url(self, info: SplashScreenData) -> str:
    pass


class DetailedSplashScreen(BaseURLSplashScreen):

  @override
  def get_url(self, info: SplashScreenData) -> str:
    browser: Browser = info.browser
    title = html.escape(browser.app_name.title())
    version = html.escape(str(browser.version))
    run_type = "Run"
    bg_color = "#000"
    if info.is_warmup:
      title = f"Warmup: {title}"
      run_type = "Warmup Run"
      bg_color = "#444"
    page = "".join((
        "<html><head>"
        f"<title>{run_type} Details</title>",
        "<style>",
        "html{"
        "font-family:sans-serif;",
        f"background-color:{bg_color};",
        "color:#fff",
        "}",
        "dl{display:grid;grid-template-columns:max-content auto}",
        "dt{grid-column-start:1}",
        "dd{grid-column-start:2;font-family:monospace}",
        "</style>",
        "</head><body>",
        f"<h1>{title} {version}</h1>",
        self._render_browser_details(info),
        self._render_run_details(info),
        "</body></html>",
    ))
    data_url = f"data:text/html;charset=utf-8,{url_helper.quote(page)}"
    return data_url

  def _render_properties(self, title: str, properties: dict[str, Any]) -> str:
    section = f"<h2>{html.escape(title)}</h2><dl>"
    for property_name, value in properties.items():
      section += f"<dt>{html.escape(property_name)}</dt>"
      section += f"<dd>{html.escape(str(value))}</dd>"
    section += "</dl>"
    return section

  def _render_browser_details(self, info: SplashScreenData) -> str:
    browser: Browser = info.browser
    properties = {"User Agent": browser.user_agent(), **browser.details_json()}
    return self._render_properties("Browser Details", properties)

  def _render_run_details(self, info: SplashScreenData) -> str:
    return self._render_properties("Run Details", info.run_details)


class MinimalSplashScreen(DetailedSplashScreen):

  @override
  def _render_browser_details(self, info: SplashScreenData) -> str:
    properties = {"User Agent": info.browser.user_agent()}
    return self._render_properties("Browser Details", properties)


class URLSplashScreen(BaseURLSplashScreen):

  def __init__(self, url: str, timeout: float = 2) -> None:
    super().__init__(timeout)
    self._url = url

  @override
  def get_url(self, info: SplashScreenData) -> str:
    del info
    return self._url

  @property
  def url(self) -> str:
    return self._url


SplashScreen.NONE = SplashScreen()
SplashScreen.MINIMAL = MinimalSplashScreen()
SplashScreen.DETAILED = DetailedSplashScreen()
SplashScreen.DEFAULT = SplashScreen.DETAILED
