# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from asyncio.subprocess import Process
from subprocess import Popen
from typing import Sequence, TypeAlias

from crossbench import path as pth

CmdArg: TypeAlias = pth.AnyPathLike
SequenceCmdArgs: TypeAlias = Sequence[CmdArg]
ListCmdArgs: TypeAlias = list[CmdArg]
TupleCmdArgs: TypeAlias = tuple[CmdArg, ...]
CmdArgs: TypeAlias = ListCmdArgs | TupleCmdArgs

ProcessLike: TypeAlias = Popen | Process | int
