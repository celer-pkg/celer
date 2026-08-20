# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc

from crossbench.probes.probe import Probe


class EnvModifier(Probe, metaclass=abc.ABCMeta):
  """
  A class that modifies the running environment without actually producing
  data like a Probe.

  TODO(crbug.com/374017625): Add more logic here, and maybe not inherit from
  Probe.
  """
