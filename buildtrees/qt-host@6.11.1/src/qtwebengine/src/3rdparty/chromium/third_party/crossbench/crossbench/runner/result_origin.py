# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import contextlib
import logging
from typing import TYPE_CHECKING, Iterable

from typing_extensions import override

from crossbench.decor.target_protocol import DecoratorTargetProtocol
from crossbench.probes.result_location import ResultLocation
from crossbench.runner.probe_result_origin import ProbeResultOrigin

if TYPE_CHECKING:
  from collections.abc import Generator

  from crossbench import plt
  from crossbench.browsers.browser import Browser
  from crossbench.exception import (Annotator, ExceptionAnnotationScope,
                                    TExceptionTypes)
  from crossbench.helper.durations import DurationMeasureContext, Durations
  from crossbench.path import AnyPath, LocalPath
  from crossbench.probes.probe import Probe
  from crossbench.runner.runner import Runner


class ResultOrigin(DecoratorTargetProtocol, ProbeResultOrigin, abc.ABC):
  """Base class for Run and BrowserSession, both places where
  probe results can be placed."""

  @property
  def is_local(self) -> bool:
    return self.browser_platform.is_local

  @property
  @override
  def is_remote(self) -> bool:
    return self.browser_platform.is_remote

  @property
  @abc.abstractmethod
  def browser_tmp_dir(self) -> AnyPath:
    pass

  @property
  @abc.abstractmethod
  def out_dir(self) -> LocalPath:
    pass

  @property
  @abc.abstractmethod
  def exceptions(self) -> Annotator:
    pass

  @property
  @abc.abstractmethod
  def durations(self) -> Durations:
    pass

  @property
  @abc.abstractmethod
  def browser(self) -> Browser:
    pass

  @property
  def runner(self) -> Runner:
    raise NotImplementedError(
        f"Cannot access on runner on {type(self).__name__}")

  @property
  @override
  def host_platform(self) -> plt.Platform:
    return self.browser.host_platform

  @property
  @override
  def browser_platform(self) -> plt.Platform:
    return self.browser.platform

  @property
  def probes(self) -> Iterable[Probe]:
    # TODO: migrate away from using runner
    return self.runner.probes

  @contextlib.contextmanager
  def measure(
      self, label: str
  ) -> Generator[tuple[ExceptionAnnotationScope, DurationMeasureContext], None,
                 None]:
    # Return a combined context manager that adds an named exception info
    # and measures the time during the with-scope.
    with self.exceptions.info(label) as stack, self.durations.measure(
        label) as timer:
      yield (stack, timer)

  def exception_info(self, *stack_entries: str) -> ExceptionAnnotationScope:
    return self.exceptions.info(*stack_entries)

  @override
  def exception_capture(
      self, *stack_entries: str, exceptions: TExceptionTypes = (Exception,)
  ) -> ExceptionAnnotationScope:
    return self.exceptions.capture(*stack_entries, exceptions=exceptions)

  def get_default_probe_result_path(self, probe: Probe) -> AnyPath:
    """Return a local or remote/browser-based result path depending on the
    Probe default RESULT_LOCATION."""
    if probe.RESULT_LOCATION == ResultLocation.BROWSER:
      return self.get_browser_probe_result_path(probe)
    if probe.RESULT_LOCATION == ResultLocation.LOCAL:
      return self.get_local_probe_result_path(probe)
    raise ValueError(f"Invalid probe.RESULT_LOCATION {probe.RESULT_LOCATION} "
                     f"for probe {probe}")

  @abc.abstractmethod
  def get_local_probe_result_path(self, probe: Probe) -> LocalPath:
    pass

  def get_browser_probe_result_path(self, probe: Probe) -> AnyPath:
    local_path = self.get_local_probe_result_path(probe)
    if self.is_local:
      return local_path
    # Create a temp file relative to the remote browser tmp dir.
    relative_path = local_path.relative_to(self.out_dir)
    path = self.browser_tmp_dir / relative_path
    logging.debug("Creating remote result dir=%s on platform=%s", path.parent,
                  self.browser_platform)
    self.browser_platform.mkdir(path.parent)
    return path
