# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import re
import shlex
from dataclasses import dataclass
from typing import TYPE_CHECKING, Iterable, Set

import crossbench.path as pth
from crossbench.parse import ObjectParser
from crossbench.probes.probe import Probe, ProbeConfigParser, ProbeContext
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


class DownloadsProbe(Probe):
  """
  Probe that captures downloads from websites and allows loading tests to wait
  for a download to complete.
  """
  NAME = "downloads"
  RESULT_LOCATION = ResultLocation.BROWSER

  CHROME_OS_DOWNLOADS_DIR = pth.AnyPath("/home/chronos/user/MyFiles/Downloads")

  @classmethod
  def config_parser(cls) -> ProbeConfigParser:
    parser = super().config_parser()
    parser.add_argument(
        "clear_downloads",
        aliases=("clear",),
        type=ObjectParser.bool,
        default=False,
        help="Delete all files in the download folder before every run.")
    parser.add_argument(
        "save_downloads",
        aliases=("save",),
        type=ObjectParser.bool,
        default=True,
        help=("Copy all files downloaded during a test to the test results"
              " folder."))
    return parser

  def __init__(self,
               clear_downloads: bool = False,
               save_downloads: bool = False) -> None:
    super().__init__()
    self._clear_downloads: bool = clear_downloads
    self._save_downlaods: bool = save_downloads

  def get_context(self, run: Run) -> DownloadsProbeContext:
    if run.browser_platform.is_android:
      return AndroidWebDriverDownloadsProbeContext(self, run)

    if run.browser_platform.is_chromeos:
      return FileWatchDownloadsProbeContext(self, run,
                                            self.CHROME_OS_DOWNLOADS_DIR)
    raise NotImplementedError(
        f"Probe({self}): Unsupported browser: {run.browser}")

  @property
  def clear_downloads(self) -> bool:
    return self._clear_downloads

  @property
  def save_downloads(self) -> bool:
    return self._save_downlaods


class DownloadsProbeContext(ProbeContext[DownloadsProbe]):

  def __init__(self, probe: DownloadsProbe, run: Run) -> None:
    super().__init__(probe, run)

  def get_default_result_path(self) -> pth.AnyPath:
    downloads_dir = super().get_default_result_path()
    self.browser_platform.mkdir(downloads_dir)
    return downloads_dir

  @abc.abstractmethod
  def download_complete(self, pattern: re.Pattern) -> bool:
    pass


class FileWatchDownloadsProbeContext(DownloadsProbeContext):

  def __init__(self, probe: DownloadsProbe, run: Run,
               downloads_dir: pth.AnyPath) -> None:
    super().__init__(probe, run)
    self._downloads_dir: pth.AnyPath = downloads_dir
    self._existing_downloads: Set[pth.AnyPath] = set()
    self._results: list[pth.AnyPath] = []

  def downloads(self, include_pending: bool = True) -> Iterable[pth.AnyPath]:
    downloads = self.browser_platform.iterdir(self._downloads_dir)
    if include_pending:
      return downloads
    return [file for file in downloads if file.suffix != ".crdownload"]

  def start(self) -> None:
    if self.probe.clear_downloads:
      for file in self.downloads():
        self.browser_platform.rm(file)
      self._existing_downloads = set()
    else:
      self._existing_downloads = set(self.downloads())

  def stop(self) -> None:
    if not self.probe.save_downloads:
      return
    for file in self.downloads():
      if file in self._existing_downloads:
        continue
      to_path = self.result_path / file.name
      self.browser_platform.copy_file(file, to_path)
      self._results.append(to_path)

  def teardown(self) -> ProbeResult:
    return self.browser_result(file=self._results)

  def download_complete(self, pattern: re.Pattern) -> bool:
    return any(pattern.search(file.name) for file in self.downloads())


@dataclass(frozen=True)
class AndroidDownload:
  id: str
  display_name: str


class AndroidWebDriverDownloadsProbeContext(DownloadsProbeContext):
  CONTENT_QUERY_RE = re.compile(r"Row: \d+ _display_name=(.*), _id=(\d+)")
  CONTENT_QUERY_NO_RESULTS = "No result found."

  def __init__(self, probe: DownloadsProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._existing_downloads: Set[AndroidDownload] = set()
    self._user_id: str = str(self.browser_platform.user_id())
    self._results: list[pth.AnyPath] = []

  def downloads(self,
                include_pending: bool = True) -> Iterable[AndroidDownload]:
    result: list[AndroidDownload] = []
    args = [
        "content", "query", "--user", self._user_id, "--uri",
        "content://media/external/downloads", "--where", "is_download=1",
        "--projection", "_display_name:_id"
    ]
    if not include_pending:
      args.append("--where")
      args.append("is_pending=0")
    rows = self.browser_platform.sh_stdout(*args)
    if rows.strip() == self.CONTENT_QUERY_NO_RESULTS:
      return result
    for row in rows.splitlines():
      if match := self.CONTENT_QUERY_RE.match(row):
        result.append(
            AndroidDownload(display_name=match.group(1), id=match.group(2)))
      else:
        raise RuntimeError(
            f"Android downloads content query unexpect result row: {row}")

    return result

  def delete(self, download: AndroidDownload) -> None:
    self.browser_platform.sh("content", "delete", "--user", self._user_id,
                             "--uri", "content://media/external/downloads",
                             "--where", f"_id='{download.id}'")

  def start(self) -> None:
    if self.probe.clear_downloads:
      for download in self.downloads():
        self.delete(download)
    self._existing_downloads = set(self.downloads())

  def stop(self) -> None:
    if not self.probe.save_downloads:
      return
    for download in self.downloads():
      if download in self._existing_downloads:
        continue
      to_path = self.result_path / download.display_name
      read_downloads_cmd = (
          "content",
          "read",
          "--user",
          self._user_id,
          "--uri",
          f"content://media/external/downloads/{download.id}",
      )
      cmd = (
          shlex.join(read_downloads_cmd) + ">" +
          shlex.quote(self.browser_platform.path(to_path).as_posix()))
      self.browser_platform.sh(cmd, shell=True)
      self._results.append(to_path)

  def teardown(self) -> ProbeResult:
    return self.browser_result(file=self._results)

  def download_complete(self, pattern: re.Pattern) -> bool:
    downloads = self.downloads(include_pending=False)
    return any(pattern.search(download.display_name) for download in downloads)
