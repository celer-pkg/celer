# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum
from typing import Any, Generic, Iterable, TypeVar


class UnexpectedStateError(RuntimeError):

  def __init__(self, state: BaseState, expected: Iterable[BaseState]) -> None:
    self._state = state
    self._expected = tuple(expected)
    names = ", ".join(tuple(s.name for s in expected))
    super().__init__(f"Unexpected state got={state.name} expected=({names})")

  @property
  def state(self) -> BaseState:
    return self._state

  @property
  def expected(self) -> tuple[BaseState, ...]:
    return self._expected


class BaseState(enum.IntEnum):
  """Base class for StateMachine states."""


@enum.unique
class State(BaseState):
  """Default state implementation."""
  INITIAL = enum.auto()
  SETUP = enum.auto()
  READY = enum.auto()
  RUN = enum.auto()
  DONE = enum.auto()


StateT = TypeVar("StateT", bound="BaseState")


class StateMachine(Generic[StateT]):

  def __init__(self, default: StateT) -> None:
    self._state: StateT = default

  @property
  def state(self) -> StateT:
    return self._state

  @property
  def name(self) -> str:
    return self._state.name

  def __eq__(self, other: Any) -> bool:
    if self is other:
      return True
    if isinstance(other, StateMachine):
      return self._state is other._state
    if isinstance(other, type(self._state)):
      return self._state is other
    return False

  def transition(self, *args: StateT, to: StateT) -> None:
    self.expect(*args)
    self._state = to

  def expect(self, *args: StateT) -> None:
    if self._state not in args:
      raise UnexpectedStateError(self._state, args)

  def expect_before(self, state: StateT) -> None:
    if self._state >= state:
      valid_states = (s for s in type(self._state) if s < state)
      raise UnexpectedStateError(self._state, valid_states)

  def expect_at_least(self, state: StateT) -> None:
    if self._state < state:
      valid_states = (s for s in type(self._state) if s >= state)
      raise UnexpectedStateError(self._state, valid_states)

  def __str__(self) -> str:
    return self._state.name
