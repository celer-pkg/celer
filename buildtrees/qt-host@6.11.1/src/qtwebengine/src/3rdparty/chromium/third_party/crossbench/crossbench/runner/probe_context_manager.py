# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import contextlib
import datetime as dt
import logging
from typing import TYPE_CHECKING, Generic, Iterable, Optional, Type, TypeVar

from crossbench.helper.state import State, StateMachine
from crossbench.probes.probe_context import BaseProbeContext, ProbeContext
from crossbench.probes.results import EmptyProbeResult, ProbeResult
from crossbench.runner.result_origin import ResultOrigin

if TYPE_CHECKING:
  from crossbench.probes.probe import Probe, ProbeT
  from crossbench.probes.results import ProbeResultDict

ResultOriginT = TypeVar("ResultOriginT", bound=ResultOrigin)
ProbeContextT = TypeVar("ProbeContextT", bound=BaseProbeContext)


class ProbeContextManager(Generic[ResultOriginT, ProbeContextT], abc.ABC):

  def __init__(self, result_origin: ResultOriginT,
               probe_results: ProbeResultDict) -> None:
    self._state = StateMachine(State.INITIAL)
    self._origin = result_origin
    self._probe_results = probe_results
    self._probe_contexts: dict[Type[Probe], ProbeContextT] = {}
    # Contains all probe context where the setup succeeded.
    self._setup_probe_contexts: list[ProbeContextT] = []
    self._failed_probe_contexts: list[ProbeContextT] = []
    # TODO: either prefix timers or use custom duration
    self._durations = result_origin.durations
    self._exceptions = result_origin.exceptions

  @property
  def is_ready(self) -> bool:
    return self._state == State.READY

  @property
  def is_running(self) -> bool:
    return self._state == State.RUN

  @property
  def is_success(self) -> bool:
    return self._exceptions.is_success

  def _measure(self, name: str):
    return self._origin.measure(name)

  @contextlib.contextmanager
  def _capture(self, label: str, measure: bool = False):
    with self._exceptions.capture(label):
      if not measure:
        yield
      else:
        with self._origin.durations.measure(label):
          yield

  def setup(self, probes: Iterable[Probe], is_dry_run: bool) -> None:
    self._state.transition(State.INITIAL, to=State.SETUP)
    if not is_dry_run:
      self._setup_probes(tuple(probes))
    self._state.transition(State.SETUP, to=State.READY)

  def _setup_probes(self, probes: tuple[Probe, ...]) -> bool:
    # We always have internal probes
    assert probes, "No probes provided"
    with self._capture("probes-setup", measure=True):
      self._validate_probes(probes)
      self._create_contexts(probes)
      self._setup_contexts()
    return self.is_success

  def _validate_probes(self, probes: tuple[Probe, ...]) -> None:
    assert not self._probe_contexts, "Wrong probe context initialization order"
    probe_set = set()
    for probe in probes:
      assert probe not in probe_set, (f"Got duplicate probe name={probe.name}")
      probe_set.add(probe)
      assert probe.is_attached, (
          f"Probe {probe.name} is not properly attached to a browser")

  def _create_contexts(self, probes: tuple[Probe, ...]) -> None:
    unique_contexts = set()
    for probe in probes:
      if probe.PRODUCES_DATA:
        self._probe_results[probe] = EmptyProbeResult()
      with self._capture(f"{probe.name} get_context"):
        if probe_context := self.get_probe_context(probe):
          assert probe_context not in unique_contexts
          unique_contexts.add(probe_context)
          probe_cls = type(probe)
          assert probe_cls not in self._probe_contexts
          self._probe_contexts[probe_cls] = probe_context

  def _setup_contexts(self) -> None:
    for probe_context in self._probe_contexts.values():
      with self._capture(f"probes-setup {probe_context.name}"):
        try:
          probe_context.setup()
          self._setup_probe_contexts.append(probe_context)
        except:
          self._failed_probe_contexts.append(probe_context)
          raise

  @contextlib.contextmanager
  def open(self, is_dry_run: bool):
    self._state.transition(State.READY, to=State.RUN)
    probe_start_time = dt.datetime.now()
    combined_contexts = contextlib.ExitStack()

    for probe_context in self._setup_probe_contexts:
      probe_context.set_start_time(probe_start_time)
      if not is_dry_run:
        combined_contexts.enter_context(probe_context.open())

    with combined_contexts:
      self._durations["probes-start"] = dt.datetime.now() - probe_start_time
      yield self

  def teardown(self, is_dry_run: bool) -> None:
    self._state.transition(State.READY, State.RUN, to=State.DONE)
    with self._measure("probes-teardown"):
      self._teardown(self._setup_probe_contexts, is_dry_run)
      self._probe_contexts = {}
      self._setup_probe_contexts = []
      self._failed_probe_contexts = []

  def _teardown(self,
                probe_contexts: list[ProbeContextT],
                is_dry_run: bool,
                setup_error: bool = False) -> None:
    if setup_error:
      assert self._probe_contexts, "Invalid state"
    self._state.expect(State.DONE)
    logging.debug("PROBE SCOPE TEARDOWN")
    if is_dry_run:
      return
    for probe_context in reversed(probe_contexts):
      with self._capture(f"Probe {probe_context.name} teardown", measure=True):
        assert probe_context.result_origin == self._origin
        probe_results: ProbeResult = probe_context.teardown()
        probe = probe_context.probe
        if probe_results.is_empty:
          self._warn_empty_probe_result(probe)
        self._probe_results[probe] = probe_results

  def _warn_empty_probe_result(self, probe: Probe) -> None:
    if not probe.PRODUCES_DATA:
      return
    if probe.is_internal:
      return
    logging.warning("Probe did not extract any data. probe=%s in %s", probe,
                    self._origin)

  @abc.abstractmethod
  def get_probe_context(self, probe: Probe) -> Optional[ProbeContextT]:
    pass

  def find_probe_context(
      self, probe_cls: Type[ProbeT]) -> Optional[ProbeContext[ProbeT]]:
    if probe_context := self._probe_contexts.get(probe_cls):
      assert isinstance(probe_context.probe, probe_cls), (
          f"Expected instance of {probe_cls}: got {probe_context.probe}")
      return probe_context  # type: ignore
    return None
