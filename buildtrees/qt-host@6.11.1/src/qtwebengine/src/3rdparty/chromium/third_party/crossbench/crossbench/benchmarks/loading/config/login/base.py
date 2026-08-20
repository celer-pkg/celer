# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Final

from typing_extensions import override

from crossbench.benchmarks.loading.config.blocks import ActionBlock

if TYPE_CHECKING:
  from crossbench.cli.config.secrets import UsernamePassword
  from crossbench.runner.run import Run


class BaseLoginBlock(ActionBlock):
  LABEL: Final[str] = "login"

  @override
  def validate(self) -> None:
    super().validate()
    assert self.index == 0, (
        f"Login block has to be the first, but got {self.index}")

  @property
  @override
  def is_login(self) -> bool:
    return True

  def is_logged_in(self,
                   run: Run,
                   secret: UsernamePassword,
                   strict: bool = False) -> bool:
    return run.browser.is_logged_in(secret, strict)


class PresetLoginBlock(BaseLoginBlock):

  @override
  def validate_actions(self) -> None:
    """Skip validation, since PresetLoginBlocks have an unknown number
    of actions."""

  def __len__(self) -> int:
    """LoginBlocks will have at least one action. Given they're not known
    upfront we set this to 1. This also ensures that bool(login_block) is
    True."""
    return 1
