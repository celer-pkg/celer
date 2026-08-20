# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import Self, Set, TypeVar

from crossbench.decor import base
from crossbench.runner.groups.session import BrowserSessionRunGroup
from crossbench.runner.run import Run

SessionDecoratorT = TypeVar("SessionDecoratorT", bound="SessionDecorator")


class SessionDecorator(base.Decorator[Run]):

  def runs(self) -> Set[Run]:
    return self._targets

  @abc.abstractmethod
  def get_context(self: Self, target: Run) -> SessionDecoratorContext[Self]:
    pass


class SessionDecoratorContext(base.DecoratorContext[SessionDecoratorT,
                                                    BrowserSessionRunGroup]):

  @property
  def browser_session(self) -> BrowserSessionRunGroup:
    return self._target
