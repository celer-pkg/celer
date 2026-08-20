# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import enum

from crossbench.config import ConfigEnum


@enum.unique
class ButtonClick(ConfigEnum):
  LEFT = ("left", "Press left mouse button")
  RIGHT = ("right", "Press right mouse button")
  MIDDLE = ("middle", "Press middle mouse button")


@enum.unique
class ReadyState(ConfigEnum):
  """See https://developer.mozilla.org/en-US/docs/Web/API/Document/readyState"""
  # Non-blocking:
  ANY = ("any", "Ignore ready state")
  # Blocking (on dom event):
  LOADING = ("loading", "The document is still loading.")
  INTERACTIVE = ("interactive", "The document has finished loading "
                 "but sub-resources might still be loading")
  COMPLETE = ("complete",
              "The document and all sub-resources have finished loading.")


@enum.unique
class WindowTarget(ConfigEnum):
  """See https://developer.mozilla.org/en-US/docs/Web/API/Window/open"""
  # TODO: pull this out to the browsers and use this enum instead of the strings
  # in the browser show_url implementations.
  SELF = ("_self", "The current browsing context. (Default)")
  BLANK = ("_blank", "Usually a new tab, but users can configure browsers "
           "to open a new window instead.")
  PARENT = ("_parent", "The parent browsing context of the current one. "
            "If no parent, behaves as _self.")
  TOP = ("_top", "The topmost browsing context "
         "(the 'highest' context that's an ancestor of the current one). "
         "If no ancestors, behaves as _self.")
  # The following options are Crossbench specific and are not understoon by the
  # underlying call to window.open() in JS.
  NEW_TAB = ("_new_tab", "A new tab.")
  NEW_WINDOW = ("_new_window", "A new window.")
