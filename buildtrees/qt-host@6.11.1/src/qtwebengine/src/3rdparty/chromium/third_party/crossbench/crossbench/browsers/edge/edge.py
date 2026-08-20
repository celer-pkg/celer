# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.chromium_based.chromium_based import ChromiumBased
from crossbench.browsers.edge.base import EdgeBaseMixin


class Edge(EdgeBaseMixin, ChromiumBased):
  DEFAULT_FLAGS = (
      "--enable-benchmarking",
      "--disable-extensions",
      "--no-first-run",
  )

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.EDGE | BrowserAttributes.CHROMIUM_BASED
