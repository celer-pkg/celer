# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
import os
import shutil
import stat
import tempfile
from typing import TYPE_CHECKING

from selenium import webdriver
from selenium.webdriver.edge.options import Options as EdgeOptions
from selenium.webdriver.edge.service import Service as EdgeService
from typing_extensions import override

import crossbench
import crossbench.exception
from crossbench import path as pth
from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.chromium.webdriver import ChromiumBasedWebDriver
from crossbench.browsers.edge.base import EdgeBaseMixin

if TYPE_CHECKING:
  from selenium.webdriver.chromium.webdriver import ChromiumDriver

  from crossbench import plt


class EdgeWebDriver(EdgeBaseMixin, ChromiumBasedWebDriver):

  WEB_DRIVER_OPTIONS = EdgeOptions
  WEB_DRIVER_SERVICE = EdgeService

  @classmethod
  @override
  def type_name(cls) -> str:
    return "edge"

  @override
  def _find_driver(self) -> pth.AnyPath:
    finder = EdgeWebDriverDownloader(self)
    return finder.download()

  @override
  def _create_driver(
      self,
      options: EdgeOptions,  # type: ignore
      service: EdgeService) -> ChromiumDriver:  # type: ignore
    return webdriver.Edge(options=options, service=service)

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return (BrowserAttributes.EDGE | BrowserAttributes.CHROMIUM_BASED
            | BrowserAttributes.WEBDRIVER)


class EdgeWebDriverDownloader:
  BASE_URL = "https://msedgedriver.microsoft.com"

  def __init__(self, browser: EdgeWebDriver) -> None:
    self.browser = browser
    self.platform: plt.Platform = browser.platform
    assert self.browser.is_local, (
        "Cannot download chromedriver for remote browser yet")
    self.extension: str = ""
    if self.platform.is_win:
      self.extension = ".exe"
    cache_dir = self.platform.host_platform.local_cache_dir("driver")
    self.driver_path: pth.LocalPath = (
        cache_dir / f"edgedriver-{self.browser.version.major}{self.extension}")

  def download(self) -> pth.LocalPath:
    if not self.driver_path.exists():
      with crossbench.exception.annotate(
          f"Downloading edgedriver for {self.browser.version}"):
        self._download()
    return self.driver_path

  def _download(self) -> None:
    arch = self._arch_identifier()
    archive_name = f"edgedriver_{arch}.zip"
    url = self.BASE_URL + f"/{self.browser.version.parts_str}/{archive_name}"
    logging.info("EDGEDRIVER downloading %s: %s", self.browser.version, url)
    with tempfile.TemporaryDirectory() as tmp_dir:
      archive_file = pth.LocalPath(tmp_dir) / archive_name
      self.platform.download_to(url, archive_file)
      unpack_dir = pth.LocalPath(tmp_dir) / "extracted"
      shutil.unpack_archive(os.fspath(archive_file), os.fspath(unpack_dir))
      driver = unpack_dir / f"msedgedriver{self.extension}"
      assert driver.is_file(), (f"Extracted driver at {driver} does not exist.")
      shutil.move(os.fspath(driver), os.fspath(self.driver_path))
      self.driver_path.chmod(self.driver_path.stat().st_mode | stat.S_IEXEC)

  def _arch_identifier(self) -> str:
    if self.platform.is_linux:
      assert self.platform.is_x64, "edgedriver is only available on linux x64"
      return "linux64"
    if self.platform.is_macos:
      if self.platform.is_arm64:
        return "mac64_m1"
      if self.platform.is_x64:
        return "mac64"
    elif self.platform.is_win:
      if self.platform.is_x64:
        return "win64"
      if self.platform.is_ia32:
        return "win32"
      if self.platform.is_arm64:
        return "arm64"
    raise ValueError(f"Unsupported edgedriver platform {self.platform}")
