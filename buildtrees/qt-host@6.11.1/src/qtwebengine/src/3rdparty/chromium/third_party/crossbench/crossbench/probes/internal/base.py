# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type, TypeVar

from typing_extensions import override

from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.probe import Probe

if TYPE_CHECKING:
  from crossbench.probes.results import ProbeResult


class InternalProbe(Probe):
  IS_GENERAL_PURPOSE = False

  @property
  @override
  def is_internal(self) -> bool:
    return True


class InternalJsonResultProbe(JsonResultProbe, InternalProbe):
  IS_GENERAL_PURPOSE = False

  @override
  def get_context_cls(self) -> Type[InternalJsonResultProbeContext]:
    return InternalJsonResultProbeContext


InternalJsonResultProbeT = TypeVar(
    "InternalJsonResultProbeT", bound="InternalJsonResultProbe")


class InternalJsonResultProbeContext(
    JsonResultProbeContext[InternalJsonResultProbeT]):
  FLATTEN = False

  @override
  def stop(self) -> None:
    # Only extract data in the late teardown phase.
    pass

  @override
  def teardown(self) -> ProbeResult:
    self._json_data = self.extract_json(self.run)  # pylint: disable=no-member
    return super().teardown()
