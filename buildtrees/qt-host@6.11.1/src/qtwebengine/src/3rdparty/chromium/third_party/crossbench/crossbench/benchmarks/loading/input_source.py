# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum

from crossbench.config import ConfigEnum


@enum.unique
class InputSource(ConfigEnum):
  JS = ("js", "Inject a script into the webpage to simulate the action.")
  TOUCH = ("touch", "Use the touchscreen to perform the action")
  MOUSE = ("mouse", "Use the mouse to perform the action")
  KEYBOARD = ("keyboard", "Use the keyboard to perform the action")
