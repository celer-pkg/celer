#!/usr/bin/env vpython3
# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
from crossbench.cli.btp import BTPUtil

if __name__ == "__main__":
  btp = BTPUtil()
  btp.run(sys.argv)
