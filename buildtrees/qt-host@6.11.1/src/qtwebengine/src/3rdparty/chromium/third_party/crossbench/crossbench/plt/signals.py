# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum
import signal
from typing import TypeAlias


class Signals(enum.IntEnum):
  pass


@enum.unique
class WinSignals(Signals):
  CTRL_C_EVENT = 0
  CTRL_BREAK_EVENT = 1
  SIGABRT = signal.SIGABRT
  SIGFPE = signal.SIGFPE
  SIGILL = signal.SIGILL
  SIGINT = signal.SIGINT
  SIGSEGV = signal.SIGSEGV
  SIGTERM = signal.SIGTERM
  SIGBREAK = 21


# Python doesn't allow enum subclasses so we have to duplicate enum values
# This thoroughly tested in SignalsTestCase for consistency.
class _PosixSignals(Signals):
  pass


class PosixBaseSignal(_PosixSignals):
  """ Signals names AND values supported on all posix platforms. """
  SIGABRT = signal.SIGABRT
  SIGFPE = signal.SIGFPE
  SIGILL = signal.SIGILL
  SIGINT = signal.SIGINT
  SIGSEGV = signal.SIGSEGV
  SIGTERM = signal.SIGTERM
  SIGHUP = 1
  SIGQUIT = 3
  SIGTRAP = 5
  SIGKILL = 9
  SIGPIPE = 13
  SIGALRM = 14
  SIGTTIN = 21
  SIGTTOU = 22
  SIGXCPU = 24
  SIGXFSZ = 25
  SIGVTALRM = 26
  SIGPROF = 27
  SIGWINCH = 28


class PosixSignals(_PosixSignals):
  """ Signal names (not values) support on all posix platforms.
  See specific subclasses for platform-specific values."""
  SIGABRT = signal.SIGABRT
  SIGFPE = signal.SIGFPE
  SIGILL = signal.SIGILL
  SIGINT = signal.SIGINT
  SIGSEGV = signal.SIGSEGV
  SIGTERM = signal.SIGTERM
  SIGHUP = 1
  SIGQUIT = 3
  SIGTRAP = 5
  SIGIOT = 6
  SIGBUS = 7
  SIGKILL = 9
  SIGUSR1 = 10
  SIGUSR2 = 12
  SIGPIPE = 13
  SIGALRM = 14
  SIGCLD = 17
  SIGCHLD = 17
  SIGCONT = 18
  SIGSTOP = 19
  SIGTSTP = 20
  SIGTTIN = 21
  SIGTTOU = 22
  SIGURG = 23
  SIGXCPU = 24
  SIGXFSZ = 25
  SIGVTALRM = 26
  SIGPROF = 27
  SIGWINCH = 28
  SIGIO = 29
  SIGPOLL = 29
  SIGPWR = 30
  SIGSYS = 31
  SIGRTMIN = 34
  SIGRTMAX = 64


class LinuxSignals(_PosixSignals):
  # Source:
  # https://man7.org/linux/man-pages/man7/signal.7.html
  SIGABRT = signal.SIGABRT
  SIGFPE = signal.SIGFPE
  SIGILL = signal.SIGILL
  SIGINT = signal.SIGINT
  SIGSEGV = signal.SIGSEGV
  SIGTERM = signal.SIGTERM
  SIGHUP = 1
  SIGQUIT = 3
  SIGTRAP = 5
  SIGIOT = 6
  SIGBUS = 7
  SIGKILL = 9
  SIGUSR1 = 10
  SIGUSR2 = 12
  SIGPIPE = 13
  SIGALRM = 14
  SIGSTKFLT = 16
  SIGCLD = 17
  SIGCHLD = 17
  SIGCONT = 18
  SIGSTOP = 19
  SIGTSTP = 20
  SIGTTIN = 21
  SIGTTOU = 22
  SIGURG = 23
  SIGXCPU = 24
  SIGXFSZ = 25
  SIGVTALRM = 26
  SIGPROF = 27
  SIGWINCH = 28
  SIGIO = 29
  SIGPOLL = 29
  SIGPWR = 30
  SIGSYS = 31
  SIGRTMIN = 34
  SIGRTMAX = 64


class MacOSSignals(_PosixSignals):
  # Source:
  # https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/signal.3.html
  SIGABRT = signal.SIGABRT
  SIGFPE = signal.SIGFPE
  SIGILL = signal.SIGILL
  SIGINT = signal.SIGINT
  SIGSEGV = signal.SIGSEGV
  SIGTERM = signal.SIGTERM
  SIGHUP = 1
  SIGQUIT = 3
  SIGTRAP = 5
  SIGEMT = 7
  SIGIOT = 6
  SIGKILL = 9
  SIGBUS = 10
  SIGSYS = 12
  SIGPIPE = 13
  SIGALRM = 14
  SIGURG = 16
  SIGSTOP = 17
  SIGTSTP = 18
  SIGCONT = 19
  SIGCHLD = 20
  SIGTTIN = 21
  SIGTTOU = 22
  SIGIO = 23
  SIGXCPU = 24
  SIGXFSZ = 25
  SIGVTALRM = 26
  SIGPROF = 27
  SIGWINCH = 28
  SIGINFO = 29
  SIGUSR1 = 30
  SIGUSR2 = 31


# Type unions of concrete Signals implementations.
AnySignals: TypeAlias = (
    WinSignals | PosixBaseSignal | PosixSignals | LinuxSignals | MacOSSignals)
AnyPosixSignals: TypeAlias = (
    PosixBaseSignal | PosixSignals | LinuxSignals | MacOSSignals)
