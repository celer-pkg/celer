# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Most of this code is originally based on the colored_traceback package:

# Copyright (c) 2014, Anton Backer <olegov@gmail.com>
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

from __future__ import annotations

import io
import logging
import traceback
from typing import Callable

from pygments import highlight
from pygments.formatters import get_formatter_by_name
from pygments.lexers import get_lexer_by_name
from pygments.util import ClassNotFound

from crossbench.cli import ui


def _get_term_color_support() -> int:
  try:
    import curses  # pylint: disable=import-outside-toplevel
  except ImportError:
    # Probably Windows, which doesn't have great curses support
    return 16
  curses.setupterm()
  return curses.tigetnum("colors")


def _determine_formatter(style: str = "default", colors=None):
  colors = colors or _get_term_color_support()
  logging.debug("Detected support for %s colors", colors)
  if colors == 256:
    fmt_options = {"style": style}
  elif style in ("light", "dark"):
    fmt_options = {"bg": style}
  else:
    fmt_options = {"bg": "dark"}
  fmt_alias = "terminal256" if colors == 256 else "terminal"
  try:
    return get_formatter_by_name(fmt_alias, **fmt_options)
  except ClassNotFound as e:
    logging.debug("Error setting up colorizer: %s", e)
    return get_formatter_by_name(fmt_alias)


def _get_causes(ex_type, ex_value, tb) -> list[traceback.TracebackException]:
  tb_exception = traceback.TracebackException(ex_type, ex_value, tb)
  causes: list[traceback.TracebackException] = []
  current = tb_exception
  while True:
    causes.append(current)
    if current.__cause__ is not None:
      current = current.__cause__
    elif (current.__context__ is not None and not current.__suppress_context__):
      current = current.__context__
    else:
      break
  return causes


def _get_tb_printer() -> Callable[[str], None]:
  try:
    if ui.COLOR_LOGGING:
      return _get_formatting_tb_printer()
  except Exception as e:  # pylint: disable=broad-exception-caught
    logging.debug("Failed to initializer error formatting: %s", e)
  return print


def _get_formatting_tb_printer() -> Callable[[str], None]:
  formatter = _determine_formatter()
  lexer = get_lexer_by_name("py3tb")

  def formatting_tb_printer(formatted_tb: str) -> None:
    print(highlight(formatted_tb, lexer, formatter))

  return formatting_tb_printer


def excepthook(ex_type, ex_value, tb) -> None:
  causes: list[traceback.TracebackException] = _get_causes(
      ex_type, ex_value, tb)
  tb_printer = _get_tb_printer()

  print("-" * 80)
  # Print exception causes in non-reverse order compared to the default
  # python traceback formatter. This way we keep the stack order consistent
  # where the inner frame is *always* printed below the outer frame.
  # This makes scanning for exceptions much more intuitive.
  for i, cause in enumerate(causes):
    if i:
      print("")
    buffer = io.StringIO()
    # TODO: use better helpers when migrated to 3.11.
    for line in cause.format(chain=False):
      print(line, file=buffer, end="")
    tb_printer(buffer.getvalue())

  if len(causes) > 0:
    print("-" * 80)
    print("OUTERMOST EXCEPTION SUMMARY:")
    print("-" * 80)
    formatted_tb: str = _print_outermost_exception(causes[0])
    tb_printer(formatted_tb)


def _print_outermost_exception(
    last_exception: traceback.TracebackException) -> str:
  buffer = io.StringIO()
  if stack := last_exception.stack:
    innermost_frame: traceback.FrameSummary = stack[-1]
    # TODO: use better helpers when migrated to 3.11.
    stack_summary = traceback.StackSummary.from_list((innermost_frame,))
    print("  ...", file=buffer)
    for line in stack_summary.format():
      print(line, file=buffer, end="")
  # TODO: use better helpers when migrated to 3.11.
  for line in last_exception.format_exception_only():
    print(line, file=buffer, end=None)
  return buffer.getvalue()
