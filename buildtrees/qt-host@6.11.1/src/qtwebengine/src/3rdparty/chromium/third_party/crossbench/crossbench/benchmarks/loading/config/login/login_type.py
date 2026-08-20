# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum
from typing import TYPE_CHECKING, Type

from crossbench.benchmarks.loading.config.login.google import GoogleLogin
from crossbench.config import ConfigEnum

if TYPE_CHECKING:
  from crossbench.benchmarks.loading.config.login.base import BaseLoginBlock


@enum.unique
class LoginType(ConfigEnum):
  GOOGLE = ("google", "Login for google properties")


LOGIN_LOOKUP: dict[LoginType, Type[BaseLoginBlock]] = {
    LoginType.GOOGLE: GoogleLogin
}
