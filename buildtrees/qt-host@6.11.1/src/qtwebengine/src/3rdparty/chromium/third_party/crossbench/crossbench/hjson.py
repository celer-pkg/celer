# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import Any

import hjson


def _check_for_duplicate_keys(key_values) -> dict[str, Any]:
  result = {}
  for key, value in key_values:
    if key in result:
      raise ValueError(f"Duplicate key in hjson: {key}")
    result[key] = value
  return result


def load_unique_keys(file) -> Any:
  return hjson.load(file, object_pairs_hook=_check_for_duplicate_keys)


def loads_unique_keys(string_value: str) -> Any:
  return hjson.loads(string_value, object_pairs_hook=_check_for_duplicate_keys)
