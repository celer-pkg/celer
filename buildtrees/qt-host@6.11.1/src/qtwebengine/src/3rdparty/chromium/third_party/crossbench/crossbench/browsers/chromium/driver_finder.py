# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
import os
import re
import shutil
import stat
import tempfile
import zipfile
from typing import TYPE_CHECKING, Final, Optional

from crossbench import exception
from crossbench import path as pth
from crossbench.browsers.chrome.version import ChromeVersion
from crossbench.browsers.chromium_based import helper
from crossbench.helper import url_helper

if TYPE_CHECKING:
  from crossbench.browsers.chromium_based.webdriver import \
      ChromiumBasedWebDriver
  from crossbench.browsers.version import BrowserVersion
  from crossbench.plt.base import Platform


class ChromeDriverFinder:

  # Using CFT as abbreviation for Chrome For Testing here.
  CFT_MIN_MILESTONE = 115
  CFT_BASE_URL: str = "https://googlechromelabs.github.io/chrome-for-testing"
  CFT_VERSION_URL: str = f"{CFT_BASE_URL}/{{version}}.json"
  CFT_LATEST_URL: str = f"{CFT_BASE_URL}/LATEST_RELEASE_{{major}}"

  CFT_PLATFORM: Final[dict[tuple[str, str], str]] = {
      ("linux", "x64"):
          "linux64",
      ("macos", "x64"):
          "mac-x64",
      ("macos", "arm64"):
          "mac-arm64",
      ("win", "ia32"):
          "win32",
      ("win", "x64"):
          "win64",
      # TODO(crbug/418674629): There is currently no Windows ARM64 version of
      # Chrome for Testing, so we get ChromeDriver for win64 instead.
      ("win", "arm64"):
          "win64",
  }

  def __init__(self, browser: ChromiumBasedWebDriver) -> None:
    self.browser = browser
    self.platform: Platform = browser.platform
    self.host_platform: Platform = browser.platform.host_platform
    extension: str = ""
    if self.host_platform.is_win:
      extension = ".exe"
    cache_dir = self.host_platform.local_cache_dir("driver")
    self.driver_path: pth.LocalPath = (
        cache_dir / f"chromedriver-{self.browser.version.major}{extension}")
    self._validate_browser()

  def _validate_browser(self) -> None:
    browser_platform = self.browser.platform
    if browser_platform.is_local:
      return
    # Some remote platforms rely on a local chromedriver
    if (browser_platform.is_android or browser_platform.is_remote_ssh):
      return
    raise RuntimeError("Cannot download chromedriver for remote browser yet")

  def find_local_build(self) -> pth.LocalPath:
    assert self.browser.app_path
    # assume it's a local build
    lookup_dir: pth.LocalPath = self.host_platform.local_path(
        self.browser.app_path.parent)
    driver_path = lookup_dir / "chromedriver"
    if self.host_platform.is_win:
      driver_path = driver_path.with_suffix(".exe")
    if self.host_platform.is_file(driver_path):
      return driver_path
    error_message: list[str] = [f"Driver '{driver_path}' does not exist."]
    if helper.is_build_dir(lookup_dir, self.host_platform):
      error_message += [helper.build_chromedriver_instructions(lookup_dir)]
    else:
      error_message += ["Please manually provide a chromedriver binary."]
    raise DriverNotFoundError("\n".join(error_message))

  def download(self) -> pth.LocalPath:
    if not self.host_platform.is_file(self.driver_path):
      with exception.annotate(
          f"Downloading chromedriver for {self.browser.version}"):
        self._download()
    return self.driver_path

  def _download(self) -> None:
    milestone = self.browser.version.major
    logging.info("CHROMEDRIVER Downloading from %s v%s",
                 self.browser.type_name(), milestone)
    if self._try_download_cft(milestone):
      return
    if self._try_download_pre_115_stable(milestone):
      return
    if self._try_download_canary(milestone):
      return
    raise DriverNotFoundError(
        "Please manually compile/download chromedriver for "
        f"{self.browser.type_name()} {self.browser.version}")

  def _try_download_cft(self, milestone: int) -> bool:
    if milestone < self.CFT_MIN_MILESTONE:
      return False
    listing_url, url = self._get_cft_url(milestone)
    return self._try_download_url(milestone, url, listing_url)

  def _try_download_pre_115_stable(self, milestone: int) -> bool:
    listing_url, url = self._get_pre_115_stable_url(milestone)
    return self._try_download_url(milestone, url, listing_url)

  def _try_download_canary(self, milestone: int) -> bool:
    listing_url, url = self._get_canary_url()
    return self._try_download_url(milestone, url, listing_url)

  def _try_download_url(self, milestone: int, url: Optional[str],
                        listing_url: Optional[str]) -> bool:
    if not url:
      return False
    logging.info("CHROMEDRIVER Downloading M%s: %s", milestone, listing_url or
                 url)
    maybe_driver: pth.LocalPath | None = None
    with tempfile.TemporaryDirectory() as tmp_dir:
      if ".zip" not in url:
        maybe_driver = pth.LocalPath(tmp_dir) / "chromedriver"
        self.host_platform.download_to(url, maybe_driver)
      else:
        maybe_driver = self._download_zip_url(url, tmp_dir)
      if not maybe_driver or not maybe_driver.is_file():
        raise DriverNotFoundError(
            f"Extracted driver at {maybe_driver} does not exist.")
      self.driver_path.parent.mkdir(parents=True, exist_ok=True)
      shutil.move(os.fspath(maybe_driver), os.fspath(self.driver_path))
      self.driver_path.chmod(self.driver_path.stat().st_mode | stat.S_IEXEC)
    return True

  def _download_zip_url(self, url: str,
                        tmp_dir: str) -> Optional[pth.LocalPath]:
    zip_file = pth.LocalPath(tmp_dir) / "download.zip"
    self.host_platform.download_to(url, zip_file)
    with zipfile.ZipFile(zip_file, "r") as zip_ref:
      zip_ref.extractall(zip_file.parent)
    zip_file.unlink()
    candidates: list[pth.LocalPath] = [
        path for path in zip_file.parent.glob("**/*")
        if path.is_file() and "chromedriver" in path.name
    ]
    # Find exact match first:
    maybe_drivers: list[pth.LocalPath] = [
        path for path in candidates if path.stem == "chromedriver"
    ]
    # Backup less strict matching:
    maybe_drivers += candidates
    if not maybe_drivers:
      return None
    return maybe_drivers[0]

  def _get_cft_url(self, milestone: int) -> tuple[str, Optional[str]]:
    logging.debug("ChromeDriverFinder: Looking up chrome-for-testing version.")
    platform_name: str | None = self.CFT_PLATFORM.get(self.host_platform.key)
    if not platform_name:
      raise DriverNotFoundError(
          f"Unsupported platform {self.host_platform.key} for chromedriver.")
    listing_url, version_data = self._get_cft_version_data(milestone)
    download_url: str | None = None
    if version_data:
      download_url = self._get_cft_driver_download_url(version_data,
                                                       platform_name)
    return (listing_url, download_url)

  def _get_cft_version_data(self, milestone: int) -> tuple[str, Optional[dict]]:
    logging.debug("ChromeDriverFinder: Trying direct download url")
    listing_url, data = self._get_cft_precise_version_data(self.browser.version)
    if data:
      return listing_url, data
    logging.debug(
        "ChromeDriverFinder: Invalid precise version url %s, "
        "using M%s", listing_url, milestone)
    return self._get_ctf_milestone_data(milestone)

  def _get_cft_precise_version_data(
      self, version: BrowserVersion) -> tuple[str, Optional[dict]]:
    version_url: str = self.CFT_VERSION_URL.format(version=version.parts_str)
    try:
      response = url_helper.get(version_url)
      version_data = response.json()
      return (version_url, version_data)
    except url_helper.RequestException as e:
      logging.debug("ChromeDriverFinder: "
                    "Precise version download failed %s", e)
      return (version_url, None)

  def _get_ctf_milestone_data(self,
                              milestone: int) -> tuple[str, Optional[dict]]:
    latest_version_url: str = self.CFT_LATEST_URL.format(major=milestone)
    try:
      response = url_helper.get(latest_version_url)
      alternative_version = ChromeVersion.parse(response.text.strip())
      logging.debug(
          "ChromeDriverFinder: Using alternative version %s "
          "for M%s", alternative_version, milestone)
      return self._get_cft_precise_version_data(alternative_version)
    except url_helper.RequestException:
      return (self.CFT_BASE_URL, None)

  def _get_cft_driver_download_url(self, version_data,
                                   platform_name) -> Optional[str]:
    if all_downloads := version_data.get("downloads"):
      driver_downloads: dict = all_downloads.get("chromedriver", [])
      for download in driver_downloads:
        if isinstance(download, dict) and download["platform"] == platform_name:
          return download["url"]
    return None

  PRE_115_STABLE_URL: str = "http://chromedriver.storage.googleapis.com"

  def _get_pre_115_stable_url(self,
                              milestone: int) -> tuple[str, Optional[str]]:
    logging.debug(
        "ChromeDriverFinder: "
        "Looking upe old-style stable version M%s", milestone)
    assert milestone < self.CFT_MIN_MILESTONE, (
        f"Expected requested M{milestone} < M{self.CFT_MIN_MILESTONE}")
    listing_url = f"{self.PRE_115_STABLE_URL}/index.html"
    driver_version: str | None = self._get_pre_115_driver_version(milestone)
    if not driver_version:
      return listing_url, None
    if self.host_platform.is_linux:
      arch_suffix = "linux64"
    elif self.host_platform.is_macos:
      arch_suffix = "mac64"
      if self.host_platform.is_arm64:
        # The uploaded chromedriver archives changed the naming scheme after
        # chrome version 106.0.5249.21 for Arm64 (previously m1):
        #   before: chromedriver_mac64_m1.zip
        #   after:  chromedriver_mac_arm64.zip
        last_old_naming_version = (106, 0, 5249, 21)
        version_tuple = tuple(map(int, driver_version.split(".")))
        if version_tuple <= last_old_naming_version:
          arch_suffix = "mac64_m1"
        else:
          arch_suffix = "mac_arm64"
    elif self.host_platform.is_win:
      arch_suffix = "win32"
    else:
      raise DriverNotFoundError("Unsupported chromedriver platform")
    url = (f"{self.PRE_115_STABLE_URL}/{driver_version}/"
           f"chromedriver_{arch_suffix}.zip")
    return listing_url, url

  def _get_pre_115_driver_version(self, milestone) -> Optional[str]:
    if milestone < 70:
      return self._get_pre_70_driver_version(milestone)
    url = f"{self.PRE_115_STABLE_URL}/LATEST_RELEASE_{milestone}"
    try:
      response = url_helper.get(url)
      return response.text
    except url_helper.HTTPError as e:
      if e.response.status_code != 404:
        raise DriverNotFoundError(f"Could not query {url}") from e
      logging.debug("ChromeDriverFinder: Could not load latest release url %s",
                    e)
    return None

  def _get_pre_70_driver_version(self, milestone) -> Optional[str]:
    response = url_helper.get(f"{self.PRE_115_STABLE_URL}/2.46/notes.txt")
    lines: list[str] = response.text.splitlines()
    for i, line in enumerate(lines):
      if not line.startswith("---"):
        continue
      [min_version, max_version] = map(int, re.findall(r"\d+", lines[i + 1]))
      if min_version <= milestone <= max_version:
        match = re.search(r"\d\.\d+", line)
        if not match:
          raise DriverNotFoundError(f"Could not parse version number: {line}")
        return match.group(0)
    return None

  CHROMIUM_DASH_URL: str = "https://chromiumdash.appspot.com/fetch_releases"
  CHROMIUM_LISTING_URL: str = (
      "https://www.googleapis.com/storage/v1/b/chromium-browser-snapshots/o/")
  CHROMIUM_DASH_PARAMS: dict[tuple[str, str], dict] = {
      ("linux", "x64"): {
          "dash_platform": "linux",
          "dash_channel": "dev",
          "dash_limit": 10,
      },
      ("macos", "x64"): {
          "dash_platform": "mac",
      },
      ("macos", "arm64"): {
          "dash_platform": "mac",
      },
      ("win", "ia32"): {
          "dash_platform": "win",
      },
      ("win", "x64"): {
          "dash_platform": "win64",
      },
  }
  CHROMIUM_LISTING_PREFIX: dict[tuple[str, str], str] = {
      ("linux", "x64"): "Linux_x64",
      ("macos", "x64"): "Mac",
      ("macos", "arm64"): "Mac_Arm",
      ("win", "ia32"): "Win",
      ("win", "x64"): "Win_x64",
  }

  def _get_canary_url(self) -> tuple[str, Optional[str]]:
    logging.debug(
        "ChromeDriverFinder: Try downloading the chromedriver canary version")
    properties = self.CHROMIUM_DASH_PARAMS.get(self.host_platform.key)
    if not properties:
      raise DriverNotFoundError(
          f"Unsupported platform={self.platform}, key={self.host_platform.key}")
    dash_platform = properties["dash_platform"]
    dash_channel = properties.get("dash_channel", "canary")
    # Limit should be > len(canary_versions) so we also get potentially
    # the latest dev version (only beta / stable have official driver binaries).
    dash_limit = properties.get("dash_limit", 100)
    url = url_helper.update_url_query(
        self.CHROMIUM_DASH_URL, {
            "platform": dash_platform,
            "channel": dash_channel,
            "milestone": str(self.browser.version.major),
            "num": str(dash_limit),
        })
    chromium_base_position = 0
    response = url_helper.get(url)
    version_infos = list(response.json())
    if not version_infos:
      raise DriverNotFoundError("Could not find latest version info for "
                                f"platform={self.host_platform}")
    for version_info in version_infos:
      if version_info["version"] == self.browser.version.parts_str:
        chromium_base_position = int(
            version_info["chromium_main_branch_position"])
        break

    if not chromium_base_position and version_infos:
      fallback_version_info = None
      # Try matching latest milestone
      for version_info in version_infos:
        if version_info["milestone"] == self.browser.version.major:
          fallback_version_info = version_info
          break

      if not fallback_version_info:
        # Android has a slightly different release cycle than the desktop
        # versions. Assume that the latest canary version is good enough
        fallback_version_info = version_infos[0]
      chromium_base_position = int(
          fallback_version_info["chromium_main_branch_position"])
      logging.warning(
          "Falling back to latest (not precisely matching) "
          "canary chromedriver %s (expected %s)",
          fallback_version_info["version"], self.browser.version)

    if not chromium_base_position:
      raise DriverNotFoundError("Could not find matching canary chromedriver "
                                f"for {self.browser.version}")
    # Use prefixes to limit listing results and increase chances of finding
    # a matching version
    listing_prefix = self.CHROMIUM_LISTING_PREFIX.get(self.host_platform.key)
    if not listing_prefix:
      raise NotImplementedError(
          f"Unsupported chromedriver platform {self.host_platform}")
    base_prefix = str(chromium_base_position)[:4]
    listing_url: str = url_helper.update_url_query(self.CHROMIUM_LISTING_URL, {
        "prefix": f"{listing_prefix}/{base_prefix}",
        "maxResults": "10000"
    })
    listing = url_helper.get(listing_url).json()

    versions: list[tuple[int, str]] = []
    logging.debug("Filtering %s candidate URLs.", len(listing["items"]))
    for version in listing["items"]:
      if "name" not in version:
        continue
      if "mediaLink" not in version:
        continue
      name = version["name"]
      if "chromedriver" not in name:
        continue
      parts = name.split("/")
      if "chromedriver" not in parts[-1] or len(parts) < 3:
        continue
      base = parts[1]
      try:
        int(base)
      except ValueError:
        # Ignore base if it is not an int
        continue
      versions.append((int(base), version["mediaLink"]))
    versions.sort()
    logging.debug("Found candidates: %s", versions)
    logging.debug("chromium_base_position=%s", chromium_base_position)

    for i, (base, url) in enumerate(versions):
      if base > chromium_base_position:
        base, url = versions[i - 1]
        return listing_url, url
    return listing_url, None


class DriverNotFoundError(ValueError):
  pass
