# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import atexit
import logging
import os
import re
import shlex
import subprocess
import time
from typing import TYPE_CHECKING, Iterable, Optional, TextIO

from typing_extensions import override

from crossbench.helper import url_helper
from crossbench.helper.cwd import ChangeCWD
from crossbench.helper.path_finder import WprGoToolFinder
from crossbench.parse import NumberParser, PathParser
from crossbench.path import AnyPath, LocalPath
from crossbench.plt import PLATFORM, Platform

if TYPE_CHECKING:
  from crossbench.plt.types import TupleCmdArgs


_WPR_PORT_RE: re.Pattern[str] = re.compile(r".*Starting server on "
                                           r"(?P<protocol>http|https)://"
                                           r"(?P<host>[^:]+):"
                                           r"(?P<port>\d+)")


class WprStartupError(RuntimeError):
  pass


class WprBase(abc.ABC):
  NAME: str = ""

  _key_file: AnyPath
  _cert_file: AnyPath

  def __init__(self,
               archive_path: AnyPath,
               bin_path: AnyPath,
               http_port: int = 0,
               https_port: int = 0,
               host: str = "127.0.0.1",
               inject_scripts: Optional[Iterable[AnyPath]] = None,
               key_file: Optional[AnyPath] = None,
               cert_file: Optional[AnyPath] = None,
               log_path: Optional[LocalPath] = None,
               platform: Platform = PLATFORM) -> None:
    self._platform: Platform = platform
    self._process: subprocess.Popen | None = None
    self._log_path: LocalPath | None = None
    if log_path:
      self._log_path = PathParser.not_existing_path(log_path)
    self._log_file: TextIO | None = None
    self._bin_path = bin_path
    self._go_cmd: TupleCmdArgs = ()
    self._host_http_port: int = 0
    self._host_https_port: int = 0

    wpr_root: LocalPath
    if self._bin_path.suffix == ".go":
      # `go` binary is required to run a Go source file (`wpr.go`).
      assert self._platform.is_local
      if local_go := self._platform.which("go"):
        self._go_cmd = (local_go, "run", self._bin_path)
      else:
        raise ValueError(f"'go' binary not available on {self._platform}")
      wpr_root = self._platform.local_path(self._bin_path.parents[1])
    else:
      # Assuming the binary path is precompiled and executable.
      self._go_cmd = (self._bin_path,)
      if self._platform.is_local:
        if local_wpr_go := WprGoToolFinder(self._platform).local_path:
          wpr_root = local_wpr_go.parents[1]
        else:
          raise ValueError(
              f"Could not find web_page_replay_go on {self._platform}")
      else:
        assert key_file is not None
        assert cert_file is not None
        assert inject_scripts is not None

    self._archive_path = self._validate_archive_path(archive_path)
    (self._device_http_port,
     self._device_https_port) = self._validate_ports(http_port, https_port)
    self._num_parsed_ports: int = 0
    self._host: str = host
    if self._platform.is_remote:
      assert self._host == "127.0.0.1"

    if key_file:
      self._key_file = key_file
    else:
      self._key_file = wpr_root / "ecdsa_key.pem"
    if not self._platform.is_file(self._key_file):
      raise ValueError(f"Could not find ecdsa_key.pem file: {self._key_file}")

    if cert_file:
      self._cert_file = cert_file
    else:
      self._cert_file = wpr_root / "ecdsa_cert.pem"
    if not self._platform.is_file(self._cert_file):
      raise ValueError(f"Could not find ecdsa_cert.pem file: {self._cert_file}")

    if inject_scripts is None:
      inject_scripts = [wpr_root / "deterministic.js"]
    for script in inject_scripts:
      if "," in str(script):
        raise ValueError(f"Injected script path cannot contain ',': {script}")
      if not self._platform.is_file(script):
        raise ValueError(f"Injected script does not exist: {script}")
    self._inject_scripts: tuple[AnyPath, ...] = tuple(inject_scripts)

  def _validate_ports(self, http_port: int, https_port: int) -> tuple[int, int]:
    if http_port == 0:
      logging.debug("WPR: using auto-port for http")
    else:
      http_port = NumberParser.port_number(http_port, "wpr http port")
    if https_port == 0:
      logging.debug("WPR: using auto-port for https")
    else:
      https_port = NumberParser.port_number(https_port, "wpr https port")
    if http_port and http_port == https_port:
      raise ValueError("http_port must be different from https_port, "
                       f"but got twice: {http_port}")
    return (http_port, https_port)

  @abc.abstractmethod
  def _validate_archive_path(self, path: AnyPath) -> AnyPath:
    pass

  @property
  def http_port(self) -> int:
    return self._device_http_port

  @property
  def https_port(self) -> int:
    return self._device_https_port

  @property
  def host(self) -> str:
    return self._host

  @property
  def cert_file(self) -> AnyPath:
    return self._cert_file

  @property
  @abc.abstractmethod
  def cmd(self) -> TupleCmdArgs:
    pass

  @property
  def base_cmd_flags(self) -> TupleCmdArgs:
    cmd: TupleCmdArgs = (
        f"--http_port={self._device_http_port}",
        f"--https_port={self._device_https_port}",
        f"--https_key_file={self._key_file}",
        f"--https_cert_file={self._cert_file}",
    )
    if self._inject_scripts is not None:
      injected_scripts = ",".join(
          os.fspath(path) for path in self._inject_scripts)
      cmd += (f"--inject_scripts={injected_scripts}",)
    return cmd

  def start(self) -> None:
    try:
      atexit.register(self.stop, force_shutdown=True)
      self._start_wpr()
      logging.info("WPR: waiting for startup...")
      self._wait_for_startup()
      logging.info(("WPR: Started wpr.go %s: "
                    "DONE (platform=%s, http_port=%s, http_port=%s)"),
                   self.NAME, self._platform, self.http_port, self.https_port)
    except BaseException as e:
      if isinstance(e, Exception):
        logging.debug("WPR got startup errors: %s %s", type(e), e)
      force_shutdown = isinstance(e, WprStartupError)
      self.stop(force_shutdown)
      self._handle_startup_error()
      raise

  def _start_wpr(self) -> None:
    go_cmd: TupleCmdArgs = self._go_cmd + self.cmd
    logging.info("STARTING WPR on %s: %s", self._platform,
                 shlex.join(map(str, go_cmd)))
    self._num_parsed_ports = 0
    if self._log_path:
      self._log_file = self._log_path.open("w", encoding="utf-8")  # pylint: disable=consider-using-with
    work_dir: LocalPath = LocalPath.cwd()
    if self._platform.is_local:
      work_dir = self._platform.local_path(self._bin_path.parent)
    with ChangeCWD(work_dir):
      logging.debug("Logging to %s", self._log_path)
      self._process = self._platform.popen(
          *go_cmd,
          stdout=self._log_file,
          stderr=self._log_file,
          stdin=subprocess.DEVNULL)
    if not self._process:
      raise WprStartupError(f"Could not start {type(self).__name__}")

  def _handle_startup_error(self) -> None:
    logging.error("WPR: Could not start %s", type(self).__name__)
    if not self._log_path or not self._log_path.exists():
      return
    logging.error("WPR: Check log files %s", self._log_path)
    try:
      with self._log_path.open("r", encoding="utf-8") as f:
        log_lines = list(f.readlines())
        logging.error("  %s", "  ".join(log_lines[-4:]))
    except Exception as e:  # pylint: disable=broad-except
      logging.debug("Got exception while reading wpr log file: %s", e)

  def _forward_ports(self) -> None:
    assert self._process, "Should not forward ports if WPR is not running"
    if self._platform.is_remote:
      ports = self._platform.ports
      self._host_http_port = ports.forward(0, self._device_http_port)
      self._host_https_port = ports.forward(0, self._device_https_port)
    else:
      self._host_http_port = self._device_http_port
      self._host_https_port = self._device_https_port

  def _wait_for_startup(self) -> None:
    assert self._process, "process not started"
    assert self._log_path, "missing log_path"
    assert self._num_parsed_ports == 0, "WPR did not shut down correctly."
    time.sleep(1)
    with self._log_path.open("r", encoding="utf-8") as log_file:
      while self._process.poll() is None:
        line = log_file.readline()
        if not line:
          time.sleep(0.1)
          continue
        if self._parse_wpr_log_line(line):
          break
    if self._process.poll():
      raise self._startup_failure()

    self._forward_ports()
    time.sleep(0.1)
    try:
      self._open_wpr_cmd_url("generate-200")
      return
    except url_helper.HTTPError as e:
      logging.debug("Could not query wpr server: %s", e)
    raise self._startup_failure()

  def _startup_failure(self) -> WprStartupError:
    return WprStartupError("Could not start wpr.go.\n"
                           f"See log for more details: {self._log_path}")

  def _parse_wpr_log_line(self, line: str) -> bool:
    if "Failed to start server on" in line:
      logging.error(line)
      raise WprStartupError(
          f"Could not start wpr.go server, address in use: {line}")
    line = line.strip()
    if match := _WPR_PORT_RE.match(line):
      protocol = match["protocol"].lower()
      port = int(match["port"])
      if protocol == "http":
        self._device_http_port = port
        self._num_parsed_ports += 1
      elif protocol == "https":
        self._device_https_port = port
        self._num_parsed_ports += 1
      else:
        logging.error("WPR: got invalid protocol: %s", line)
      self._host = match["host"]
      if not self._host:
        raise WprStartupError(f"WPR: could not parse host from: {line}")

    if self._num_parsed_ports == 2 and (self._device_http_port and
                                        self._device_https_port):
      logging.debug("WPR: https_port=%s http_port=%s", self._device_https_port,
                    self._device_http_port)
      return True
    return False

  def _open_wpr_cmd_url(self,
                        cmd: str,
                        verbose: bool = True) -> url_helper.Response:
    test_url = (
        f"http://{self._host}:{self._host_http_port}/web-page-replay-{cmd}")
    return url_helper.get(test_url, timeout=1, verbose=verbose)

  def stop(self, force_shutdown: bool = False) -> None:
    atexit.unregister(self.stop)
    if not self._process:
      return
    try:
      if not force_shutdown:
        self._shut_down()
      if self._log_file:
        self._log_file.close()
        self._log_file = None
      if force_shutdown:
        self._platform.terminate_gracefully(self._process, timeout=1)
    finally:
      self._process = None

  def _shut_down(self) -> None:
    logging.info("WPR: shutting down %s.", self.NAME)
    try:
      self._open_wpr_cmd_url("command-exit", verbose=False)
    except url_helper.RequestException:
      # The above request always fails because WPR closes the connection
      # without response.
      pass


class WprRecorder(WprBase):
  NAME: str = "recorder"

  @property
  def cert_file(self) -> LocalPath:
    return self._platform.local_path(self._cert_file)

  @property
  @override
  def cmd(self) -> TupleCmdArgs:
    return ("record",) + super().base_cmd_flags + (str(self._archive_path),)

  @override
  def _validate_archive_path(self, path: AnyPath) -> LocalPath:
    return PathParser.not_existing_path(path, "Wpr.go result archive")

  def clear(self) -> None:
    """Start a new recording by clearing out all existing recorded requests."""
    self._open_wpr_cmd_url("command-clear")


class WprReplayServer(WprBase):
  NAME: str = "replay"

  def __init__(self,
               archive_path: AnyPath,
               bin_path: AnyPath,
               http_port: int = 0,
               https_port: int = 0,
               host: str = "127.0.0.1",
               inject_scripts: Optional[Iterable[AnyPath]] = None,
               key_file: Optional[AnyPath] = None,
               cert_file: Optional[AnyPath] = None,
               rules_file: Optional[AnyPath] = None,
               log_path: Optional[LocalPath] = None,
               fuzzy_url_matching: bool = True,
               serve_chronologically: bool = True,
               platform: Platform = PLATFORM) -> None:
    super().__init__(archive_path, bin_path, http_port, https_port, host,
                     inject_scripts, key_file, cert_file, log_path, platform)
    self._rules_file: AnyPath | None = None
    if rules_file:
      self._rules_file = PathParser.non_empty_file_path(rules_file)
    self._fuzzy_url_matching: bool = fuzzy_url_matching
    self._serve_chronologically: bool = serve_chronologically

  @override
  def _validate_archive_path(self, path: AnyPath) -> AnyPath:
    assert self._platform.is_file(path)
    return path

  @property
  @override
  def cmd(self) -> TupleCmdArgs:
    cmd = ("replay",) + super().base_cmd_flags
    if self._rules_file:
      cmd += (f"--rules_file={self._rules_file }",)
    if not self._fuzzy_url_matching:
      cmd += ("--disable_fuzzy_url_matching",)
    if self._serve_chronologically:
      cmd += ("--serve_response_in_chronological_sequence",)
    cmd += (str(self._archive_path),)
    return cmd
