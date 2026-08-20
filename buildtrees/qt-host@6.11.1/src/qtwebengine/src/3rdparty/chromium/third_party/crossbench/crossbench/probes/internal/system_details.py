# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.probes.internal.base import (InternalJsonResultProbe,
                                             InternalJsonResultProbeContext)

if TYPE_CHECKING:
  from crossbench.runner.actions import Actions
  from crossbench.types import Json


class SystemDetailsProbe(InternalJsonResultProbe):
  """
  Runner-internal meta-probe: Collects the browser's system/platform details.
  """
  NAME = "cb.system.details"
  AUTO_MERGE_REPETITIONS = False

  @override
  def get_context_cls(self) -> Type[InternalJsonResultProbeContext]:
    return SystemDetailsProbeContext


class SystemDetailsProbeContext(InternalJsonResultProbeContext):

  @override
  def to_json(self, actions: Actions) -> Json:
    return self.run.browser_platform.system_details()
