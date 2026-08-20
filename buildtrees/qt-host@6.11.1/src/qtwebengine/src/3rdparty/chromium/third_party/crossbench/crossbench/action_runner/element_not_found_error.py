# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import annotations


class ElementNotFoundError(RuntimeError):

  def __init__(self, selector: str) -> None:
    message = f"Could not find matching DOM element: {repr(selector)}"
    super().__init__(message)
