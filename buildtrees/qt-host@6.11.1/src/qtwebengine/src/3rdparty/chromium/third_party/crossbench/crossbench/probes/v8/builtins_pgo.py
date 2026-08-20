# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.probes.chromium_probe import ChromiumProbe
from crossbench.probes.probe_context import ProbeContext
from crossbench.probes.results import LocalProbeResult, ProbeResult

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup


class V8BuiltinsPGOProbe(ChromiumProbe):
  """
  Chromium-only Probe to extract V8 builtins PGO data.
  The resulting data is used to optimize Torque and CSA builtins.
  """
  NAME = "v8.builtins.pgo"

  @override
  def attach(self, browser: Browser) -> None:
    super().attach(browser)
    browser.js_flags.set("--allow-natives-syntax")

  @override
  def get_context_cls(self) -> Type[V8BuiltinsPGOProbeContext]:
    return V8BuiltinsPGOProbeContext

  @override
  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    merged_result_path = group.get_local_probe_result_path(self)
    result_files = (run.results[self].file for run in group.runs)
    result_file = self.host_platform.concat_files(
        inputs=result_files, output=merged_result_path)
    return LocalProbeResult(file=(result_file,))

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    merged_result_path = group.get_local_probe_result_path(self)
    result_files = (g.results[self].file for g in group.repetitions_groups)
    result_file = self.host_platform.concat_files(
        inputs=result_files, output=merged_result_path)
    return LocalProbeResult(file=(result_file,))


class V8BuiltinsPGOProbeContext(ProbeContext[V8BuiltinsPGOProbe]):
  _pgo_counters: str | None = None

  @override
  def setup(self) -> None:
    pass

  def start(self) -> None:
    pass

  def stop(self) -> None:
    with self.run.actions("Extract Builtins PGO DATA") as actions:
      self._pgo_counters = actions.js(
          "return %GetAndResetTurboProfilingData();")

  def teardown(self) -> ProbeResult:
    assert self._pgo_counters is not None and self._pgo_counters, (
        "Chrome didn't produce any V8 builtins PGO data. "
        "Please make sure to set the v8_enable_builtins_profiling=true "
        "gn args.")
    pgo_file = self.local_result_path
    with pgo_file.open("a") as f:
      f.write(self._pgo_counters)
    return LocalProbeResult(file=(pgo_file,))
