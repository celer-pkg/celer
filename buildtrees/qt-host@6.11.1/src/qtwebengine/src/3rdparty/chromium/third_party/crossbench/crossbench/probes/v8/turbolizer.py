# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.helper import fs_helper
from crossbench.probes.chromium_probe import ChromiumProbe
from crossbench.probes.probe_context import ProbeContext
from crossbench.probes.result_location import ResultLocation
from crossbench.probes.results import (BrowserProbeResult, LocalProbeResult,
                                       ProbeResult)

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.path import AnyPath


class V8TurbolizerProbe(ChromiumProbe):
  """
  Chromium-only Probe for extracting detailed turbofan graphs.
  Note: This probe can have significant overhead.
  Tool: https://v8.dev/tools/head/turbolizer
  """
  NAME = "v8.turbolizer"
  RESULT_LOCATION = ResultLocation.BROWSER

  @override
  def attach(self, browser: Browser) -> None:
    super().attach(browser)
    browser.flags.set("--no-sandbox")
    browser.js_flags.set("--trace-turbo")

  @override
  def get_context_cls(self) -> Type[V8TurbolizerProbeContext]:
    return V8TurbolizerProbeContext


class V8TurbolizerProbeContext(ProbeContext[V8TurbolizerProbe]):

  @property
  def results_dir(self) -> AnyPath:
    # Put v8.turbolizer files into separate dirs in case we have
    # multiple isolates
    turbolizer_log_dir = super().result_path
    self.browser_platform.mkdir(turbolizer_log_dir, exist_ok=True)
    return turbolizer_log_dir

  @override
  def setup(self) -> None:
    js_flags = self.session.extra_js_flags
    js_flags["--trace-turbo-path"] = str(self.results_dir)
    js_flags["--trace-turbo-cfg-file"] = str(self.results_dir / "cfg.graph")

  def start(self) -> None:
    pass

  def stop(self) -> None:
    pass

  def teardown(self) -> ProbeResult:
    log_dir = self.local_result_path.parent
    # Copy the files from a potentially remote browser to a the local result
    # dir.
    result: BrowserProbeResult = self.browser_result(file=(log_dir,))
    local_log_dir = result.file
    assert local_log_dir.is_dir()
    # Sort files locally after transferring them.
    log_files = fs_helper.sort_by_file_size(local_log_dir.glob("*"))
    return LocalProbeResult(file=log_files)
