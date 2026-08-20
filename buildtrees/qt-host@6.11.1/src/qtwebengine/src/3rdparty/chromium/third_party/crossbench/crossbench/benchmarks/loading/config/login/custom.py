# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses

from typing_extensions import override

from crossbench.benchmarks.loading.config.login.base import BaseLoginBlock
from crossbench.benchmarks.loading.config.login.login_type import (LOGIN_LOOKUP,
                                                                   LoginType)


@dataclasses.dataclass(frozen=True)
class LoginBlock(BaseLoginBlock):

  @classmethod
  @override
  def parse_str(cls, value: str) -> BaseLoginBlock:  # type: ignore
    login_type = LoginType.parse(value)
    return LOGIN_LOOKUP[login_type]()
