# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.probes.internal.base import InternalProbe
from crossbench.probes.probe_context import ProbeContext

if TYPE_CHECKING:
  from crossbench.probes.results import ProbeResult


class BrowserDriverLogProbe(InternalProbe):
  """
  Runner-internal: Collects the driver logs
  """
  NAME = "browser.driver.log"

  @override
  def get_context_cls(self) -> Type[BrowserDriverLogProbeContext]:
    return BrowserDriverLogProbeContext


class BrowserDriverLogProbeContext(ProbeContext[BrowserDriverLogProbe]):

  @override
  def setup(self) -> None:
    pass

  def start(self) -> None:
    pass

  def stop(self) -> None:
    pass

  def teardown(self) -> ProbeResult:
    # TODO: support remote driver log
    driver_log_file = self.browser.driver_log_file
    if not driver_log_file:
      return self.empty_result()
    # safaridriver writes the log to non-configurable system-folder from which
    # we need to copy it out.
    if driver_log_file != self.local_result_path:
      self.host_platform.copy_file(driver_log_file, self.local_result_path)
    return self.local_result(file=(self.local_result_path,))
