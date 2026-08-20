# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
import shutil
from typing import TYPE_CHECKING, Any, Iterable, Optional, Self, Type

from immutabledict import immutabledict
from typing_extensions import override

from crossbench import plt
from crossbench.helper import fs_helper
from crossbench.helper.cwd import ChangeCWD
from crossbench.helper.path_finder import WprGoToolFinder
from crossbench.network.replay.web_page_replay import WprRecorder
from crossbench.parse import PathParser
from crossbench.probes.probe import Probe, ProbeConfigParser, ProbeContext
from crossbench.probes.results import (EmptyProbeResult, LocalProbeResult,
                                       ProbeResult, ProbeResultDict)

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.path import LocalPath
  from crossbench.plt.port_manager import PortScope
  from crossbench.runner.groups.base import RunGroup
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.groups.stories import StoriesRunGroup
  from crossbench.runner.run import Run



class WebPageReplayProbe(Probe):
  """
  Probe to collect browser requests to wpr.go archive which then can be
  replayed using a local proxy server.

  Chrome telemetry's wpr.go:
  https://chromium.googlesource.com/catapult/+/HEAD/web_page_replay_go/README.md
  """

  NAME = "wpr"

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument("http_port", type=int, default=8080, required=False)
    parser.add_argument("https_port", type=int, default=8081, required=False)
    parser.add_argument(
        "wpr_go_bin", type=plt.PLATFORM.parse_local_binary_path, required=False)
    parser.add_argument(
        "key_file", type=PathParser.existing_file_path, required=False)
    parser.add_argument(
        "cert_file", type=PathParser.existing_file_path, required=False)
    parser.add_argument(
        "inject_scripts",
        is_list=True,
        type=PathParser.existing_file_path,
        required=False)
    parser.add_argument(
        "use_test_root_certificate", type=bool, default=False, required=False)
    parser.add_argument(
        "record_setup",
        type=bool,
        default=True,
        help="Also include the requests that are part of "
        "the setup / login steps, "
        "which might include passwords.")
    return parser

  def __init__(self,
               http_port: int = 0,
               https_port: int = 0,
               wpr_go_bin: Optional[LocalPath] = None,
               inject_scripts: Optional[Iterable[LocalPath]] = None,
               key_file: Optional[LocalPath] = None,
               cert_file: Optional[LocalPath] = None,
               use_test_root_certificate: bool = False,
               record_setup: bool = True) -> None:
    super().__init__()
    host_platform = plt.PLATFORM
    if not wpr_go_bin:
      wpr_go_bin = WprGoToolFinder(host_platform).local_path
    if not wpr_go_bin:
      raise RuntimeError(f"Could not find wpr.go on {host_platform}")
    self._wpr_go_bin: LocalPath = host_platform.parse_local_binary_path(
        wpr_go_bin, "wpr.go")

    self._recorder_kwargs: immutabledict[str, Any] = immutabledict(
        bin_path=wpr_go_bin,
        http_port=http_port,
        https_port=https_port,
        inject_scripts=inject_scripts,
        key_file=key_file,
        cert_file=cert_file,
    )

    self._https_port = https_port
    self._http_port = http_port
    self._use_test_root_certificate = use_test_root_certificate
    self._record_setup = record_setup

  @property
  def https_port(self) -> int:
    return self._https_port

  @property
  def http_port(self) -> int:
    return self._http_port

  @property
  def recorder_kwargs(self) -> immutabledict:
    return self._recorder_kwargs

  @property
  def use_test_root_certificate(self) -> bool:
    return self._use_test_root_certificate

  @property
  def record_setup(self) -> bool:
    return self._record_setup

  @property
  @override
  def result_path_name(self) -> str:
    return "archive.wprgo"

  def is_compatible(self, browser: Browser) -> bool:
    return browser.attributes().is_chromium_based and browser.platform.is_local

  @override
  def get_context_cls(self) -> Type[WprRecorderProbeContext]:
    return WprRecorderProbeContext

  @override
  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    results = [run.results[self].file for run in group.runs]
    return self.merge_group(results, group)

  @override
  def merge_stories(self, group: StoriesRunGroup) -> ProbeResult:
    results = [
        subgroup.results[self].file for subgroup in group.repetitions_groups
    ]
    return self.merge_group(results, group)

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    results = [subgroup.results[self].file for subgroup in group.story_groups]
    return self.merge_group(results, group)

  def merge_group(self, results: list[LocalPath],
                  group: RunGroup) -> ProbeResult:
    result_file = group.get_local_probe_result_path(self)
    if not results:
      return EmptyProbeResult()
    first_wprgo = results.pop(0)
    # TODO migrate to platform
    shutil.copy(first_wprgo, result_file)
    for repetition_file in results:
      self.httparchive_merge(repetition_file, result_file)
    return LocalProbeResult(file=[result_file])

  def httparchive_merge(self, input_archive: LocalPath,
                        output_archive: LocalPath) -> None:
    cmd: list[str | LocalPath] = [
        "go",
        "run",
        self._wpr_go_bin.parent / "httparchive.go",
        "merge",
        output_archive,
        input_archive,
        output_archive,
    ]
    with ChangeCWD(self._wpr_go_bin.parent):
      self.host_platform.sh(*cmd)

  @override
  def log_run_result(self, run: Run) -> None:
    self._log_results(run.results)

  @override
  def log_browsers_result(self, group: BrowsersRunGroup) -> None:
    self._log_results(group.results)

  def _log_results(self, result_dict: ProbeResultDict) -> None:
    if self not in result_dict:
      return
    wpr_archive: LocalPath = result_dict[self].file
    logging.info("-" * 80)
    logging.critical("WPR archive:")
    logging.critical("  %s [%s]", wpr_archive,
                     fs_helper.get_file_size(wpr_archive))


class WprRecorderProbeContext(ProbeContext[WebPageReplayProbe]):

  def __init__(self, probe: WebPageReplayProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._wprgo_log: LocalPath = self.local_result_path.with_name(
        "wpr_record.log")
    self._host: str = "127.0.0.1"
    kwargs = dict(self.probe.recorder_kwargs)
    kwargs.update({
        "platform": run.host_platform,
        "log_path": self._wprgo_log,
        "archive_path": self.result_path,
    })
    self._recorder = WprRecorder(**kwargs)
    self._browser_platform = run.browser_platform
    self._ports: PortScope = run.host_platform.ports

  @override
  def setup(self) -> None:
    self._recorder.start()
    self._setup_extra_flags()
    self._setup_port_forwarding()

  def _setup_extra_flags(self) -> None:
    if not self.probe.use_test_root_certificate:
      cert_hash_file = self._recorder.cert_file.parent / "wpr_public_hash.txt"
      if not cert_hash_file.is_file():
        raise ValueError(
            f"Could not read public key hash file: {cert_hash_file}")
      cert_skip_list = ",".join(cert_hash_file.read_text().strip().splitlines())
      self.session.extra_flags[
          "--ignore-certificate-errors-spki-list"] = cert_skip_list
    # TODO: support ts_proxy traffic shaping
    # session.extra_flags["--proxy-server"] =  (
    #   "socks://{self._ts_proxy_host}:{self._ts_proxy_port}")
    # session.extra_flags["--proxy-bypass-list"] = "<-loopback>"
    self.session.extra_flags["--host-resolver-rules"] = (
        f"MAP *:80 {self._host}:{self._recorder.http_port},"
        f"MAP *:443 {self._host}:{self._recorder.https_port},"
        "EXCLUDE localhost")
    # TODO: add replay support, see:
    # https://crsrc.org/c/third_party/catapult/telemetry/telemetry/internal/backends/chrome/chrome_startup_args.py

  def _setup_port_forwarding(self) -> None:
    if self._browser_platform.is_remote:
      # TODO: Fix run.setup and teardown layering so they they're called with
      # the same active port scope.
      self._ports = self._browser_platform.ports
      self._ports.reverse_forward(self._recorder.http_port,
                                  self._recorder.http_port)
      self._ports.reverse_forward(self._recorder.https_port,
                                  self._recorder.https_port)

  def start(self) -> None:
    if not self.probe.record_setup:
      assert self._recorder
      self._recorder.clear()

  def stop(self) -> None:
    pass

  def teardown(self) -> ProbeResult:
    self._teardown_port_forwarding()
    self._recorder.stop()
    return LocalProbeResult(file=(self.local_result_path,))

  def _teardown_port_forwarding(self) -> None:
    if self._browser_platform.is_remote:
      self._ports.stop_reverse_forward(self._recorder.http_port)
      self._ports.stop_reverse_forward(self._recorder.https_port)
