# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import contextlib
import logging
from typing import TYPE_CHECKING, Iterator, Optional, TypeVar
from urllib.parse import urlparse

from typing_extensions import override

from crossbench import exception
from crossbench import path as pth
from crossbench.cli import ui
from crossbench.network.base import Network
from crossbench.parse import PathParser

if TYPE_CHECKING:
  from crossbench import plt
  from crossbench.network.traffic_shaping.base import TrafficShaper
  from crossbench.path import LocalPath
  from crossbench.runner.groups.session import BrowserSessionRunGroup


GS_PREFIX = "gs://"

ReplayNetworkT = TypeVar("ReplayNetworkT", bound="ReplayNetwork")

class ReplayNetwork(Network):
  """ A network implementation that can be used to replay requests
  from a an archive."""

  def __init__(self,
               archive: pth.LocalPath | str,
               traffic_shaper: Optional[TrafficShaper] = None,
               browser_platform: Optional[plt.Platform] = None) -> None:
    super().__init__(traffic_shaper, browser_platform)
    self._archive_path = self._ensure_archive(archive)

  @property
  @override
  def is_wpr(self) -> bool:
    return True

  @property
  def archive_path(self) -> LocalPath:
    return self._archive_path

  @contextlib.contextmanager
  @override
  def open(self: ReplayNetworkT,
           session: BrowserSessionRunGroup) -> Iterator[ReplayNetworkT]:
    with exception.annotate(f"Starting {type(self).__name__}"):
      with super().open(session):
        with self._open_replay_server(session):
          with self._traffic_shaper.open(self, session):
            yield self

  @contextlib.contextmanager
  def _open_replay_server(self, session: BrowserSessionRunGroup):
    del session
    yield

  def _generate_filename(self, url: str) -> str:
    blob = self.host_platform.prepare_gcs_request(url)
    if md5 := blob.md5_hash:
      safe_md5 = pth.safe_filename(md5)
      url_path = pth.AnyPosixPath(urlparse(url).path)
      return f"{url_path.stem}_{safe_md5}{url_path.suffix}"
    raise RuntimeError(f"Could not find md5 hash in blob: {url}")

  def _download_gcloud_archive(self, url: str) -> LocalPath:
    title: str = f"Downloading {url}"
    with exception.annotate(title), ui.spinner(title=title):
      local_path = (
          self.host_platform.local_cache_dir("wpr") /
          self._generate_filename(url))
      if local_path.is_file():
        logging.info("Found cached WPR archive: %s", local_path)
        return local_path
      logging.info("Downloading WPR archive from %s to %s", url, local_path)
      self.host_platform.download_gcs_file(url, local_path)
    return local_path

  def _ensure_archive(self, archive: pth.LocalPath | str) -> LocalPath:
    if isinstance(archive, str) and archive.startswith(GS_PREFIX):
      return self._download_gcloud_archive(url=archive)
    return PathParser.existing_file_path(archive).resolve()
