# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import dataclasses
from typing import TYPE_CHECKING, Sequence
from xml.sax.saxutils import escape

from typing_extensions import override

if TYPE_CHECKING:
  from crossbench.action_runner.display_rectangle import DisplayRectangle
  from crossbench.benchmarks.loading.point import Point


@dataclasses.dataclass(frozen=True)
class ScreenshotAnnotation(abc.ABC):
  label: str

  @abc.abstractmethod
  def svg_annotation(self) -> str:
    pass


def annotate_screenshot_svg(screen_width: int, screen_height: int,
                            screenshot_file: str,
                            annotations: Sequence[ScreenshotAnnotation]) -> str:
  all_annotations = ''.join(
      [annotation.svg_annotation() for annotation in annotations])
  return (f'<svg version="1.1"'
          f' width="{screen_width}" height="{screen_height}"'
          ' xmlns="http://www.w3.org/2000/svg">'
          f'<image href="{screenshot_file}"'
          f' width="{screen_width}" height="{screen_height}" />'
          f'{all_annotations}'
          '</svg>')


@dataclasses.dataclass(frozen=True)
class ScreenshotRectAnnotation(ScreenshotAnnotation):
  rect: DisplayRectangle

  @override
  def svg_annotation(self) -> str:
    rect = self.rect
    return (f'<rect x="{rect.left}" y="{rect.top}"'
            f' width="{rect.width}" height="{rect.height}"'
            ' fill="rgb(255 255 0 / 0.5)">'
            f'<title>{escape(self.label)}</title>'
            '</rect>')


@dataclasses.dataclass(frozen=True)
class ScreenshotPointAnnotation(ScreenshotAnnotation):
  point: Point

  @override
  def svg_annotation(self) -> str:
    x = self.point.x
    y = self.point.y
    return (f'<g><title>{escape(self.label)}</title>'
            f'<polygon points="{x},{y} {x-48},{y-24} {x-24},{y-48}"'
            ' fill="rgb(0 0 255 / 0.33)" />'
            f'<rect x="{x-0.5}" y="{y-0.5}"'
            ' width="1" height="1" fill="rgb(0 0 255)" />'
            '</g>')
