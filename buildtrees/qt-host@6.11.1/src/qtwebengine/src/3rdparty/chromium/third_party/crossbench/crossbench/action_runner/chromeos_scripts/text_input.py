# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pytype: skip-file

# This script is to be run directly on a ChromeOS device to redirect characters
# read from stdin to be typed on an emulated keyboard device.

import sys

import uinput.cros_keys

while True:
  char = sys.stdin.read(1)

  if not char:
    break

  uinput.cros_keys.type_chars(char)
