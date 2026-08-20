# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
from typing import TYPE_CHECKING

from typing_extensions import override

from crossbench.benchmarks.loading.config.blocks import ActionBlock
from crossbench.benchmarks.loading.page.base import DEFAULT_DURATION, PAGE_LIST
from crossbench.benchmarks.loading.page.interactive import InteractivePage
from crossbench.benchmarks.loading.playback_controller import \
    PlaybackController
from crossbench.benchmarks.loading.tab_controller import TabController

if TYPE_CHECKING:
  from crossbench.types import JsonDict


class LivePage(InteractivePage):

  @classmethod
  @override
  def all_story_names(cls) -> tuple[str, ...]:
    return tuple(page.name for page in PAGE_LIST)

  def __init__(
      self,
      name: str,
      url: str,
      duration: dt.timedelta = DEFAULT_DURATION,
      playback: PlaybackController = PlaybackController.default(),
      tabs: TabController = TabController.default(),
      about_blank_duration: dt.timedelta = dt.timedelta()
  ) -> None:
    self.url: str = url
    blocks = (ActionBlock.from_url(url, duration),)
    super().__init__(
        name,
        blocks=blocks,
        playback=playback,
        tabs=tabs,
        about_blank_duration=about_blank_duration)

  @override
  def details_json(self) -> JsonDict:
    result = super().details_json()
    result["url"] = str(self.url)
    return result

  @property
  @override
  def first_url(self) -> str:
    return self.url

  def __str__(self) -> str:
    return f"Page(name={self.name}, url={self.url})"

  def help(self) -> str:
    return f"{self.url},{self.duration.total_seconds()}s"


LIVE_PAGES = (
    LivePage("blank", "about:blank", dt.timedelta(seconds=1)),
    LivePage("amazon", "https://www.amazon.de/s?k=heizkissen",
             dt.timedelta(seconds=5)),
    LivePage("bing", "https://www.bing.com/images/search?q=not+a+squirrel",
             dt.timedelta(seconds=5)),
    LivePage("caf", "http://www.caf.fr", dt.timedelta(seconds=6)),
    LivePage("cnn", "https://cnn.com/", dt.timedelta(seconds=7)),
    LivePage("ecma262", "https://tc39.es/ecma262/#sec-numbers-and-dates",
             dt.timedelta(seconds=10)),
    LivePage("expedia", "https://www.expedia.com/", dt.timedelta(seconds=7)),
    LivePage("facebook", "https://facebook.com/shakira",
             dt.timedelta(seconds=8)),
    LivePage("google", "https://google.com/search?q=newyork",
             dt.timedelta(seconds=7)),
    LivePage("maps", "https://goo.gl/maps/TEZde4y4Hc6r2oNN8",
             dt.timedelta(seconds=10)),
    LivePage("microsoft", "https://microsoft.com/", dt.timedelta(seconds=6)),
    LivePage("provincial", "http://www.provincial.com",
             dt.timedelta(seconds=6)),
    LivePage("sueddeutsche", "https://www.sueddeutsche.de/wirtschaft",
             dt.timedelta(seconds=8)),
    LivePage("theverge", "https://www.theverge.com/", dt.timedelta(seconds=10)),
    LivePage("timesofindia", "https://timesofindia.indiatimes.com/",
             dt.timedelta(seconds=8)),
    LivePage("twitter", "https://twitter.com/wernertwertzog?lang=en",
             dt.timedelta(seconds=6)),
    LivePage(
        "wikipedia",
        "https://ja.wikipedia.org/wiki/%E3%82%AD%E3%83%A3%E3%83%83%E3%83%84_(%E6%98%A0%E7%94%BB)",
        dt.timedelta(seconds=6)),
)

assert not PAGE_LIST, "PAGE_LIST was already initialized."
PAGE_LIST.extend(LIVE_PAGES)

PAGES: dict[str, LivePage] = {page.name: page for page in LIVE_PAGES}
PAGE_LIST_SMALL: tuple[LivePage, ...] = (PAGES["facebook"], PAGES["maps"],
                                         PAGES["timesofindia"], PAGES["cnn"])
