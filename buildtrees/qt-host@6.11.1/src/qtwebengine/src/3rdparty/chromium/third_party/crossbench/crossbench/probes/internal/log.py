# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.probes.internal.base import InternalProbe
from crossbench.probes.probe_context import ProbeContext

if TYPE_CHECKING:
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


class LogProbe(InternalProbe):
  """
  Runner-internal meta-probe: Collects the python logging data from the runner
  itself.
  """
  NAME = "cb.log"

  @override
  def get_context_cls(self) -> Type[LogProbeContext]:
    return LogProbeContext


class LogProbeContext(ProbeContext[LogProbe]):

  def __init__(self, probe_instance: LogProbe, run: Run) -> None:
    super().__init__(probe_instance, run)
    self._log_handler: logging.Handler | None = None

  @override
  def setup(self) -> None:
    log_formatter = logging.Formatter(
        "%(asctime)s [%(threadName)-12.12s] [%(levelname)-5.5s] "
        "[%(name)s]  %(message)s")
    self._log_handler = logging.FileHandler(self.result_path)
    self._log_handler.setFormatter(log_formatter)
    self._log_handler.setLevel(logging.DEBUG)
    logging.getLogger().addHandler(self._log_handler)

  def start(self) -> None:
    pass

  def stop(self) -> None:
    pass

  def teardown(self) -> ProbeResult:
    assert self._log_handler
    logging.getLogger().removeHandler(self._log_handler)
    self._log_handler = None
    return self.local_result(file=(self.local_result_path,))
