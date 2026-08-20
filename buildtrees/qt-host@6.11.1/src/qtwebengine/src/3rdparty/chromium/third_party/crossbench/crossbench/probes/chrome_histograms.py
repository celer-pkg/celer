# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import argparse
import dataclasses
import datetime as dt
import functools
import logging
import re
from typing import TYPE_CHECKING, Any, Mapping, Optional, Self, Sequence, Type

from typing_extensions import override

from crossbench.action_runner.action.enums import ReadyState
from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.webview.embedder import WebviewEmbedder
from crossbench.parse import ObjectParser
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.metric import MetricsMerger
from crossbench.probes.result_location import ResultLocation

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.probes.probe import ProbeConfigParser
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run
  from crossbench.types import Json


class ChromeHistogramMetric(abc.ABC):
  """
  Stores enough information to log a single metric from a diff between two UMA
  histograms.
  """

  def __init__(self, name: str, histogram_name: str) -> None:
    super().__init__()
    self._name = name
    self._histogram_name = histogram_name

  @property
  def name(self) -> str:
    return self._name

  @property
  def histogram_name(self) -> str:
    return self._histogram_name

  @abc.abstractmethod
  def compute(self, delta: ChromeHistogramSample,
              baseline: ChromeHistogramSample) -> float:
    pass


class ChromeHistogramCountMetric(ChromeHistogramMetric):

  def __init__(self, histogram_name: str) -> None:
    super().__init__(f"{histogram_name}_count", histogram_name)

  @override
  def compute(self, delta: ChromeHistogramSample,
              baseline: ChromeHistogramSample) -> float:
    return delta.diff_count(baseline)


class ChromeHistogramMeanMetric(ChromeHistogramMetric):

  def __init__(self, histogram_name: str) -> None:
    super().__init__(f"{histogram_name}_mean", histogram_name)

  @override
  def compute(self, delta: ChromeHistogramSample,
              baseline: ChromeHistogramSample) -> float:
    return delta.diff_mean(baseline)


class ChromeHistogramPercentileMetric(ChromeHistogramMetric):

  def __init__(self, histogram_name: str, percentile: int) -> None:
    super().__init__(f"{histogram_name}_p{percentile}", histogram_name)
    self._percentile = percentile

  @override
  def compute(self, delta: ChromeHistogramSample,
              baseline: ChromeHistogramSample) -> float:
    return delta.diff_percentile(baseline, self._percentile)


PERCENTILE_METRIC_RE: re.Pattern[str] = re.compile(r"^p(\d+)$")


def parse_histogram_metrics(value: Any,
                            name: str = "value"
                           ) -> Sequence[ChromeHistogramMetric]:
  result: list[ChromeHistogramMetric] = []
  d = ObjectParser.dict(value, name)
  for k, v in d.items():
    histogram_name = ObjectParser.any_str(k, f"{name} name")
    metrics = ObjectParser.non_empty_sequence(
        v, f"{name} {histogram_name} metrics")
    for x in metrics:
      metric = ObjectParser.any_str(x)
      if metric == "count":
        result.append(ChromeHistogramCountMetric(histogram_name))
        continue
      if metric == "mean":
        result.append(ChromeHistogramMeanMetric(histogram_name))
        continue
      m = re.match(PERCENTILE_METRIC_RE, metric)
      if not m:
        raise argparse.ArgumentTypeError(
            f"{name} {repr(histogram_name)} "
            f"{repr(metric)} is not a valid metric")
      percentile = int(m[1])
      if percentile < 0 or percentile > 100:
        raise argparse.ArgumentTypeError(
            f"{name} {repr(histogram_name)} "
            f"{repr(metric)} is not a valid percentile")
      result.append(ChromeHistogramPercentileMetric(histogram_name, percentile))
  return result


class ChromeHistogramsProbe(JsonResultProbe):
  """
  Probe that collects UMA histogram metrics from Chrome.
  """
  NAME = "chrome_histograms"
  RESULT_LOCATION = ResultLocation.LOCAL

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "metrics",
        required=True,
        type=parse_histogram_metrics,
        help=("Required dictionary of Chrome UMA histogram metric names. "
              "Histograms are recorded before and after a test and any "
              "differences logged. "
              "See tools/metrics/histograms/metadata/storage/histograms.xml "
              "or chrome://histograms for a list of available histograms."))
    parser.add_argument(
        "use_baseline",
        aliases=("baseline",),
        type=bool,
        default=True,
        help="Dump histograms at start to use as baseline")
    return parser

  def __init__(self,
               metrics: Sequence[ChromeHistogramMetric],
               use_baseline: bool = True) -> None:
    super().__init__()
    self._metrics = metrics
    self._use_baseline = use_baseline

  @property
  def metrics(self) -> Sequence[ChromeHistogramMetric]:
    return self._metrics

  @property
  def use_baseline(self) -> bool:
    return self._use_baseline

  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    self.expect_browser(browser, BrowserAttributes.CHROMIUM_BASED)

  def get_context_cls(self) -> Type[ChromeHistogramsProbeContext]:
    return ChromeHistogramsProbeContext

  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged = MetricsMerger.merge_json_list(
        story_group.results[self].json
        for story_group in group.repetitions_groups)
    return self.write_group_result(group, merged)

  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    return self.merge_browsers_json_list(group).merge(
        self.merge_browsers_csv_list(group))


@dataclasses.dataclass
class ChromeHistogramBucket:
  min: int
  max: int | None
  count: int


ChromeHistogramBuckets = list[ChromeHistogramBucket]


class ChromeHistogramSample:
  """
  Stores the contents of one UMA histogram and provides helpers to generate
  metrics based on the difference between two samples.
  """

  # Generated by https://source.chromium.org/chromium/chromium/src/+/main:base/metrics/sample_vector.cc;l=520;drc=de573334f8fa97f9a7c99577611302736d2490b6
  # Example histogram body lines (with whitespace shortened):
  # "1326111536  -------------------O                              (19 = 63.3%)"
  # "114   ---O                                              (3 = 3.1%) {92.7%}"
  # "12  ... "
  # "1000..."
  _BUCKET_RE = re.compile(
      r"^(-?\d+) *(?:(?:-*O "  # Bucket min and ASCII bar
      r"+\((\d+) = \d+\.\d%\)(?: \{\d+\.\d%\}"  # Count and optional sum %
      r")?)|(?:\.\.\. ))$"  # Or a "..." line
  )

  # Generated by https://source.chromium.org/chromium/chromium/src/+/main:base/metrics/sample_vector.cc;l=538;drc=de573334f8fa97f9a7c99577611302736d2490b6
  # Example histogram header lines:
  # "Histogram: UKM.InitSequence recorded 1 samples, mean = 1.0 (flags = 0x41)"
  # "Histogram: WebUI.CreatedForUrl recorded 30 samples (flags = 0x41)"
  _HEADER_RE = re.compile(r"^Histogram: +.* recorded (\d+) samples"
                          r"(?:, mean = (-?\d+\.\d+))?"
                          r"(?: \(flags = (0x[0-9A-Fa-f]+)\))?$")

  @classmethod
  def from_json(cls, histogram_dict: Mapping[str,
                                             Any]) -> ChromeHistogramSample:
    name = ObjectParser.any_str(histogram_dict["name"], "histogram name")
    header = ObjectParser.any_str(histogram_dict["header"], "histogram header")
    body = ObjectParser.any_str(histogram_dict["body"], "histogram body")

    m = re.match(cls._HEADER_RE, header)
    if not m:
      raise argparse.ArgumentTypeError(
          f"{name} histogram header has invalid data: {header}")
    count = int(m.group(1))
    mean = float(m.group(2)) if m.group(2) is not None else None
    flags = int(m.group(3), 16) if m.group(3) is not None else 0

    bucket_counts: dict[int, int] = {}
    bucket_maxes: dict[int, int] = {}
    prev_min: int | None = None
    for i, line in enumerate(body.splitlines(), start=1):
      m = re.match(cls._BUCKET_RE, line)
      if not m:
        raise argparse.ArgumentTypeError(
            f"{name} histogram body line {i} has invalid data: {line}")

      bucket_min = int(m.group(1))

      # Previous bucket's max is this bucket's min.
      if prev_min is not None:
        bucket_maxes[prev_min] = bucket_min
      prev_min = bucket_min

      if bucket_count_str := m.group(2):
        bucket_count = int(bucket_count_str)
        if bucket_count > 0:
          bucket_counts[bucket_min] = bucket_count
    return ChromeHistogramSample(name, count, mean, flags, bucket_counts,
                                 bucket_maxes)

  def __init__(self,
               name: str,
               count: int = 0,
               mean: Optional[float] = 0,
               flags: int = 0,
               bucket_counts: Optional[dict[int, int]] = None,
               bucket_maxes: Optional[dict[int, int]] = None) -> None:
    self._name = name
    self._count = count
    self._mean = mean
    self._flags = flags
    self._bucket_counts = bucket_counts or {}
    self._bucket_maxes = bucket_maxes or {}
    bucket_sum = sum(self._bucket_counts.values())
    if count != bucket_sum:
      raise ValueError(f"Histogram {name} has {count} total samples, "
                       f"but buckets add to {bucket_sum}")

  @property
  def mean(self) -> Optional[float]:
    return self._mean

  @property
  def count(self) -> int:
    return self._count

  @property
  def flags(self) -> int:
    return self._flags

  def bucket_max(self, bucket_min: int) -> Optional[int]:
    return self._bucket_maxes.get(bucket_min)

  def bucket_count(self, bucket_min: int) -> int:
    return self._bucket_counts.get(bucket_min, 0)

  def diff_buckets(self,
                   baseline: ChromeHistogramSample) -> ChromeHistogramBuckets:
    buckets: ChromeHistogramBuckets = []
    for bucket_min, bucket_count in self._bucket_counts.items():
      bucket_count = bucket_count - baseline.bucket_count(bucket_min)
      bucket_max: int | None = self._bucket_maxes.get(bucket_min)
      buckets.append(
          ChromeHistogramBucket(bucket_min, bucket_max, bucket_count))
    return buckets

  def diff_percentile(self, baseline: ChromeHistogramSample,
                      percentile: int) -> float:
    if percentile < 0 or percentile > 100:
      raise ValueError(f"{percentile} is not a valid percentile")
    buckets = self.diff_buckets(baseline)
    count = functools.reduce(lambda s, b: b.count + s, buckets, 0)
    if count == 0:
      raise ValueError(
          f"{self._name} can not compute percentile without any samples")
    target = count * percentile / 100
    for bucket in buckets:
      if target <= bucket.count:
        if bucket.max is None:
          return bucket.min
        # Assume all samples are evenly distributed within the bucket.
        # NB: 0 <= target <= bucket_count
        t = target / (bucket.count + 1)
        return bucket.min * (1 - t) + bucket.max * t
      target -= bucket.count
    raise ValueError("overflowed histogram buckets looking for percentile")

  def diff_mean(self, baseline: ChromeHistogramSample) -> float:
    count = self._count - baseline.count
    if count <= 0:
      raise ValueError(f"{self._name} can not compute mean without any samples")
    if self._mean is None or baseline.mean is None:
      raise ValueError(
          f"{self._name} has no mean reported, is it an enum histogram?")

    return (self._mean * self._count - baseline.mean * baseline.count) / count

  def diff_count(self, baseline: ChromeHistogramSample) -> int:
    return self._count - baseline.count

  @property
  def name(self) -> str:
    return self._name


class ChromeHistogramsProbeContext(JsonResultProbeContext[ChromeHistogramsProbe]
                                  ):

  # JS code that overrides the chrome.send response handler and requests
  # histograms.
  HISTOGRAM_SEND = """
function webUIResponse(id, isSuccess, response) {
  if (id === "crossbench_histograms_1") {
    window.crossbench_histograms = response;
  }
}
window.cr.webUIResponse = webUIResponse;
chrome.send("requestHistograms", ["crossbench_histograms_1", "", true]);
"""

  # JS code that checks if there is a histogram response.
  HISTOGRAM_WAIT = "return !!window.crossbench_histograms"

  # JS code that returns the histograms response.
  HISTOGRAM_DATA = "return window.crossbench_histograms"

  def __init__(self, probe: ChromeHistogramsProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._baseline: dict[str, ChromeHistogramSample] | None = None
    self._delta: dict[str, ChromeHistogramSample] | None = None

  def dump_histograms(self, name: str) -> dict[str, ChromeHistogramSample]:
    with self.run.actions(
        f"Probe({self.probe.name}) dump histograms {name}") as actions:
      actions.show_url(
          "chrome://histograms",
          ready_state=ReadyState.COMPLETE,
          timeout=dt.timedelta(seconds=10))
      actions.js(self.HISTOGRAM_SEND)
      actions.wait_js_condition(self.HISTOGRAM_WAIT, 0.1, timeout=10.0)
      data = actions.js(self.HISTOGRAM_DATA)
      histogram_list = ObjectParser.sequence(data)
      histograms: dict[str, ChromeHistogramSample] = {}
      for histogram_dict in histogram_list:
        histogram = ChromeHistogramSample.from_json(
            ObjectParser.dict(histogram_dict))
        histograms[histogram.name] = histogram
      logging.debug(
        "Extracted histograms:\n%s",
        str.join("\n", histograms.keys()))
      return histograms

  def start(self) -> None:
    if self.probe.use_baseline:
      self._baseline = self.dump_histograms("start")
    else:
      self._baseline = {}
    super().start()

  def stop(self) -> None:
    if isinstance(self.browser, WebviewEmbedder):
      embedder_driver = self.browser.start_driver(self.session)
    else:
      embedder_driver = None
    try:
      self._delta = self.dump_histograms("stop")
    finally:
      if embedder_driver:
        embedder_driver.quit()
    super().stop()

  @override
  def to_json(self, actions: Actions) -> Json:
    del actions
    assert self._baseline is not None, "Probe was not started"
    if self.probe.use_baseline:
      assert self._baseline, "Did not extract start histograms"
    assert self._delta, "Did not extract end histograms"
    json = {}
    for metric in self.probe.metrics:
      baseline = self._baseline.get(
          metric.histogram_name, ChromeHistogramSample(metric.histogram_name))
      delta = self._delta.get(metric.histogram_name,
                              ChromeHistogramSample(metric.histogram_name))
      try:
        json[metric.name] = metric.compute(delta, baseline)
      except Exception as e:  # pylint: disable=broad-exception-caught
        logging.warning("Failed to log metric %s: %s", metric.name, e)
    return json
