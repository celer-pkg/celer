# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pytype: skip-file

# This script is to be run directly on a ChromeOS device to emulate a mouse and
# then move to and click a location.

import sys
import time

import uinput

screen_width = int(sys.argv[1])
screen_height = int(sys.argv[2])

events = (
    uinput.ABS_X + (0, screen_width, 0, 0),
    uinput.ABS_Y + (0, screen_height, 0, 0),
    uinput.BTN_LEFT,
    uinput.BTN_RIGHT,
)

with uinput.Device(events) as device:
  # The system needs a bit of time before it can start processing events from
  # the newly registered device.
  time.sleep(0.2)

  sys.stdout.write("0\n")
  sys.stdout.flush()

  while True:
    duration = float(sys.stdin.readline())
    x = int(sys.stdin.readline())
    y = int(sys.stdin.readline())

    device.emit(uinput.ABS_X, x, syn=False)
    device.emit(uinput.ABS_Y, y)

    device.emit(uinput.BTN_LEFT, 1)
    time.sleep(duration)
    device.emit(uinput.BTN_LEFT, 0)

    sys.stdout.write("0\n")
    sys.stdout.flush()
