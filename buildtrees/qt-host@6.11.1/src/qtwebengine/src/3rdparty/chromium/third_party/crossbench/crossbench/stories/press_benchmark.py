# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import logging
from typing import TYPE_CHECKING, Optional, Self, Sequence

from typing_extensions import override

from crossbench.parse import ObjectParser
from crossbench.stories.story import Story

if TYPE_CHECKING:
  from crossbench.runner.run import Run


class PressBenchmarkStory(Story, metaclass=abc.ABCMeta):
  NAME: str = ""
  URL: str = ""
  URL_OFFICIAL: str = ""
  URL_LOCAL: str = ""
  SUBSTORIES: tuple[str, ...] = ()

  @classmethod
  @override
  def all_story_names(cls) -> tuple[str, ...]:
    assert cls.SUBSTORIES
    return cls.SUBSTORIES

  @classmethod
  def default_story_names(cls) -> tuple[str, ...]:
    """Override this method to use a subset of all_story_names as default
    selection if no story names are provided."""
    return cls.all_story_names()

  @classmethod
  def all(cls,
          separate: bool = False,
          url: Optional[str] = None,
          **kwargs) -> list[Self]:
    return cls.from_names(cls.all_story_names(), separate, url, **kwargs)

  @classmethod
  def default(cls,
              separate: bool = False,
              url: Optional[str] = None,
              **kwargs) -> list[Self]:
    return cls.from_names(cls.default_story_names(), separate, url, **kwargs)

  @classmethod
  def from_names(cls,
                 substories: Sequence[str],
                 separate: bool = False,
                 url: Optional[str] = None,
                 **kwargs) -> list[Self]:
    if not substories:
      raise ValueError("No substories provided")
    if separate:
      return [
          cls(  # pytype: disable=not-instantiable
              url=url,
              substories=[substory],
              **kwargs) for substory in substories
      ]
    return [
        cls(  # pytype: disable=not-instantiable
            url=url,
            substories=substories,
            **kwargs)
    ]

  def __init__(self,
               *args,
               substories: Sequence[str] = (),
               duration: Optional[float] = None,
               url: Optional[str] = None,
               **kwargs) -> None:
    cls = self.__class__
    assert self.SUBSTORIES, f"{cls}.SUBSTORIES is not set."
    assert self.NAME is not None, f"{cls}.NAME is not set."
    self._verify_url(self.URL, "URL")
    self._verify_url(self.URL_OFFICIAL, "URL_OFFICIAL")
    self._verify_url(self.URL_LOCAL, "URL_LOCAL")
    assert substories, f"No substories provided for {cls}"
    self._substories: tuple[str, ...] = tuple(substories)
    self._verify_substories()
    kwargs["name"] = self._get_unique_name()
    kwargs["duration"] = duration or self._get_initial_duration()
    super().__init__(*args, **kwargs)
    # If the _custom_url is empty, we generate a matching URL when the
    # local file server is used.
    self._custom_url: str | None = url

  def _get_unique_name(self) -> str:
    substories_set = set(self._substories)
    if substories_set == set(self.default_story_names()):
      return self.NAME
    if substories_set == set(self.all_story_names()):
      name = "all"
    else:
      name = "_".join(self._substories)
    if len(name) > 220:
      # Crop the name and add some random hash bits
      name = name[:220] + hex(hash(name))[2:10]
    return name

  def _get_initial_duration(self) -> dt.timedelta:
    # Fixed delay for startup costs
    startup_delay = dt.timedelta(seconds=2)
    # Add some slack due to different story lengths
    story_factor = 0.5 + 1.1 * len(self._substories)
    return startup_delay + (story_factor * self.substory_duration)

  def get_run_url(self, run: Run) -> str:
    if self._custom_url:
      # TODO: check that we have a live network / url host matches network host
      return self._custom_url
    network = run.browser_session.network
    # Create a matching URL for a local file server.
    if network.is_local_file_server and network.http_port:
      return f"http://{network.host}:{network.http_port}"
    # Return default URL in case of live network.
    return self.url

  @property
  @override
  def substories(self) -> tuple[str, ...]:
    return tuple(self._substories)

  @property
  def has_default_substories(self) -> bool:
    return tuple(self.substories) == self.default_story_names()

  @property
  def fast_duration(self) -> dt.timedelta:
    """Expected benchmark duration on fast machines.
    Keep this low enough to not have to wait needlessly at the end of a
    benchmark.
    """
    return self.duration / 3

  @property
  def slow_duration(self) -> dt.timedelta:
    """Max duration that covers run-times on slow machines and/or
    debug-mode browsers.
    Making this number too large might cause needless wait times on broken
    browsers/benchmarks.
    """
    return dt.timedelta(seconds=15) + self.duration * 5

  @property
  @abc.abstractmethod
  def substory_duration(self) -> dt.timedelta:
    pass

  @property
  def url(self) -> str:
    return self._custom_url or self.URL

  def _verify_url(self, url: str, property_name: str) -> None:
    cls = self.__class__
    assert url is not None, f"{cls}.{property_name} is not set."

  def _verify_substories(self) -> None:
    ObjectParser.unique_sequence(self._substories, "substories", ValueError)
    if self._substories == self.SUBSTORIES:
      return
    for substory in self._substories:
      assert substory in self.SUBSTORIES, (f"Unknown {self.NAME} substory %s" %
                                           substory)

  @override
  def log_run_details(self, run: Run) -> None:
    super().log_run_details(run)
    self.log_run_test_url(run)

  @property
  def test_url(self) -> str:
    return self.URL

  def log_run_test_url(self, run: Run) -> None:
    del run
    if self.url != self.test_url:
      logging.info("🔗 STORY URL:                %s", self.url)
    logging.info("🔗 STORY PUBLIC TEST URL:    %s", self.test_url)
