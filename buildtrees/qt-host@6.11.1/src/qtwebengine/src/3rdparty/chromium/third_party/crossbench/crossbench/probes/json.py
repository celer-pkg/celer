# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import csv
import json
import logging
import re
from collections import defaultdict
from typing import (TYPE_CHECKING, Any, Callable, Generic, Optional, Type,
                    TypeVar)

import xlsxwriter
from tabulate import tabulate
from typing_extensions import override
from xlsxwriter.utility import xl_rowcol_to_cell

from crossbench.probes import helper
from crossbench.probes.metric import (CSVFormatter, MetricsMerger,
                                      metric_geomean)
from crossbench.probes.probe import Probe
from crossbench.probes.probe_context import ProbeContext
from crossbench.probes.probe_error import ProbeMissingDataError
from crossbench.probes.results import LocalProbeResult, ProbeResult

if TYPE_CHECKING:
  from crossbench import path as pth
  from crossbench.path import LocalPath
  from crossbench.runner.actions import Actions
  from crossbench.runner.groups.base import RunGroup
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.run import Run
  from crossbench.types import Json

IS_NUMERIC_RE = re.compile(r"[0-9.e+\-]+")

class JsonResultProbe(Probe, metaclass=abc.ABCMeta):
  """
  Abstract Probe that stores a Json result extracted by the `to_json` method

  Tje `to_json` is provided by subclasses. A typical examples includes just
  running a JS script on the page.
  Multiple Json result files for RepetitionsRunGroups are merged with the
  MetricsMerger. Custom merging for other RunGroups can be defined in the
  subclass.
  """

  SORT_KEYS = True
  AUTO_MERGE_REPETITIONS = True

  @property
  @override
  def result_path_name(self) -> str:
    return f"{self.name}.json"

  @override
  def merge_repetitions(
      self,
      group: RepetitionsRunGroup,
  ) -> ProbeResult:
    if not self.AUTO_MERGE_REPETITIONS:
      return super().merge_repetitions(group)
    merger = MetricsMerger()
    has_empty_results = False
    for run in group.runs:
      if self not in run.results:
        raise ProbeMissingDataError(
            f"Probe {self.NAME} produced no data to merge.")
      if run.results[self].is_empty:
        has_empty_results = True
        continue
      source_file = run.results[self].json
      assert source_file.is_file(), (
          f"{source_file} from {run} is not a file or doesn't exist.")
      with source_file.open(encoding="utf-8") as f:
        merger.add(json.load(f))
    if has_empty_results:
      logging.error("Probe %s produced empty results for some runs.", self.NAME)
    return self.write_group_result(group, merger, csv_formatter=CSVFormatter)

  def merge_browsers_json_list(self, group: BrowsersRunGroup) -> ProbeResult:
    merged_json: dict[str, dict[str, Any]] = {}
    for story_group in group.story_groups:
      browser_result: dict[str, Any] = {}
      merged_json[story_group.browser.unique_name] = browser_result
      browser_result["info"] = story_group.info
      browser_json_path = story_group.results[self].json
      assert browser_json_path.is_file(), (
          f"{browser_json_path} from {story_group} "
          "is not a file or doesn't exist.")
      with browser_json_path.open(encoding="utf-8") as f:
        browser_result["data"] = json.load(f)
    merged_json_path = group.get_local_probe_result_path(self)
    assert not merged_json_path.exists(), (
        f"Cannot override existing Json result: {merged_json_path}")
    with merged_json_path.open("w", encoding="utf-8") as f:
      json.dump(merged_json, f, indent=2)
      # TODO(375390958): figure out why files aren't fully written to
      # pyfakefs here.
      f.write("\n")
    return LocalProbeResult(json=(merged_json_path,))

  def merge_browsers_csv_list(self, group: BrowsersRunGroup) -> ProbeResult:
    csv_file_list: list[LocalPath] = []
    for story_group in group.story_groups:
      csv_file_list.append(story_group.results[self].csv)
    merged_table = helper.merge_csv(csv_file_list, row_header_len=-1)
    merged_json_path = group.get_local_probe_result_path(self, exists_ok=True)
    merged_csv_path = merged_json_path.with_suffix(".csv")
    assert not merged_csv_path.exists(), (
        f"Cannot override existing CSV result: {merged_csv_path}")
    with merged_csv_path.open("w", newline="", encoding="utf-8") as f:
      csv.writer(f, delimiter="\t").writerows(merged_table)

    merged_xlsx_path = merged_json_path.with_suffix(".xlsx")
    XLSXWriter.write(merged_table, merged_xlsx_path)
    return LocalProbeResult(csv=(merged_csv_path,), xlsx=(merged_xlsx_path,))

  def write_group_result(
      self,
      group: RunGroup,
      merged_data: dict | list | MetricsMerger,
      csv_formatter: Optional[Type[CSVFormatter]] = CSVFormatter,
      value_fn: Callable[[Any], Any] = metric_geomean) -> ProbeResult:
    merged_json_path = group.get_local_probe_result_path(self)
    with merged_json_path.open("w", encoding="utf-8") as f:
      if isinstance(merged_data, (dict, list)):
        json.dump(merged_data, f, indent=2)
      else:
        json.dump(merged_data.to_json(sort=self.SORT_KEYS), f, indent=2)
      # TODO(375390958): figure out why files aren't fully written to
      # pyfakefs here.
      f.write("\n")
    if not csv_formatter:
      return LocalProbeResult(json=(merged_json_path,))
    if not isinstance(merged_data, MetricsMerger):
      raise ValueError("write_csv is only supported for MetricsMerger, "
                       f"but found {type(merged_data)}'.")
    return self.write_group_csv_result(group, merged_data, merged_json_path,
                                       csv_formatter, value_fn)

  def write_group_csv_result(self, group: RunGroup, merged_data: MetricsMerger,
                             merged_json_path: LocalPath,
                             csv_formatter: Type[CSVFormatter],
                             value_fn: Callable[[Any], Any]) -> ProbeResult:
    merged_csv_path = merged_json_path.with_suffix(".csv")
    assert not merged_csv_path.exists(), (
        f"Cannot override existing CSV result: {merged_csv_path}")
    # Create a CSV table:
    # 0 | info label 0,                                          info_value 0
    #     ...                                                    ...
    # N | info label N,                                          info_value N
    # 0 | metric 0 full path, metric path[0] ... metric path[N], metric 0 value
    #     ...                                                    ...
    # M | metric M full path, ...                                metric M value
    headers: list[tuple[str, Any]] = []
    for label, info_value in group.info.items():
      headers.append((label, info_value))
    csv_data = csv_formatter(
        merged_data, value_fn, headers=headers, sort=self.SORT_KEYS).table
    with merged_csv_path.open("w", newline="", encoding="utf-8") as f:
      writer = csv.writer(f, delimiter="\t")
      writer.writerows(csv_data)
    return LocalProbeResult(json=(merged_json_path,), csv=(merged_csv_path,))

  LOG_SUMMARY_KEYS = ("label", "browser", "version", "os", "device", "cpu",
                      "runs", "failed runs")

  def _log_result_metrics(self, data: dict) -> None:
    table: dict[str, list[str]] = defaultdict(list)
    for browser_result in data.values():
      for info_key in self.LOG_SUMMARY_KEYS:
        table[info_key].append(browser_result["info"][info_key])
      data = browser_result["data"]
      self._extract_result_metrics_table(data, table)
    flattened: list[list[str]] = list(
        [label] + values for label, values in table.items())
    logging.critical(tabulate(flattened, tablefmt="plain"))

  def _extract_result_metrics_table(self, metrics: dict[str, Any],
                                    table: dict[str, list[str]]) -> None:
    """Add individual metrics to the table in here.
    Typically you only add score and total values for each benchmark or
    benchmark item."""
    del metrics
    del table


class XLSXWriter:

  @classmethod
  def write(cls, table: list[list[str]], path: pth.LocalPath) -> None:
    instance = cls(table, path)
    instance.write_xlsx()

  def __init__(self, table: list[list[str]], path: pth.LocalPath):
    self._table: list[list[str]] = table
    self._nof_header_cols: int = self._detect_header_cols()
    self._nof_header_rows: int = self._detect_header_rows()

    self._path = path
    self._workbook = xlsxwriter.Workbook(self._path)
    self._worksheet = self._workbook.add_worksheet()
    self._percent_format = self._workbook.add_format(
        {"num_format": "+0.0%;-0.0%;0.0%"})
    self._num_format = self._workbook.add_format({"num_format": "0.000"})

  def _detect_header_cols(self) -> int:
    nof_header_cols: int = 0
    header_row: list[str] = self._table[0]
    for col, header in enumerate(header_row[1:]):
      nof_header_cols = col + 1
      if header:
        break
    return nof_header_cols

  def _detect_header_rows(self) -> int:
    nof_header_rows: int = 0
    for row, row_data in enumerate(self._table):
      first_data: str = row_data[self._nof_header_cols]
      if not IS_NUMERIC_RE.fullmatch(first_data):
        nof_header_rows = row + 1
    # Ugly hack to account for "runs" and "failed runs":
    nof_header_rows += 2
    return nof_header_rows

  def write_xlsx(self) -> None:
    self._write_rows()
    self._close_xlsx()

  def _write_rows(self) -> None:
    for row_index, row_data in enumerate(self._table):
      self._write_row(row_index, row_data)

  def _write_row(self, row_index: int, row_data: list[str]) -> None:
    is_header_row: bool = row_index < self._nof_header_rows
    src_first_data_col_index: int = self._write_header_cols(row_index, row_data)
    dst_col_index: int = src_first_data_col_index
    # Percent diffs are computed against the first cell.
    base_cell: str = xl_rowcol_to_cell(row_index, dst_col_index)
    for src_col_index in range(src_first_data_col_index, len(row_data)):
      value = row_data[src_col_index]
      if is_header_row:
        self._worksheet.write(row_index, dst_col_index, value)
      else:
        self._worksheet.write_number(row_index, dst_col_index, float(value),
                                     self._num_format)

      current_cell: str = xl_rowcol_to_cell(row_index, dst_col_index)
      dst_col_index += 1
      # Only write diff after the first data column.
      if base_cell == current_cell:
        continue
      # Skip over diff formula for the header row (== keep empty cell).
      if not is_header_row:
        diff_formula = f"=(({current_cell}/{base_cell})-1)"
        self._worksheet.write_formula(row_index, dst_col_index, diff_formula,
                                      self._percent_format)
      dst_col_index += 1

  def _write_header_cols(self, row_index, row_data) -> int:
    for col in range(self._nof_header_cols):
      self._worksheet.write(row_index, col, row_data[col])
    return self._nof_header_cols

  def _close_xlsx(self):
    self._worksheet.freeze_panes(self._nof_header_rows, self._nof_header_cols)
    self._worksheet.set_default_row(hide_unused_rows=True)
    self._workbook.close()


JsonResultProbeT = TypeVar("JsonResultProbeT", bound="JsonResultProbe")


class JsonResultProbeContext(
    ProbeContext[JsonResultProbeT],
    Generic[JsonResultProbeT],
    metaclass=abc.ABCMeta):

  FLATTEN: bool = True

  def __init__(self, probe: JsonResultProbeT, run: Run) -> None:
    super().__init__(probe, run)
    self._json_data: Json = None

  @property
  def probe(self) -> JsonResultProbeT:
    return super().probe

  @abc.abstractmethod
  def to_json(self, actions: Actions) -> Json:
    """
    Override in subclasses.
    Returns json-serializable data.
    """
    return None

  def start(self) -> None:
    pass

  def stop(self) -> None:
    self._json_data = self.extract_json(self.run)

  def teardown(self) -> ProbeResult:
    if self._json_data is None:
      return self.empty_result()
    self._json_data = self.process_json_data(self._json_data)
    return self.write_json(self.run, self._json_data)

  def extract_json(self, run: Run) -> Json:
    with run.actions(f"Extracting Probe({self.probe.name})") as actions:
      json_data = self.to_json(actions)
      assert json_data is not None, (
          f"Probe({self.probe.name}) produced no data")
      return json_data

  def write_json(self, run: Run, json_data: Json) -> ProbeResult:
    flattened_file = None
    with run.actions(f"Writing Probe({self.probe.name})"):
      assert json_data is not None, (
          f"Probe({self.probe.name}) produced no Json data.")
      raw_file = self.local_result_path
      if self.FLATTEN:
        raw_file = raw_file.with_suffix(".json.nested")
        flattened_file = self.local_result_path
        flat_json_data = self.flatten_json_data(json_data)
        with flattened_file.open("w", encoding="utf-8") as f:
          json.dump(flat_json_data, f, indent=2)
          # TODO(375390958): figure out why files aren't fully written to
          # pyfakefs here.
          f.write("\n")
      with raw_file.open("w", encoding="utf-8") as f:
        json.dump(json_data, f, indent=2)
        # TODO(375390958): figure out why files aren't fully written to
        # pyfakefs here.
        f.write("\n")
    if flattened_file:
      return LocalProbeResult(json=(flattened_file,), file=(raw_file,))
    return LocalProbeResult(json=(raw_file,))

  def process_json_data(self, json_data: Json) -> Json:
    return json_data

  def flatten_json_data(self, json_data: Any) -> Json:
    return helper.Flatten(json_data).data
