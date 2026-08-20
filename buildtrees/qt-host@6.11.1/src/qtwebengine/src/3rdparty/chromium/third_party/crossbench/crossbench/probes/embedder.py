# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type, cast

from typing_extensions import override

from crossbench.browsers.webview.embedder import WebviewEmbedder
from crossbench.parse import ObjectParser
from crossbench.probes.js import parse_javascript
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.metric import MetricsMerger
from crossbench.probes.probe import ProbeConfigParser, ProbeIncompatibleBrowser
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.plt.android_adb import AndroidAdbPlatform
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.types import Json


class WebviewEmbedderProbe(JsonResultProbe):
  """
  Android-only probe to collect performance data from an embedder.
  """
  NAME = "embedder"
  RESULT_LOCATION = ResultLocation.LOCAL
  IS_GENERAL_PURPOSE = True

  @classmethod
  def config_parser(cls) -> ProbeConfigParser:
    parser = super().config_parser()
    parser.add_argument(
        "js",
        type=parse_javascript,
        required=True,
        help=("Required JavaScript code that is run immediately after "
              "a story has finished. The code must return a JS object with "
              "(nested) metric values (numbers)."))
    return parser

  def __init__(self, js):
    super().__init__()
    self._metric_js = js

  @property
  def metric_js(self) -> str:
    return self._metric_js

  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    if not isinstance(browser, WebviewEmbedder):
      raise ProbeIncompatibleBrowser(self, browser,
                                     "Only supported for WV embedders")

  def get_context_cls(self) -> Type[WebviewEmbedderProbeContext]:
    return WebviewEmbedderProbeContext

  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        story_group.results[self].json
        for story_group in group.repetitions_groups)
    return self.write_group_result(group, merged)

  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self.merge_browsers_json_list(group).merge(
        self.merge_browsers_csv_list(group))


class WebviewEmbedderProbeContext(JsonResultProbeContext[WebviewEmbedderProbe]):
  @property
  @override
  def browser(self) -> WebviewEmbedder:
    browser = super().browser
    # TODO(b/412981884): use BrowserAttributes instead of instance check
    assert isinstance(browser, WebviewEmbedder), (
      "Only supported for WV embedders")
    return cast("WebviewEmbedder", browser)

  @property
  @override
  def browser_platform(self) -> AndroidAdbPlatform:
    browser_platform = super().browser_platform
    assert browser_platform.is_android, (
        f"Expected android platform, but got {browser_platform}")
    return cast("AndroidAdbPlatform", browser_platform)

  @override
  def to_json(self, actions: Actions) -> Json:
    driver = self.browser.start_driver(self.session)
    try:
      data = actions.js(self.probe.metric_js)
    finally:
      driver.quit()
    return ObjectParser.non_empty_dict(data, "JS metric data")
