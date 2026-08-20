# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.chromium.base import ChromiumBaseMixin
from crossbench.browsers.chromium_based.chromium_based import ChromiumBased


class Chromium(ChromiumBaseMixin, ChromiumBased):

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.CHROMIUM | BrowserAttributes.CHROMIUM_BASED

  @classmethod
  @override
  def type_name(cls) -> str:
    return "chromium"
