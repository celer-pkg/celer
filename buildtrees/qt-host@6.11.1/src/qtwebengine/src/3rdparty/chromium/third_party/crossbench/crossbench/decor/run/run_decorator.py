# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import Self, Set, TypeVar

from crossbench.decor import base
from crossbench.runner.run import Run

RunDecoratorT = TypeVar("RunDecoratorT", bound="RunDecorator")


class RunDecorator(base.Decorator[Run]):

  def runs(self) -> Set[Run]:
    return self._targets

  @abc.abstractmethod
  def get_context(self: Self, target: Run) -> RunDecoratorContext[Self]:
    pass


class RunDecoratorContext(base.DecoratorContext[RunDecoratorT, Run]):

  @property
  def run(self) -> Run:
    return self._target
