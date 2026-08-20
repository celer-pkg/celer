# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import contextlib
import logging
import sys
import traceback as tb
from dataclasses import dataclass
from typing import TYPE_CHECKING, Any, Optional, Type

from crossbench.helper import collection_helper, txt_helper

if TYPE_CHECKING:
  from types import TracebackType

  from crossbench.types import JsonList


TInfoStack = tuple[str, ...]

TExceptionTypes = tuple[Type[BaseException], ...]


@dataclass
class Entry:
  traceback: list[str]
  exception: BaseException
  info_stack: TInfoStack


class MultiException(ValueError):
  """Default exception thrown by ExceptionAnnotator.assert_success.
  It holds on to the ExceptionAnnotator and its previously captured exceptions
  are automatically added to active ExceptionAnnotator in an
  ExceptionAnnotationScope."""

  def __init__(self, message: str, exceptions: ExceptionAnnotator) -> None:
    super().__init__(message)
    self.exceptions: ExceptionAnnotator = exceptions

  def __len__(self) -> int:
    return len(self.exceptions)

  def matching(self, *args: Type[BaseException]) -> list[BaseException]:
    return self.exceptions.matching(*args)

  @property
  def annotator(self) -> ExceptionAnnotator:
    return self.exceptions


class ExceptionAnnotationScope:
  """Used in a with-scope to annotate exceptions with a TInfoStack.

  Used via the capture/annotate/info helper methods on
  ExceptionAnnotator.
  """

  def __init__(
      self,
      annotator: ExceptionAnnotator,
      exception_types: TExceptionTypes,
      ignore_exception_types: TExceptionTypes,
      entries: tuple[str, ...],
      throw_cls: Optional[Type[BaseException]] = None,
  ) -> None:
    logging.debug("EAS: %s%s", "  " * annotator.depth, " ".join(entries))
    self._annotator = annotator
    self._exception_types = exception_types
    self._ignore_exception_types = ignore_exception_types + (
        StopIteration, GeneratorExit, StopAsyncIteration)
    self._ignore_exception_types = ignore_exception_types
    self._added_info_stack_entries = entries
    self._throw_cls: Type[BaseException] | None = throw_cls
    self._previous_info_stack: TInfoStack = ()

  def __enter__(self) -> ExceptionAnnotationScope:
    self._previous_info_stack = self._annotator.enter(
        self._added_info_stack_entries)
    return self

  def __exit__(self, exception_type: Optional[Type[BaseException]],
               exception_value: Optional[BaseException],
               traceback: Optional[TracebackType]) -> bool:
    if not exception_value or not exception_type:
      self._annotator.leave(self._previous_info_stack)
      # False => exception not handled
      return False
    if issubclass(exception_type, self._ignore_exception_types) and (
        not issubclass(exception_type, MultiException)):
      self._annotator.leave(self._previous_info_stack)
      # False => exception not handled, directly forward
      return False
    logging.debug("Intermediate Exception: %s:%s", exception_type,
                  exception_value)
    if self._exception_types and exception_type and (
        issubclass(exception_type, MultiException) or
        issubclass(exception_type, self._exception_types)):
      # Handle matching exceptions directly here and prevent further
      # exception handling by returning True.
      self._annotator.append(exception_value)
      self._annotator.leave(self._previous_info_stack)
      if self._throw_cls:
        self._annotator.assert_success(exception_cls=self._throw_cls)
      return True
    if exception_value not in self._annotator._pending_exceptions:
      self._annotator._pending_exceptions[
          exception_value] = self._annotator.info_stack
    self._annotator._info_stack = self._previous_info_stack
    # False => exception not handled
    return False

class ExceptionAnnotator:
  """Collects exceptions with full backtraces and user-provided info stacks.

  Additional stack information is constructed from active
  ExceptionAnnotationScopes.
  """

  def __init__(self,
               throw: bool = False,
               throw_cls: Optional[Type[BaseException]] = None) -> None:
    self._exceptions: list[Entry] = []
    self.throw: bool = throw
    self._throw_cls: Type[BaseException] | None = throw_cls
    # The info_stack adds additional meta information to handle exceptions.
    # Unlike the source-based backtrace, this can contain dynamic information
    # for easier debugging.
    self._info_stack: TInfoStack = ()
    # Associates raised exception with the info_stack at that time for later
    # use in the `handle` method.
    # This is cleared whenever we enter a  new ExceptionAnnotationScope.
    self._pending_exceptions: dict[BaseException, TInfoStack] = {}
    self._depth = 0

  @property
  def is_success(self) -> bool:
    return len(self._exceptions) == 0

  @property
  def info_stack(self) -> TInfoStack:
    return self._info_stack

  @property
  def exceptions(self) -> list[Entry]:
    return self._exceptions

  @property
  def depth(self) -> int:
    return self._depth

  def __getitem__(self, key: Any) -> Entry:
    if not isinstance(key, int):
      raise TypeError(f"Expected int key, but got: {key}")
    return self._exceptions[key]

  def __len__(self) -> int:
    return len(self._exceptions)

  def enter(self, added_info_stack_entries: tuple[str, ...]) -> tuple[str, ...]:
    self._depth += 1
    self._pending_exceptions.clear()
    previous_stack = self._info_stack
    self._info_stack = previous_stack + added_info_stack_entries
    return previous_stack

  def leave(self, previous_stack: tuple[str, ...]) -> None:
    self._depth -= 1
    self._info_stack = previous_stack

  def matching(self, *args: Type[BaseException]) -> list[BaseException]:
    result = []
    for entry in self._exceptions:
      exception = entry.exception
      if isinstance(exception, *args):
        result.append(exception)
    return result

  def assert_success(
      self,
      message: Optional[str] = None,
      exception_cls: Type[BaseException] = MultiException,
  ) -> None:
    if self.is_success:
      return
    if message is None:
      message = "{}"
    message = message.format(self)
    if issubclass(exception_cls, MultiException):
      exception = exception_cls(message, self)
      raise exception
    raise exception_cls(message)

  def info(self, *stack_entries: str) -> ExceptionAnnotationScope:
    """Only sets info stack entries, exceptions are passed-through."""
    return ExceptionAnnotationScope(self, tuple(), tuple(), stack_entries)

  def capture(
      self,
      *stack_entries: str,
      exceptions: TExceptionTypes = (Exception,),
      ignore: TExceptionTypes = tuple(),
  ) -> ExceptionAnnotationScope:
    """Sets info stack entries and captures exceptions.
    - Does not rethrow captured exceptions
    - Does not directly throw a MultiExceptions, unless assert_success()
      is called. """
    return ExceptionAnnotationScope(self, exceptions, ignore, stack_entries,
                                    self._throw_cls)

  @contextlib.contextmanager
  def annotate(self,
               *stack_entries,
               exceptions: TExceptionTypes = (Exception,),
               ignore: TExceptionTypes = tuple()):
    """Sets info stack entries and rethrows an annotated
      MultiException by default ."""
    with self.capture(*stack_entries, exceptions=exceptions, ignore=ignore):
      yield self
    self.assert_success()

  def extend(self, annotator: ExceptionAnnotator,
             is_nested: bool = False) -> None:
    if is_nested:
      self._extend_with_prepended_stack_info(annotator)
    else:
      self._exceptions.extend(annotator.exceptions)

  def _extend_with_prepended_stack_info(self,
                                        annotator: ExceptionAnnotator) -> None:
    if annotator == self:
      return
    for entry in annotator.exceptions:
      merged_info_stack = self.info_stack + entry.info_stack
      merged_entry = Entry(entry.traceback, entry.exception, merged_info_stack)
      self._exceptions.append(merged_entry)

  def append(self, exception: BaseException) -> None:
    traceback_str = tb.format_exc()
    logging.debug("Intermediate Exception %s:%s", type(exception), exception)
    logging.debug(traceback_str)
    traceback: list[str] = traceback_str.splitlines()
    if isinstance(exception, KeyboardInterrupt):
      # Fast exit on KeyboardInterrupts for a better user experience.
      sys.exit(0)
    if isinstance(exception, MultiException):
      # Directly add exceptions from nested annotators.
      self.extend(exception.exceptions, is_nested=True)
    else:
      stack = self.info_stack
      if exception in self._pending_exceptions:
        stack = self._pending_exceptions[exception]
      self._exceptions.append(Entry(traceback, exception, stack))
    if self.throw:
      raise  # pylint: disable=misplaced-bare-raise

  def log(self, message: str, separator: str = "=") -> None:
    if self.is_success:
      return
    logging.error(separator * 80)
    if len(self._exceptions) == 1:
      logging.error("%s:", message)
    else:
      logging.error("%s (1/%d):", message, len(self._exceptions))
    logging.error(separator * 80)
    for entry in self._exceptions:
      logging.debug(entry.exception)
      logging.debug("\n".join(entry.traceback))
      logging.debug("-" * 80)
    is_first_entry = True
    grouped_entries: dict[TInfoStack, list[Entry]] = collection_helper.group_by(
        self._exceptions, key=lambda entry: entry.info_stack, sort_key=None)
    for info_stack, entries in grouped_entries.items():
      logging_level = logging.ERROR if is_first_entry else logging.DEBUG
      is_first_entry = False
      if info_stack:
        info = "Info: "
        joiner = "\n" + (" " * (len(info) - 2)) + "> "
        message = f"{info}{joiner.join(info_stack)}"
        logging.log(logging_level, message)
      for entry in entries:
        logging.log(logging_level, "- " * 40)
        logging.log(logging_level, "Type: %s:",
                    txt_helper.type_name(type(entry.exception)))
        logging.log(logging_level, "      %s", self.format_exception(entry))
        logging_level = logging.DEBUG
      logging.log(logging_level, "-" * 80)

  def error_messages(self) -> list[str]:
    return [self.format_exception(entry) for entry in self._exceptions]

  def to_json(self) -> JsonList:
    return [{
        "info_stack": entry.info_stack,
        "type": txt_helper.type_name(type(entry.exception)),
        "title": self.format_exception(entry),
        "trace": entry.traceback
    } for entry in self._exceptions]

  def format_exception(self, entry: Entry) -> str:
    msg = str(entry.exception).strip()
    # Try to print the source line for empty AssertionError
    if not msg and isinstance(entry.exception, AssertionError):
      return entry.traceback[-2].strip()
    return msg

  def __str__(self) -> str:
    if len(self._exceptions) == 1:
      entry = self._exceptions[0]
      stack = "\n\t".join(entry.info_stack)
      return f"{stack}: {entry.exception}"

    return "\n".join(
        f"{entry.info_stack}: {entry.exception}" for entry in self._exceptions)


# Expose simpler name
Annotator = ExceptionAnnotator

def annotate(
    *stack_entries: str,
    exceptions: TExceptionTypes = (Exception,),
    ignore: TExceptionTypes = tuple(),
    throw_cls: Optional[Type[BaseException]] = MultiException
) -> ExceptionAnnotationScope:
  """Use to annotate an exception.
  By default this will throw a MultiException which can keep track of
  more annotations."""
  return ExceptionAnnotator(throw_cls=throw_cls).capture(
      *stack_entries, exceptions=exceptions, ignore=ignore)


class ArgumentTypeMultiException(MultiException, argparse.ArgumentTypeError):
  pass


def annotate_argparsing(
    *stack_entries: str, exceptions: TExceptionTypes = (Exception,)
) -> ExceptionAnnotationScope:
  """Use this to annotate argument parsing-related code blocks to get more
  readable annotated exception back.
  - Wraps multiple exception in an ArgumentTypeMultiException
  - Single ArgumentTypeError are raised directly
  """
  return annotate(
      *stack_entries,
      exceptions=exceptions,
      throw_cls=ArgumentTypeMultiException)


class UnreachableError(RuntimeError):
  """Used for making checker tools happy in places where it's not directly
  obvious that we always return, for instance due to using one of the above
  exception annotations that could in theory mute exceptions and create an
  additional return path.
  """

  def __init__(self) -> None:
    super().__init__("Unreachable Code")
