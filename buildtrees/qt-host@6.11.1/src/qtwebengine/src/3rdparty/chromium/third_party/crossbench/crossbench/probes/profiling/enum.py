# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum
from typing import Optional

from crossbench.str_enum_with_help import StrEnumWithHelp


@enum.unique
class CleanupMode(StrEnumWithHelp):

  @classmethod
  def _missing_(cls, value) -> Optional[CleanupMode]:
    if value is True:
      return CleanupMode.ALWAYS
    if value is False:
      return CleanupMode.NEVER
    return super()._missing_(value)

  ALWAYS = ("always", "Always clean up temp files")
  AUTO = ("auto", "Best-guess auto-cleanup")
  NEVER = ("never", "Always clean up temp files")


@enum.unique
class TargetMode(StrEnumWithHelp):
  RENDERER_MAIN_ONLY = ("renderer_main_only",
                        "Profile Renderer Main thread only")
  RENDERER_PROCESS_ONLY = ("renderer_process_only",
                           "Profile Renderer process only")
  BROWSER_APP_ONLY = ("browser_app_only",
                      "Profile all processes of the Browser App only")
  SYSTEM_WIDE = ("system_wide", "Run system-wide profiling")


@enum.unique
class CallGraphMode(StrEnumWithHelp):
  # Refer to the documentation below for more details and comparison
  # between these options:
  # https://android.googlesource.com/platform/system/extras/+/master/simpleperf/doc/README.md.
  NO_CALL_GRAPH = ("no_call_graph", "Do not record a call graph")
  DWARF = ("dwarf", "Run DWARF-based unwinding unwinding")
  FRAME_POINTER = ("fp", "Run frame pointer unwinding")
