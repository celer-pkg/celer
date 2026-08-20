# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
from typing import TYPE_CHECKING, Optional, Self, Sequence, Type

from typing_extensions import override

from crossbench.action_runner.screenshot_annotation import (
    ScreenshotAnnotation, annotate_screenshot_svg)
from crossbench.probes.probe import Probe, ProbeConfigParser, ProbeContext
from crossbench.probes.probe_error import ProbeMissingDataError
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.browsers.browser import Viewport
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.path import AnyPath
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


class ScreenshotProbe(Probe):
  """
  General-purpose Probe that collects screenshots.
  """
  NAME = "screenshot"
  RESULT_LOCATION = ResultLocation.BROWSER
  IMAGE_FORMAT = "png"

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    # TODO: support interval-based screenshots
    return parser

  def _pre_check_viewport_size(self, env: RunnerEnv) -> None:
    for browser in env.browsers:
      viewport: Viewport = browser.viewport
      if viewport.is_headless:
        env.handle_warning(
            f"Cannot take screenshots for headless browser: {browser}")
      if viewport.x < 10 or viewport.y < 50:
        env.handle_warning(
            f"Viewport for '{browser}' might include toolbar: {viewport}")

  @override
  def get_context_cls(self) -> Type[ScreenshotProbeContext]:
    return ScreenshotProbeContext

  # TODO: implement merge_repetitions()
  # TODO: implement merge_browsers()


class ScreenshotProbeContext(ProbeContext[ScreenshotProbe]):

  def __init__(self, probe: ScreenshotProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._results: list[AnyPath] = []

  @override
  def get_default_result_path(self) -> AnyPath:
    screenshot_dir = super().get_default_result_path()
    self.browser_platform.mkdir(screenshot_dir)
    return screenshot_dir

  def start(self) -> None:
    self.screenshot("start")

  @override
  def start_story_run(self) -> None:
    self.screenshot("start_story")

  @override
  def stop_story_run(self) -> None:
    self.screenshot("stop_story")

  def stop(self) -> None:
    self.screenshot("stop")

  def _annotate_screenshot(self, screenshot_file_name: str, label: str,
                           annotations: Sequence[ScreenshotAnnotation]) -> None:
    (screen_width, screen_height) = self.browser_platform.display_resolution()
    svg = annotate_screenshot_svg(screen_width, screen_height,
                                  screenshot_file_name, annotations)
    svg_path = self.result_path / f"{label}.svg"
    self.browser_platform.write_text(svg_path, svg)
    self._results.append(svg_path)

  def screenshot(
      self,
      label: Optional[str] = None,
      annotations: Optional[Sequence[ScreenshotAnnotation]] = None) -> None:
    # TODO: support screen coordinates
    if not label:
      label = str(dt.datetime.now().strftime("%Y-%m-%d_%H%M%S"))
    file_name = f"{label}.{ScreenshotProbe.IMAGE_FORMAT}"
    path = self.result_path / file_name
    # TODO: use the browser's implementation first which might be more portable
    self.browser_platform.screenshot(path)
    self._results.append(path)

    if annotations:
      self._annotate_screenshot(file_name, label, annotations)

  @override
  def teardown(self) -> ProbeResult:
    if not self.browser_platform.is_dir(self.result_path):
      raise ProbeMissingDataError(
          f"No screen shot found at: {self.result_path}")
    return self.browser_result(file=tuple(self._results))
