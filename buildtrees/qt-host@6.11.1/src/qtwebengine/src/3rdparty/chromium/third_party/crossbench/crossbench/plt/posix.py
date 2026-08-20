# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import datetime as dt
import functools
import logging
import pathlib
import re
import shlex
from typing import (TYPE_CHECKING, Any, Generator, Iterator, Mapping, Optional,
                    Set, Type)

from typing_extensions import override

from crossbench import path as pth
from crossbench.helper.wait import WaitRange
from crossbench.plt import proc_helper
from crossbench.plt.base import Environ, Platform, SubprocessError
from crossbench.plt.remote import RemotePlatformMixin, RemotePopen
from crossbench.plt.signals import PosixBaseSignal

if TYPE_CHECKING:
  import subprocess

  from crossbench.plt.signals import AnyPosixSignals, Signals
  from crossbench.plt.types import CmdArg, ListCmdArgs, ProcessLike
  from crossbench.types import JsonDict


class PosixPlatform(Platform, metaclass=abc.ABCMeta):
  # pylint: disable=locally-disabled, redefined-builtin

  def __init__(self) -> None:
    super().__init__()
    self._default_tmp_dir: pth.AnyPath | None = None

  @property
  def signals(self) -> Type[AnyPosixSignals]:
    return PosixBaseSignal

  @functools.cached_property
  @override
  def version(self) -> str:  #pylint: disable=invalid-overridden-method
    return self.sh_stdout("uname", "-r").strip()

  @functools.lru_cache(maxsize=1)
  def _raw_machine_arch(self) -> str:
    if self.is_local:
      return super()._raw_machine_arch()
    return self.sh_stdout("uname", "-m").strip()

  @functools.cached_property
  @override
  def cpu(self) -> str:  #pylint: disable=invalid-overridden-method
    cpu_str = "UNKNOWN"
    for line in self.cat(self.path("/proc/cpuinfo")).splitlines():
      if line.startswith("model name"):
        _, cpu_str = line.split(":", maxsplit=2)
        break
    if num_cores := self.cpu_cores(logical=False):
      cpu_str = f"{cpu_str} {num_cores} cores"
    return cpu_str

  @functools.lru_cache(maxsize=2)
  @override
  def cpu_cores(self, logical: bool) -> int:
    if self.is_local:
      return super().cpu_cores(logical)
    if cores := self._parse_cpuinfo(logical):
      return cores
    if logical:
      if getconf := self.which("getconf"):
        if result := self.sh_stdout(getconf, "_NPROCESSORS_ONLN"):
          return int(result)
    logging.debug("Failed to get num CPU cores")
    return 0

  def _parse_cpuinfo(self, logical: bool) -> int:
    assert not self.is_macos, "unsupported operation on macos"
    entries = self.sh_stdout("grep", "-E", "processor|core id|physical id",
                             "/proc/cpuinfo")
    logical_cores: Set[int] = set()
    core_ids: list[int] = []
    physical_ids: list[int] = []

    for line in entries.splitlines():
      line = line.strip()
      if line:
        key, value = line.rsplit(": ", maxsplit=1)
        match key.strip():
          case "processor":
            logical_cores.add(int(value))
          case "core id":
            core_ids.append(int(value))
          case "physical id":
            physical_ids.append(int(value))

    if logical:
      return len(logical_cores)

    if core_ids:
      if len(core_ids) == len(physical_ids):
        pairs = set(zip(core_ids, physical_ids))
        return len(pairs)
      logging.debug("Invalid cpuinfo data: Cannot determine core counts.")

    # Android doesn't report core-id in cpuinfo, assuming single-threaded
    # CPUs and report physical_cores
    if self.is_android:
      return len(logical_cores)
    return 0


  @functools.lru_cache(maxsize=1)
  @override
  def cpu_details(self) -> dict[str, Any]:
    if self.is_local:
      return super().cpu_details()
    return {
        "info": self.cpu,
        "physical cores": self.cpu_cores(logical=False),
        "logical cores": self.cpu_cores(logical=True),
        "min frequency": "n/a",
        "max frequency": "n/a",
        "current frequency": "n/a",
    }

  @functools.lru_cache(maxsize=1)
  @override
  def os_details(self) -> JsonDict:
    if self.is_local:
      return super().os_details()
    return {
        "system": self.sh_stdout("uname").strip(),
        "release": self.sh_stdout("uname", "-r").strip(),
        "version": self.sh_stdout("uname", "-v").strip(),
        "platform": self.sh_stdout("uname", "-a").strip(),
    }

  _PY_VERSION: str = "import sys; print(64 if sys.maxsize > 2**32 else 32)"

  @functools.lru_cache(maxsize=1)
  @override
  def python_details(self) -> JsonDict:
    if self.is_local:
      return super().python_details()
    if python3 := self.which("python3"):
      return {
          "version": self.sh_stdout(python3, "--version").strip(),
          "bits": int(self.sh_stdout(python3, "-c", self._PY_VERSION).strip())
      }
    return {"version": "unknown", "bits": 64}

  UPTIME_RE = re.compile(r"up\s+"
                         r"(?:(?P<days>\d+)\s+days?,\s*)?"
                         r"(?:"
                         r"(?:(?P<hm_hours>\d+):(?P<hm_mins>\d+))|"
                         r"(?:(?P<mins_only>\d+)\s+min)"
                         r")")

  @override
  def uptime(self) -> dt.timedelta:
    """Parse posix uptime output into a timedelta object.
    Example Output:
    12:25  up  3:26, 2 users, load averages: 4.27 4.29 4.80
    """
    uptime_output = self.sh_stdout("uptime")
    match = self.UPTIME_RE.search(uptime_output)
    if not match:
      return dt.timedelta()

    groups = match.groupdict()
    days = int(groups.get("days") or 0)
    hours = int(groups.get("hm_hours") or 0)
    minutes_hm = int(groups.get("hm_mins") or 0)
    minutes_only = int(groups.get("mins_only") or 0)
    minutes = minutes_hm or minutes_only

    try:
      delta = dt.timedelta(days=days, hours=hours, minutes=minutes)
      return delta
    except ValueError:
      return dt.timedelta()


  @override
  def app_version(self, app_or_bin: pth.AnyPathLike) -> str:
    app_or_bin = self.path(app_or_bin)
    if not self.exists(app_or_bin):
      raise ValueError(f"Binary {app_or_bin} does not exist.")
    return self.sh_stdout(app_or_bin, "--version")

  @property
  @override
  def default_tmp_dir(self) -> pth.AnyPath:
    if self._default_tmp_dir and self._default_tmp_dir.parts:
      return self._default_tmp_dir
    if self.is_local:
      self._default_tmp_dir = self.path(super().default_tmp_dir)
      return self._default_tmp_dir
    env = self.environ

    for tmp_var in ("TMPDIR", "TEMP", "TMP"):
      if tmp_var not in env:
        continue
      tmp_path = self.path(env[tmp_var])
      if self.is_dir(tmp_path):
        self._default_tmp_dir = tmp_path
        assert self.is_absolute(self._default_tmp_dir)
        return tmp_path
    self._default_tmp_dir = self.path("/tmp")
    assert self.is_dir(self._default_tmp_dir), (
        f"Fallback tmp dir does not exist: {self._default_tmp_dir}")
    return self._default_tmp_dir

  @override
  def path(self, path: pth.AnyPathLike) -> pth.AnyPath:
    converted_path = path
    if isinstance(path, pathlib.PureWindowsPath):
      # Special-case posix-absolute WindowsPath.
      # for instance: WindowsPath("/usr/local/bin") or WindowsPath("C:/var/tmp")
      parts = path.parts
      if parts[0] in ("\\", "C:\\"):
        # Reassemble parts for an absolute posix path.
        parts = ("/", *path.parts[1:])
        converted_path = pth.AnyPosixPath(*parts)
    if self.is_local:
      return pth.LocalPosixPath(converted_path)
    return pth.AnyPosixPath(converted_path)

  @override
  def which(self, binary_name: pth.AnyPathLike) -> Optional[pth.AnyPath]:
    if self.is_local:
      return super().which(binary_name)
    if not binary_name:
      raise ValueError("Got empty path")
    if binary_override := self.lookup_binary_override(binary_name):
      return binary_override
    try:
      if maybe_path := self.sh_stdout("which", self.path(binary_name)).strip():
        maybe_bin = self.path(maybe_path)
        if self.exists(maybe_bin):
          return maybe_bin
    except SubprocessError:
      pass
    return None

  @override
  def cat(self, file: pth.AnyPathLike, encoding: str = "utf-8") -> str:
    if self.is_local:
      return super().cat(file, encoding)
    return self.sh_stdout("cat", self.path(file), encoding=encoding)

  @override
  def cat_bytes(self, file: pth.AnyPathLike) -> bytes:
    if self.is_local:
      return super().cat_bytes(file)
    return self.sh_stdout_bytes("cat", self.path(file))

  @override
  def rm(self,
         path: pth.AnyPathLike,
         dir: bool = False,
         missing_ok: bool = False) -> None:
    if self.is_local:
      super().rm(path, dir, missing_ok)
      return
    if missing_ok and not self.exists(path):
      return
    if dir:
      self.sh("rm", "-rf", self.path(path))
    else:
      self.sh("rm", self.path(path))

  @override
  def rename(self, src_path: pth.AnyPathLike,
             dst_path: pth.AnyPathLike) -> pth.AnyPath:
    if self.is_local:
      return super().rename(src_path, dst_path)
    dst_path = self.path(dst_path)
    self.sh("mv", self.path(src_path), dst_path)
    return dst_path

  @override
  def home(self) -> pth.AnyPath:
    if self.is_local:
      return super().home()
    return self.path(self.sh_stdout("printenv", "HOME").strip())

  @override
  def touch(self, path: pth.AnyPathLike) -> None:
    if self.is_local:
      super().touch(path)
    else:
      self.sh("touch", self.path(path))

  @override
  def mkdir(self,
            path: pth.AnyPathLike,
            parents: bool = True,
            exist_ok: bool = True) -> None:
    if self.is_local:
      super().mkdir(path, parents, exist_ok)
    elif parents or exist_ok:
      self.sh("mkdir", "-p", self.path(path))
    else:
      self.sh("mkdir", "-p", self.path(path))

  @override
  def mkdtemp(self,
              suffix: Optional[str] = None,
              prefix: Optional[str] = None,
              dir: Optional[pth.AnyPathLike] = None) -> pth.AnyPath:
    if self.is_local:
      return super().mkdtemp(suffix, prefix, dir)
    return self._mktemp_sh(is_dir=True, suffix=suffix, prefix=prefix, dir=dir)

  @override
  def mktemp(self,
             suffix: Optional[str] = None,
             prefix: Optional[str] = None,
             dir: Optional[pth.AnyPathLike] = None) -> pth.AnyPath:
    if self.is_local:
      return super().mktemp(suffix, prefix, dir)
    return self._mktemp_sh(is_dir=False, suffix=suffix, prefix=prefix, dir=dir)

  def _mktemp_sh(self,
                 is_dir: bool,
                 suffix: Optional[str] = None,
                 prefix: Optional[str] = None,
                 dir: Optional[pth.AnyPathLike] = None) -> pth.AnyPath:
    if not dir:
      dir = self.default_tmp_dir
    template = self.path(dir) / f"{prefix or ''}XXXXXXXXXXX{suffix or ''}"
    args: ListCmdArgs = ["mktemp"]
    if is_dir:
      args.append("-d")
    args.append(str(template))
    result = self.sh_stdout(*args)
    return self.path(result.strip())

  @override
  def copy_dir(self, from_path: pth.AnyPathLike,
               to_path: pth.AnyPathLike) -> pth.AnyPath:
    if self.is_local:
      return super().copy_dir(from_path, to_path)
    from_path = self.path(from_path)
    to_path = self.path(to_path)
    if not self.exists(from_path):
      raise ValueError(f"Cannot copy non-existing source path: {from_path}")
    if from_path != to_path:
      self.mkdir(to_path.parent, parents=True, exist_ok=True)
      self.sh("cp", "-R", from_path, to_path)
    return to_path

  @override
  def copy_file(self, from_path: pth.AnyPathLike,
                to_path: pth.AnyPathLike) -> pth.AnyPath:
    if self.is_local:
      return super().copy_file(from_path, to_path)
    from_path = self.path(from_path)
    to_path = self.path(to_path)
    if not self.exists(from_path):
      raise ValueError(f"Cannot copy non-existing source path: {from_path}")
    if from_path != to_path:
      self.mkdir(to_path.parent, parents=True, exist_ok=True)
      self.sh("cp", from_path, to_path)
    return to_path

  @override
  def write_text(self,
                 file: pth.AnyPathLike,
                 data: str,
                 encoding: str = "utf-8") -> None:
    if self.is_local:
      super().write_text(file, data, encoding)
      return
    # TODO: implement stdin bypass for small content
    dest_file = self.path(file)
    with self.host_platform.NamedTemporaryFile("push.data") as tmp_file:
      self.host_platform.write_text(tmp_file, data, encoding=encoding)
      self.push(tmp_file, dest_file)

  @override
  def write_bytes(self, file: pth.AnyPathLike, data: bytes) -> None:
    if self.is_local:
      super().write_bytes(file, data)
      return
    # TODO: implement stdin bypass for small content
    dest_file = self.path(file)
    with self.host_platform.NamedTemporaryFile("push.data") as tmp_file:
      self.host_platform.write_bytes(tmp_file, data)
      self.push(tmp_file, dest_file)

  @override
  def exists(self, path: pth.AnyPathLike) -> bool:
    if self.is_local:
      return super().exists(path)
    return self.sh("[", "-e", self.path(path), "]", check=False).returncode == 0

  @override
  def is_file(self, path: pth.AnyPathLike) -> bool:
    if self.is_local:
      return super().is_file(path)
    return self.sh("[", "-f", self.path(path), "]", check=False).returncode == 0

  @override
  def is_dir(self, path: pth.AnyPathLike) -> bool:
    if self.is_local:
      return super().is_dir(path)
    return self.sh("[", "-d", self.path(path), "]", check=False).returncode == 0

  @override
  def iterdir(self,
              path: pth.AnyPathLike) -> Generator[pth.AnyPath, None, None]:
    if self.is_local:
      yield from super().iterdir(path)
      return

    remote_path = self.path(path)
    if not self.is_dir(remote_path):
      raise NotADirectoryError(f"Not a directory: {remote_path}")

    for name in self.sh_stdout("ls", "-1",
                               remote_path).rstrip("\n").splitlines():
      yield remote_path / name

  @override
  def chmod(self, path: pth.AnyPathLike, mode: int) -> None:
    if self.is_local:
      super().chmod(path, mode)
    else:
      # strip the prefix
      oct_mode = oct(mode)[2:]
      self.sh("chmod", oct_mode, self.path(path))

  @override
  def send_signal(self, process: ProcessLike, signal: Signals) -> None:
    if self.is_local:
      super().send_signal(process, signal)
      return
    if pid := self.process_pid(process):
      kill_process = self.sh(
          "kill", f"-{int(signal)}", str(pid), check=False, capture_output=True)
      # wait for the process to finish.
      if kill_process.returncode > 0:
        error_str = kill_process.stdout.decode("utf-8")
        error_str += kill_process.stderr.decode("utf-8")
        raise ProcessLookupError(f"{self}: {error_str}")

  @override
  def terminate(self, process: ProcessLike) -> None:
    if self.is_local:
      super().terminate(process)
    else:
      try:
        self.send_signal(process, self.signals.SIGTERM)
      except proc_helper.PROCESS_NOT_FOUND_EXCEPTIONS:
        pass

  @override
  def kill(self, process: ProcessLike) -> None:
    if self.is_local:
      super().kill(process)
    else:
      try:
        self.send_signal(process, self.signals.SIGKILL)
      except proc_helper.PROCESS_NOT_FOUND_EXCEPTIONS:
        pass

  @override
  def process_info(self, process: ProcessLike) -> Optional[dict[str, Any]]:
    if self.is_local:
      return super().process_info(process)
    try:
      pid = self.process_pid(process)
      lines = self.sh_stdout("ps", "-o", "comm", "-p", str(pid)).splitlines()
      if len(lines) <= 1:
        return None
      assert len(lines) == 2, lines
      tokens = lines[1].split()
      assert len(tokens) == 1
      return {"comm": tokens[0]}
    except SubprocessError:
      return None

  @property
  @override
  def environ(self) -> Environ:
    if self.is_local:
      return super().environ
    return RemotePosixEnviron(self)

  @override
  def is_port_used(self, port: int) -> bool:
    return bool(self.sh_stdout("ss", "-HOlnt", "sport", "=", f"{port}"))

  def user_id(self) -> int:
    if self.is_local:
      return super().user_id()
    return int(self.sh_stdout("id", "-u").strip())

  @override
  def last_modified(self, path: pth.AnyPathLike) -> float:
    if self.is_local:
      return super().last_modified(path)
    # Get seconds since epoch
    return float(self.sh_stdout("stat", "-c", "%Y", self.path(path)))


class RemotePosixEnviron(Environ):

  def __init__(self, platform: PosixPlatform) -> None:
    self._platform = platform
    self._environ = {}
    for line in self._platform.sh_stdout("env").splitlines():
      parts = line.split("=", maxsplit=1)
      if len(parts) == 2:
        key, value = parts
        self._environ[key] = value
      else:
        assert len(parts) == 1
        key = parts[0]
        self._environ[key] = ""

  def __getitem__(self, key: str) -> str:
    return self._environ.__getitem__(key)

  def __setitem__(self, key: str, item: str) -> None:
    raise NotImplementedError("Unsupported")

  def __delitem__(self, key: str) -> None:
    raise NotImplementedError("Unsupported")

  def __iter__(self) -> Iterator[str]:
    return self._environ.__iter__()

  def __len__(self) -> int:
    return self._environ.__len__()


class RemotePosixPlatform(RemotePlatformMixin, PosixPlatform):

  @override
  def popen(self,
            *args: CmdArg,
            bufsize: int = -1,
            shell: bool = False,
            stdout=None,
            stderr=None,
            stdin=None,
            env: Optional[Mapping[str, str]] = None,
            quiet: bool = False) -> subprocess.Popen:
    del shell
    assert not (self.is_android and env), "ADB does not support env vars"

    with self.NamedTemporaryFile("popen_pid_") as temp_pid_file:
      shell_cmd = shlex.join(map(str, args))
      # Capture the PID and wait on the process to finish.
      # Ideally this would use mkfifo but that's not readily available on
      # Android.
      shell_cmd += f" & PID=$! && echo $PID >{temp_pid_file} && wait $PID"
      if not quiet:
        logging.debug("REMOTE SHELL: %s", shell_cmd)

      host_platform_cmd = self.build_shell_cmd(shell_cmd, shell=True)

      remote_popen = RemotePopen(
          self, host_platform_cmd, bufsize=bufsize, stdout=stdout,
          stderr=stderr, stdin=stdin)
      # tmp_pid_file might not have been immediately flushed:
      for _ in WaitRange(0.01, timeout=2).wait_with_backoff():
        if pid_str := self.cat(temp_pid_file):
          remote_pid = int(pid_str)
          remote_popen.set_remote_pid(remote_pid)
          return remote_popen
      raise RuntimeError("Could not read remote PID")
