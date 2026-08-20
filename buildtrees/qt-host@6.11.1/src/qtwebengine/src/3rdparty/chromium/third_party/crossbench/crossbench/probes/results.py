# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import logging
from typing import TYPE_CHECKING, Any, Iterable, Optional, cast

from immutabledict import immutabledict
from ordered_set import OrderedSet
from typing_extensions import override

from crossbench import path as pth
from crossbench.parse import ObjectParser
from crossbench.probes.helper import INTERNAL_NAME_PREFIX

if TYPE_CHECKING:
  from crossbench.probes.probe_result_key import ProbeResultKey
  from crossbench.runner.probe_result_origin import ProbeResultOrigin
  from crossbench.types import JsonDict


class DuplicateProbeResult(ValueError):
  pass


class ProbeResult(abc.ABC):
  """
  Collection of result files for a given Probe. These can be URLs or any file.

  We distinguish between two types of files, files that can be fed to Perfetto
  TraceProcessor (trace) and any other file (file). Trace files will be fed to
  the trace_processor probe if present.
  """

  def __init__(self,
               url: Optional[Iterable[str]] = None,
               file: Optional[Iterable[pth.LocalPath]] = None,
               trace: Optional[Iterable[pth.LocalPath]] = None,
               **kwargs: Iterable[pth.LocalPath]) -> None:
    self._url_list: tuple[str, ...] = ()
    if url:
      self._url_list = ObjectParser.unique_sequence(
          tuple(url), "urls", DuplicateProbeResult)
    self._trace_list: tuple[pth.LocalPath, ...] = ()
    if trace:
      self._trace_list = ObjectParser.unique_sequence(
          tuple(trace), "traces", DuplicateProbeResult)
    tmp_files: dict[str, OrderedSet[pth.LocalPath]] = {}
    if file:
      self._extend(tmp_files, file, suffix=None, allow_duplicates=False)
    for suffix, files in kwargs.items():
      self._extend(tmp_files, files, suffix=suffix, allow_duplicates=False)

    # Do last and allow duplicated
    self._extend(
        tmp_files, self._trace_list, suffix=None, allow_duplicates=True)
    self._files: immutabledict[str, tuple[pth.LocalPath, ...]] = immutabledict({
        suffix: tuple(files) for suffix, files in tmp_files.items()
    })
    # TODO: Add Metric object for keeping metrics in-memory instead of reloading
    # them from serialized JSON files for merging.
    self._values = None
    self._validate()

  def _append(self,
              tmp_files: dict[str, OrderedSet[pth.LocalPath]],
              file: pth.LocalPath,
              suffix: Optional[str] = None,
              allow_duplicates: bool = False) -> None:
    file_suffix_name = file.suffix[1:]
    if not suffix:
      suffix = file_suffix_name
    elif file_suffix_name != suffix:
      raise ValueError(
          f"Expected '.{suffix}' suffix, but got {repr(file.suffix)} "
          f"for {file}")
    if files_with_suffix := tmp_files.get(suffix):
      if file not in files_with_suffix:
        files_with_suffix.add(file)
      elif not allow_duplicates:
        raise DuplicateProbeResult(
            f"Cannot append file twice to ProbeResult: {file}")
    else:
      tmp_files[suffix] = OrderedSet((file,))

  def _extend(self,
              tmp_files: dict[str, OrderedSet[pth.LocalPath]],
              files: Iterable[pth.LocalPath],
              suffix: Optional[str] = None,
              allow_duplicates: bool = False) -> None:
    for file in files:
      self._append(
          tmp_files, file, suffix=suffix, allow_duplicates=allow_duplicates)

  def get(self, suffix: str) -> pth.LocalPath:
    if files_with_suffix := self._files.get(suffix):
      if len(files_with_suffix) != 1:
        raise ValueError(f"Expected exactly one file with suffix {suffix}, "
                         f"but got {files_with_suffix}")
      return files_with_suffix[0]
    choices: str = f"Options are {tuple(self._files.keys())}."
    if self.is_empty:
      choices = "Empty ProbeResult."
    raise ValueError(f"No files with suffix '.{suffix}'. {choices}")

  def get_all(self, suffix: str) -> list[pth.LocalPath]:
    if files_with_suffix := self._files.get(suffix):
      return list(files_with_suffix)
    return []

  @property
  def is_empty(self) -> bool:
    return not self._url_list and not self._files

  @property
  def is_remote(self) -> bool:
    return False

  def __bool__(self) -> bool:
    return not self.is_empty

  def __eq__(self, other: Any) -> bool:
    if not isinstance(other, ProbeResult):
      return False
    if self is other:
      return True
    if self._files != other._files:
      return False
    return self._url_list == other._url_list

  def merge(self, other: ProbeResult) -> ProbeResult:
    if self.is_empty:
      return other
    if other.is_empty:
      return self
    return LocalProbeResult(
        url=self.url_list + other.url_list,
        file=self.file_list + other.file_list,
        trace=self.trace_list + other.trace_list)

  def _validate(self) -> None:
    for path in self.all_files():
      if not path.exists():
        raise ValueError(f"ProbeResult path does not exist: {path}")

  def to_json(self) -> JsonDict:
    result: JsonDict = {}
    if self._url_list:
      result["url"] = self._url_list
    for suffix, files in self._files.items():
      result[suffix] = list(map(str, files))
    return result

  @property
  def has_files(self) -> bool:
    return bool(self._files)

  def all_files(self) -> Iterable[pth.LocalPath]:
    for files in self._files.values():
      yield from files

  @property
  def url(self) -> str:
    if len(self._url_list) != 1:
      raise ValueError("ProbeResult has multiple URLs.")
    return self._url_list[0]

  @property
  def url_list(self) -> list[str]:
    return list(self._url_list)

  @property
  def file(self) -> pth.LocalPath:
    if sum(len(files) for files in self._files.values()) > 1:
      raise ValueError("ProbeResult has more than one file.")
    for files in self._files.values():
      return files[0]
    raise ValueError("ProbeResult has no files.")

  @property
  def file_list(self) -> list[pth.LocalPath]:
    return list(self.all_files())

  @property
  def trace(self) -> pth.LocalPath:
    if len(self._trace_list) != 1:
      raise ValueError("ProbeResult has multiple traces.")
    return self._trace_list[0]

  @property
  def trace_list(self) -> list[pth.LocalPath]:
    return list(self._trace_list)

  @property
  def json(self) -> pth.LocalPath:
    return self.get("json")

  @property
  def json_list(self) -> list[pth.LocalPath]:
    return self.get_all("json")

  @property
  def csv(self) -> pth.LocalPath:
    return self.get("csv")

  @property
  def csv_list(self) -> list[pth.LocalPath]:
    return self.get_all("csv")


class EmptyProbeResult(ProbeResult):

  def __init__(self) -> None:
    super().__init__()

  def __bool__(self) -> bool:
    return False


class LocalProbeResult(ProbeResult):
  """LocalProbeResult can be used for files that are always available on the
  runner/local machine."""


class BrowserProbeResult(ProbeResult):
  """BrowserProbeResult are stored on the device where the browser runs.
  Result files will be automatically transferred to the local run's results
  folder.
  """

  def __init__(self,
               result_origin: ProbeResultOrigin,
               url: Optional[Iterable[str]] = None,
               file: Optional[Iterable[pth.AnyPath]] = None,
               **kwargs: Iterable[pth.AnyPath]) -> None:
    self._browser_file = file
    local_file: Iterable[pth.LocalPath] | None = None
    local_kwargs: dict[str, Iterable[pth.LocalPath]] = {}
    self._is_remote = result_origin.is_remote
    if self._is_remote:
      if file:
        local_file = self._copy_files(result_origin, file)
      for suffix_name, files in kwargs.items():
        local_kwargs[suffix_name] = self._copy_files(result_origin, files)
    else:
      # Keep local files as is.
      local_file = cast(Iterable[pth.LocalPath], file)
      local_kwargs = cast(dict[str, Iterable[pth.LocalPath]], kwargs)

    super().__init__(url, local_file, **local_kwargs)

  @property
  @override
  def is_remote(self) -> bool:
    return self._is_remote

  def _copy_files(self, result_origin: ProbeResultOrigin,
                  paths: Iterable[pth.AnyPath]) -> Iterable[pth.LocalPath]:
    assert paths, "Got no remote paths to copy."
    # Copy result files from remote tmp dir to local results dir
    browser_platform = result_origin.browser_platform
    remote_tmp_dir = result_origin.browser_tmp_dir
    out_dir = result_origin.out_dir
    local_result_paths: list[pth.LocalPath] = []
    for remote_path in paths:
      try:
        relative_path = remote_path.relative_to(remote_tmp_dir)
      except ValueError:
        logging.debug(
            "Browser result is not in browser tmp dir: "
            "only using the name of '%s'", remote_path)
        relative_path = result_origin.host_platform.local_path(remote_path.name)
      local_result_path = out_dir / relative_path
      browser_platform.pull(remote_path, local_result_path)
      assert local_result_path.exists(), "Failed to copy result file."
      local_result_paths.append(local_result_path)
    return local_result_paths


class ProbeResultDict:
  """
  Maps Probes to their result files Paths.
  """

  def __init__(self, path: pth.AnyPath) -> None:
    self._path = path
    self._dict: dict[str, ProbeResult] = {}

  def __setitem__(self, probe: ProbeResultKey, result: ProbeResult) -> None:
    assert isinstance(result, ProbeResult)
    self._dict[probe.name] = result

  def __getitem__(self, probe: ProbeResultKey) -> ProbeResult:
    name = probe.name
    if name not in self._dict:
      raise KeyError(f"No results for probe='{name}'")
    return self._dict[name]

  def __contains__(self, probe: ProbeResultKey) -> bool:
    return probe.name in self._dict

  def __bool__(self) -> bool:
    return bool(self._dict)

  def __len__(self) -> int:
    return len(self._dict)

  def get(self, probe: ProbeResultKey, default: Any = None) -> ProbeResult:
    return self._dict.get(probe.name, default)

  def get_by_name(self, name: str, default: Any = None) -> ProbeResult:
    # Debug helper only.
    # Use bracket `results[probe]` or `results.get(probe)` instead.
    return self._dict.get(name, default)

  def to_json(self) -> JsonDict:
    data: JsonDict = {}
    for probe_name, results in self._dict.items():
      if isinstance(results, (pth.AnyPath, str)):
        data[probe_name] = str(results)
      elif results.is_empty:
        if not probe_name.startswith(INTERNAL_NAME_PREFIX):
          logging.debug("probe=%s did not produce any data.", probe_name)
        data[probe_name] = None
      else:
        data[probe_name] = results.to_json()
    return data

  def all_traces(self) -> Iterable[pth.LocalPath]:
    for probe_result in self._dict.values():
      yield from probe_result.trace_list
