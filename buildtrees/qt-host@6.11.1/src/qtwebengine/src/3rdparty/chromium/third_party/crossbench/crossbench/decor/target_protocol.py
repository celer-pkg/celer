# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
from typing import TYPE_CHECKING, Protocol

if TYPE_CHECKING:
  from crossbench.exception import ExceptionAnnotationScope, TExceptionTypes


class DecoratorTargetProtocol(Protocol):

  @abc.abstractmethod
  def exception_capture(
      self, *stack_entries: str, exceptions: TExceptionTypes = (Exception,)
  ) -> ExceptionAnnotationScope:
    pass
