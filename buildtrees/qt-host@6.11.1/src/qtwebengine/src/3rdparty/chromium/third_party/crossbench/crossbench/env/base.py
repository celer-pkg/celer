# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import logging
from typing import TYPE_CHECKING, Iterable, Optional

import colorama

from crossbench import plt
from crossbench.cli.config.env import EnvConfig, ValidationMode

if TYPE_CHECKING:
  from crossbench.plt.base import CmdArg, Platform


class ValidationError(Exception):
  pass


class BaseEnv(abc.ABC):

  def __init__(self,
               platform: Platform,
               config: Optional[EnvConfig] = None,
               validation_mode: ValidationMode = ValidationMode.THROW) -> None:
    self._platform = platform
    self._config: EnvConfig = config or EnvConfig()
    self._validation_mode: ValidationMode = validation_mode

  @property
  def platform(self) -> Platform:
    return self._platform

  @property
  def config(self) -> EnvConfig:
    return self._config

  @property
  def validation_mode(self) -> ValidationMode:
    return self._validation_mode

  def handle_validation_warning(self, message: str) -> None:
    message = f"Runner/Host environment requests cannot be fulfilled: {message}"
    self.handle_warning(message)

  def handle_warning(self,
                     message: str,
                     allow_interactive: bool = True) -> None:
    """Process a warning, depending on the requested mode, this will
    - throw an error,
    - log a warning,
    - prompts for continue [Yn], or
    - skips (and just debug logs) a warning.
    If returned True (in the prompt mode) the env validation may continue.
    """
    if self._validation_mode == ValidationMode.SKIP:
      logging.debug("Ignoring %s", message)
      return
    if self._validation_mode == ValidationMode.WARN:
      logging.warning(message)
      return
    if self._validation_mode == ValidationMode.PROMPT:
      if allow_interactive:
        result = input(f"{colorama.Fore.RED}{message} Continue?"
                       f"{colorama.Fore.RESET} [Yn]")
        # Accept <enter> as default input to continue.
        if result.lower() != "n":
          return
    elif self._validation_mode != ValidationMode.THROW:
      raise ValueError(
          f"Unknown environment validation mode={self._validation_mode}")
    raise ValidationError(message)

  def check_installed(self,
                      binaries: Iterable[str],
                      message: str = "Missing binaries: {}") -> None:
    assert not isinstance(binaries, str), "Expected iterable of strings."
    missing_binaries = list(
        binary for binary in binaries if not self._platform.which(binary))
    if missing_binaries:
      self.handle_validation_warning(message.format(missing_binaries))

  def check_sh_success(self,
                       *args: CmdArg,
                       message: str = "Could not execute: {}") -> None:
    assert args, "Missing sh arguments"
    try:
      assert self._platform.sh_stdout(*args, quiet=True)
    except plt.SubprocessError as e:
      self.handle_validation_warning(message.format(e))

  def setup(self) -> None:
    self.validate()

  @abc.abstractmethod
  def validate(self) -> None:
    pass
