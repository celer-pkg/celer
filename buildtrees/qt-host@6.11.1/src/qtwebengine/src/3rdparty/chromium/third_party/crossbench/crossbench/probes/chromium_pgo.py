# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import functools
import logging
from typing import TYPE_CHECKING, Final, Type, cast

from typing_extensions import override

from crossbench import path as pth
from crossbench.browsers.chromium.devtools import DevToolsClient
from crossbench.browsers.chromium.webdriver import ChromiumWebDriverAndroid
from crossbench.parse import PathParser
from crossbench.probes.chromium_probe import ChromiumProbe
from crossbench.probes.probe import ProbeConfigParser, ProbeContext
from crossbench.probes.probe_error import ProbeMissingDataError
from crossbench.probes.results import (EmptyProbeResult, LocalProbeResult,
                                       ProbeResult)

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.env import HostEnvironment
  from crossbench.runner.run import Run

DEFAULT_REMOTE_PGO_ROOT_PATH: pth.AnyPath = (
    pth.AnyPosixPath("/data_mirror/data_ce/null/0/"))


class ChromiumPgoProbe(ChromiumProbe):
  """
    Chromium-only Probe to dump PGO profiles from the target device and
    downloads them.
    The resulting data is used to optimize Chromium.
    """
  NAME = "chromium_pgo"

  _REMOTE_PGO_CACHE_SUFFIX: Final[str] = "cache/pgo_profiles"

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser:
    parser = super().config_parser()
    parser.add_argument(
        "remote_pgo_root_path",
        type=PathParser.any_path,
        default=DEFAULT_REMOTE_PGO_ROOT_PATH,
        help=("Root path for the PGO profile directories on the target device."
              "The browser package name will be appended to this path."),
    )
    return parser

  def __init__(
      self,
      remote_pgo_root_path: pth.AnyPath = DEFAULT_REMOTE_PGO_ROOT_PATH,
  ) -> None:
    super().__init__()
    self._remote_pgo_root_path = remote_pgo_root_path

  @override
  def attach(self, browser: Browser) -> None:
    super().attach(browser)
    flags = browser.flags
    flags["--remote-allow-origins"] = "*"

  @override
  def validate_browser(self, env: HostEnvironment, browser: Browser) -> None:
    super().validate_browser(env, browser)
    self.expect_android(browser)

  @property
  def remote_pgo_root_path(self) -> pth.AnyPath:
    return self._remote_pgo_root_path

  @property
  def remote_pgo_cache_suffix(self) -> str:
    return self._REMOTE_PGO_CACHE_SUFFIX

  @override
  def get_context_cls(self) -> Type[ChromiumPgoProbeContext]:
    # This probe currently only supports Android (enforced by
    # validate_browser()). If other platforms are added, inspect
    # browser.platform here to return the correct context. For now,
    # defaults to Android.
    return ChromiumPgoProbeContextAndroid


class ChromiumPgoProbeContext(ProbeContext[ChromiumPgoProbe]):
  """
  Base context for Chromium PGO (Profile Guided Optimization) probes.
  This class defines the interface for PGO operations. Platform-specific
  contexts should inherit from this class.
  """


class ChromiumPgoProbeContextAndroid(ChromiumPgoProbeContext):
  """Android-specific context for Chromium PGO probes."""
  PGO_CMD_ID: Final[int] = 0

  def __init__(self, probe: ChromiumPgoProbe, run: Run):
    super().__init__(probe, run)
    probe.expect_android(run.browser)
    self._devtools_client: DevToolsClient | None = None
    self._initial_pgo_timestamps: dict[pth.AnyPath, float] = {}

  @functools.cached_property
  def remote_pgo_dir(self) -> pth.AnyPath:
    android_browser = cast(ChromiumWebDriverAndroid, self.run.browser)
    return (self.probe.remote_pgo_root_path / android_browser.android_package /
            self.probe.remote_pgo_cache_suffix)

  def _get_devtools_client(self) -> DevToolsClient:
    if not self._devtools_client:
      self._devtools_client = DevToolsClient(
          platform=self.browser_platform,
          requested_local_port=0,
          remote_devtools_identifier="chrome_devtools_remote")
    return self._devtools_client

  def _trigger_pgo_dump(self) -> bool:
    """Triggers a PGO profile dump via DevTools."""
    request = {
        "method": "NativeProfiling.dumpProfilingDataOfAllProcesses",
        "id": self.PGO_CMD_ID
    }
    logging.debug("Triggering PGO dump.")
    devtools_client = self._get_devtools_client()
    # Ensure devtools_client is connected before sending command
    with devtools_client:
      success, _ = devtools_client.send_command(request)
      if success:
        logging.info("PGO dump triggered successfully.")
      else:
        logging.error("Failed to trigger PGO dump.")
      return success

  def _list_pgo_profiles(self) -> list[pth.AnyPath]:
    """Lists PGO profile files in the specified directory on the device."""
    if not self.browser_platform.is_dir(self.remote_pgo_dir):
      return []
    return list(self.browser_platform.iterdir(self.remote_pgo_dir))

  def _clean_pgo_profiles(self) -> None:
    """Removes the PGO profile directory from the device."""
    self.browser_platform.rm(self.remote_pgo_dir, dir=True, missing_ok=True)
    logging.debug("Cleaned PGO profiles from: %s", self.remote_pgo_dir)

  @override
  def start(self) -> None:
    pass

  @override
  def stop(self) -> None:
    self._initial_pgo_timestamps = self._get_pgo_timestamps()
    if not self._trigger_pgo_dump():
      raise RuntimeError("PGO: Failed to dump profiles during stop phase")

  def _get_pgo_timestamps(self) -> dict[pth.AnyPath, float]:
    """Returns a dictionary of PGO file paths and their modification times."""
    timestamps: dict[pth.AnyPath, float] = {}
    for f in self._list_pgo_profiles():
      timestamps[f] = self.browser_platform.last_modified(f)
    return timestamps

  def _is_pgo_dump_completed(self) -> bool:
    current_timestamps = self._get_pgo_timestamps()
    return any(
        current_timestamps.get(f) != self._initial_pgo_timestamps.get(f)
        for f in current_timestamps)

  def _wait_for_pgo_dump(self, timeout: float) -> None:
    """Waits for PGO profile files to be modified."""
    for _ in self.run.wait_range(0.2, timeout, 0).wait_with_backoff():
      if self._is_pgo_dump_completed():
        logging.info("PGO dump completed.")
        return
    raise ProbeMissingDataError(
        f"PGO dump did not complete within {timeout} seconds.")

  @override
  def teardown(self) -> ProbeResult:
    downloaded_local_pgo_files: list[pth.LocalPath] = []
    logging.debug("Remote PGO directory: %s", self.remote_pgo_dir)

    self._wait_for_pgo_dump(timeout=3)

    remote_pgo_files = self._list_pgo_profiles()
    if not remote_pgo_files:
      logging.info("PGO: No PGO profile files found in %s.",
                   self.remote_pgo_dir)
      return EmptyProbeResult()

    for remote_file_path in remote_pgo_files:
      file_name = remote_file_path.name
      local_file_path = self.local_result_path / file_name
      self.browser_platform.pull(remote_file_path, local_file_path)
      downloaded_local_pgo_files.append(local_file_path)

    # Clean profiles after successful download
    self._clean_pgo_profiles()

    if not downloaded_local_pgo_files:
      # This case might be redundant if already handled by no remote files found
      return EmptyProbeResult()

    return LocalProbeResult(file=downloaded_local_pgo_files)
