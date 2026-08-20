# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import difflib
from typing import TYPE_CHECKING, Any, Callable, Iterable, Optional, TypeVar

if TYPE_CHECKING:
  from crossbench.path import AnyPath

  InputT = TypeVar("InputT")
  KeyT = TypeVar("KeyT")
  GroupT = TypeVar("GroupT")
  PathT = TypeVar("PathT", bound=AnyPath)


def group_by(
    collection: Iterable[InputT],
    key: Callable[[InputT], KeyT],
    value: Optional[Callable[[InputT], Any]] = None,
    group: Optional[Callable[[KeyT], GroupT]] = None,
    sort_key: Optional[Callable[[tuple[KeyT, GroupT]], Any]] = str
) -> dict[KeyT, GroupT]:
  """
  Works similar to itertools.groupby but does a global, SQL-style grouping
  instead of a line-by-line basis like uniq.

  key:   a function that returns the grouping key for a group
  group: a function that accepts a group_key and returns a group object that
    has an append() method.
  """
  if not key:  # type: ignore
    raise ValueError("No key function provided")
  key_fn = key
  value_fn = value or (lambda item: item)
  group_fn: Callable[[KeyT], GroupT] = group or (lambda key: [])  # type: ignore
  groups: dict[KeyT, GroupT] = {}
  for input_item in collection:
    group_key: KeyT = key_fn(input_item)
    group_item = value_fn(input_item)
    if group_key not in groups:
      new_group: GroupT = group_fn(group_key)
      groups[group_key] = new_group
      new_group.append(group_item)  # type: ignore
    else:
      groups[group_key].append(group_item)  # type: ignore
  if sort_key:
    # sort keys as well for more predictable behavior
    return dict(sorted(groups.items(), key=sort_key))
  return dict(groups.items())


def close_matches_message(choice: str,
                          choices: Iterable[str]) -> tuple[str, str | None]:
  error_message: str = ""
  similar_choices = difflib.get_close_matches(choice, choices)
  alternative: str | None = None
  if len(similar_choices) > 1:
    error_message += f" Did you mean one of {', '.join(similar_choices)}?"
  elif len(similar_choices) == 1:
    alternative = similar_choices[0]
    error_message += f" Did you mean {repr(alternative)}?"
  return error_message, alternative
