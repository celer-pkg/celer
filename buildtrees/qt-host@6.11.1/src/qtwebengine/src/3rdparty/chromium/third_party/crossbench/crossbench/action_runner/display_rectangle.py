# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
from typing import TYPE_CHECKING

from crossbench.benchmarks.loading.point import Point

if TYPE_CHECKING:
  from typing_extensions import Self

SCROLL_BOUNDS_OFFSET_FACTOR: float = 0.1

@dataclasses.dataclass(frozen=False)
# Represents a rectangular section of the device's display.
class DisplayRectangle:
  # The top left corner of the rectangle.
  origin: Point
  # The width in pixels of the rectangle.
  width: int
  # The height in pixels of the rectangle.
  height: int

  # Stretches or squishes the rectangle by |factor|
  def __mul__(self, factor: float) -> DisplayRectangle:
    return DisplayRectangle(
        Point(round(self.origin.x * factor), round(self.origin.y * factor)),
        round(self.width * factor), round(self.height * factor))

  __rmul__ = __mul__

  def __bool__(self) -> bool:
    return self.width != 0 and self.height != 0

  # Translates the rectangle into |other|
  def shift_by(self, other: Self) -> DisplayRectangle:
    return DisplayRectangle(
        Point(self.origin.x + other.origin.x, self.origin.y + other.origin.y),
        self.width, self.height)

  def get_scrollable_area(self) -> tuple[int, int, int]:
    scrollable_top = self.top
    scrollable_bottom = self.bottom
    max_swipe_distance = scrollable_bottom - scrollable_top

    trim_amount = int(round(max_swipe_distance * SCROLL_BOUNDS_OFFSET_FACTOR))

    scrollable_top += trim_amount
    scrollable_bottom -= trim_amount

    return (scrollable_top, scrollable_bottom,
            scrollable_bottom - scrollable_top)

  # Returns the sub-rectangle of |other| that exists within |self|.
  # |other| must have the same reference origin as |self|.
  def intersection(self, other: Self) -> DisplayRectangle:
    assert other.left < self.right and other.top < self.bottom, (
        "Rectangles do not intersect. Maybe you need to add 'scroll_into_view'."
    )

    width: int = other.width

    if other.right > self.right:
      width = self.right - other.left

    height: int = other.height

    if other.bottom > self.bottom:
      height = self.bottom - other.top

    return DisplayRectangle(other.origin, width, height)

  @property
  def left(self) -> int:
    return self.origin.x

  @property
  def right(self) -> int:
    return self.origin.x + self.width

  @property
  def top(self) -> int:
    return self.origin.y

  @property
  def bottom(self) -> int:
    return self.origin.y + self.height

  @property
  def mid_x(self) -> int:
    return round(self.origin.x + (self.width / 2))

  @property
  def mid_y(self) -> int:
    return round(self.origin.y + (self.height / 2))

  @property
  def middle(self) -> Point:
    return Point(self.mid_x, self.mid_y)
