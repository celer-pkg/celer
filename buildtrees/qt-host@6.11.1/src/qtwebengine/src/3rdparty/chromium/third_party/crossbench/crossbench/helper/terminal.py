# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import Final

CLEAR_END: Final[str] = "\x1b[J"
STORE_CURSOR_POS: Final[str] = "\x1b[s"
RESTORE_CURSOR_POS: Final[str] = "\x1b[u"
CURSOR_RIGHT = "\x1b[%sC"
