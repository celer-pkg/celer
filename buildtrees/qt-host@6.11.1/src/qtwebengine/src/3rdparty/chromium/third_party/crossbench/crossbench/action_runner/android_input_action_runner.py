# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
import re
from typing import TYPE_CHECKING, Optional, cast

from crossbench.action_runner.base import InputSourceNotImplementedError
from crossbench.action_runner.default_action_runner import DefaultActionRunner
from crossbench.action_runner.display_rectangle import DisplayRectangle
from crossbench.action_runner.element_not_found_error import \
    ElementNotFoundError
from crossbench.action_runner.screenshot_annotation import (
    ScreenshotPointAnnotation, ScreenshotRectAnnotation)
from crossbench.benchmarks.loading.point import Point

if TYPE_CHECKING:
  from crossbench.action_runner.action import all as i_action
  from crossbench.action_runner.action.position import UiSelectorConfig
  from crossbench.browsers.attributes import BrowserAttributes
  from crossbench.plt.android_adb import AndroidAdbPlatform
  from crossbench.runner.actions import Actions
  from crossbench.runner.run import Run


class ViewportInfo:

  def __init__(self,
               raw_chrome_window_bounds: DisplayRectangle,
               window_inner_height: int,
               window_inner_width: int,
               element_rect: Optional[DisplayRectangle] = None) -> None:
    self._element_rect: DisplayRectangle | None = None

    # On android, clank does not report the correct window.devicePixelRatio
    # when a page is zoomed.
    # Zoom can happen automatically on load with pages that force a certain
    # viewport width (such as speedometer), so calculate the ratio manually.
    # Note: this calculation assumes there are no system borders on the side of
    # the chrome window.
    self._actual_pixel_ratio: float = float(raw_chrome_window_bounds.width /
                                            window_inner_width)

    window_inner_height = int(
        round(self.actual_pixel_ratio * window_inner_height))
    window_inner_width = int(
        round(self.actual_pixel_ratio * window_inner_width))

    # On Android there may be a system added border from the top of the app view
    # that is included in the mAppBounds rectangle dimensions. Calculate the
    # height of this border using the difference between the height reported by
    # chrome and the height reported by android.
    top_border_height = raw_chrome_window_bounds.height - window_inner_height

    self._chrome_window: DisplayRectangle = DisplayRectangle(
        Point(raw_chrome_window_bounds.origin.x,
              raw_chrome_window_bounds.origin.y + top_border_height),
        raw_chrome_window_bounds.width,
        raw_chrome_window_bounds.height - top_border_height)

    if element_rect:
      self._element_rect = (element_rect * self.actual_pixel_ratio).shift_by(
          self._chrome_window)
      self._element_rect = self.chrome_window.intersection(self._element_rect)

  @property
  def chrome_window(self) -> DisplayRectangle:
    return self._chrome_window

  @property
  def actual_pixel_ratio(self) -> float:
    return self._actual_pixel_ratio

  def element_rect(self) -> Optional[DisplayRectangle]:
    return self._element_rect

  def element_center(self) -> Optional[Point]:
    if not self._element_rect:
      return None
    return self._element_rect.middle

  def css_to_native_distance(self, distance: float) -> float:
    return distance * self.actual_pixel_ratio


class AndroidInputActionRunner(DefaultActionRunner):
  """Custom ActionRunner for Android."""

  # Represents the position of the chrome main window relative to the entire
  # screen as reported by Android window manager.
  _raw_chrome_window_bounds: DisplayRectangle | None = None

  @property
  def raw_chrome_window_bounds(self) -> DisplayRectangle:
    assert self._raw_chrome_window_bounds, "Uninitialized chrome window bounds"
    return self._raw_chrome_window_bounds

  _BOUNDS_RE = re.compile(
      r"mAppBounds=Rect\((?P<left>\d+), (?P<top>\d+) - (?P<right>\d+),"
      r" (?P<bottom>\d+)\)")

  _GET_JS_VALUES = """
const found_element = arguments[0] && element;
if(found_element && arguments[1]) element.scrollIntoView();
rect = found_element ? element.getBoundingClientRect() : new DOMRect();
return [
  found_element,
  window.innerHeight,
  window.innerWidth,
  rect.left,
  rect.top,
  rect.width,
  rect.height
];"""

  def scroll_touch(self, run: Run, action: i_action.ScrollAction) -> None:
    with run.actions("ScrollAction", measure=False) as actions:

      viewport_info = self._get_viewport_info(run, actions, action.selector)

      # The scroll distance is specified in terms of css pixels so adjust to the
      # native pixel density.
      total_scroll_distance = (
          viewport_info.css_to_native_distance(action.distance))

      # Default to scrolling within the entire chrome window.
      scroll_area: DisplayRectangle = viewport_info.chrome_window

      if action.selector:
        if element_rect := viewport_info.element_rect():
          scroll_area = element_rect
        else:
          if action.required:
            raise ElementNotFoundError(action.selector)
          return

      (scrollable_top, scrollable_bottom,
       max_swipe_distance) = scroll_area.get_scrollable_area()

      remaining_distance = abs(total_scroll_distance)

      while remaining_distance > 0:

        current_distance = min(max_swipe_distance, remaining_distance)

        # The duration for this swipe should be only a fraction of the total
        # duration since the entire distance may not be covered in one swipe.
        current_duration = (current_distance /
                            abs(total_scroll_distance)) * action.duration

        # If scrolling down, the swipe should start at the bottom and end above.
        y_start = scrollable_bottom
        y_end = scrollable_bottom - current_distance

        # If scrolling up, the swipe should start at the top and end below.
        if total_scroll_distance < 0:
          y_start = scrollable_top
          y_end = scrollable_top + current_distance

        self._swipe_impl(run, round(scroll_area.mid_x), round(y_start),
                         round(scroll_area.mid_x), round(y_end),
                         current_duration)

        remaining_distance -= current_distance

  def click_touch(self, run: Run, action: i_action.ClickAction) -> None:
    self._click_impl(run, action, False)

  def click_mouse(self, run: Run, action: i_action.ClickAction) -> None:
    self._click_impl(run, action, True)

  def swipe(self, run: Run, action: i_action.SwipeAction) -> None:
    with run.actions("SwipeAction", measure=False):
      self._swipe_impl(run, action.start_x, action.start_y, action.end_x,
                       action.end_y, action.duration)

  def text_input_keyboard(self, run: Run,
                          action: i_action.TextInputAction) -> None:
    if action.text:
      self._rate_limit_keystrokes(run, action, self._type_characters)
    elif keyevent := action.keyevent:
      self._send_keyevent(run, keyevent)

  def _click_impl(self, run: Run, action: i_action.ClickAction,
                  use_mouse: bool) -> None:
    if action.duration > dt.timedelta():
      raise InputSourceNotImplementedError(self, action, action.input_source,
                                           "Non-zero duration not implemented")
    coordinates: Point | None = None
    with run.actions("ClickAction", measure=False) as actions:

      if coordinates_config := action.position.coordinates:
        coordinates = coordinates_config.point()
      elif ui_selector := action.position.ui_selector:
        if use_mouse:
          raise InputSourceNotImplementedError(
            self, action, action.input_source,
            "Mouse actions not implemented for UiSelectorConfig")
        self._click_ui_selector(run, ui_selector, action.timeout)
      elif selector_config := action.position.selector:
        if selector_config.wait:
          self.wait_for_element_impl(
              actions,
              selector=selector_config.selector,
              timeout=action.timeout,
              scroll_into_view=selector_config.scroll_into_view,
              check_element_rect=True,
              required=selector_config.required)

        viewport_info = self._get_viewport_info(
            run, actions, selector_config.selector,
            selector_config.scroll_into_view)

        rect = viewport_info.element_rect()
        if not rect:
          logging.warning("No clickable element_rect found for %s",
                          selector_config.selector)
          if selector_config.required:
            raise ElementNotFoundError(selector_config.selector)
          return

        self.add_failure_screenshot_annotation(
            ScreenshotRectAnnotation(
                label="Chrome viewport", rect=viewport_info.chrome_window))
        self.add_failure_screenshot_annotation(
            ScreenshotRectAnnotation(label=selector_config.selector, rect=rect))
        coordinates = Point(rect.mid_x, rect.mid_y)

      if not action.position.ui_selector:
        cmd: list[str] = ["input"]

        if use_mouse:
          cmd.append("mouse")
        assert coordinates, "missing coordinates"
        self.add_failure_screenshot_annotation(
            ScreenshotPointAnnotation(label="click", point=coordinates))
        cmd.extend(["tap", str(coordinates.x), str(coordinates.y)])

        run.browser_platform.sh(*cmd)

      if action.verify:
        self.wait_for_element_impl(
            actions,
            selector=action.verify,
            timeout=action.timeout,
            check_element_rect=True)

  def _swipe_impl(self, run: Run, start_x: int, start_y: int, end_x: int,
                  end_y: int, duration: dt.timedelta) -> None:

    duration_millis = round(duration // dt.timedelta(milliseconds=1))

    run.browser_platform.sh("input", "swipe", str(start_x), str(start_y),
                            str(end_x), str(end_y), str(duration_millis))

  def _get_viewport_info(self,
                         run: Run,
                         actions: Actions,
                         selector: Optional[str] = None,
                         scroll_into_view: bool = False) -> ViewportInfo:

    script = ""

    if selector:
      selector, script = self.get_selector_script(selector)

    script += self._GET_JS_VALUES

    (found_element, inner_height, inner_width, left, top, width,
     height) = actions.js(
         script, arguments=[selector, scroll_into_view])

    # If the chrome window position has not yet been found,
    # initialize it now.
    # Note: this assumes the chrome app will not be moved or resized during
    # the test.
    if not self._raw_chrome_window_bounds:
      self._raw_chrome_window_bounds = self._find_chrome_window_size(run)

    element_rect: DisplayRectangle | None = None
    if found_element:
      element_rect = DisplayRectangle(Point(left, top), width, height)

    return ViewportInfo(self.raw_chrome_window_bounds, inner_height,
                        inner_width, element_rect)


  # Returns the name of the browser's main window as reported by android's
  # window manager.
  def _get_browser_window_name(self,
                               browser_attributes: BrowserAttributes) -> str:
    if browser_attributes.is_chrome:
      return "chrome.Main"

    raise RuntimeError("Unsupported browser for android action runner.")

  def _find_chrome_window_size(self, run: Run) -> DisplayRectangle:
    # Find the chrome app window position by dumping the android app window
    # list.
    #
    # Chrome's main view is always called 'chrome.Main' and is followed by the
    # configuration for that window.
    #
    # The mAppBounds config of the chrome.Main window contains the dimensions
    # for the visible part of the current chrome window formatted like this for
    # a 800 height by 480 width window:
    #
    # mAppBounds=Rect(0, 0 - 480, 800)
    browser_main_window_name = self._get_browser_window_name(
        run.browser.attributes())

    raw_window_config = run.browser_platform.sh_stdout("dumpsys", "window",
                                                       "windows")

    raw_window_config = raw_window_config[raw_window_config
                                          .find(browser_main_window_name):]

    match = self._BOUNDS_RE.search(raw_window_config)
    if not match:
      raise RuntimeError("Could not find chrome window bounds")

    width = int(match["right"]) - int(match["left"])
    height = int(match["bottom"]) - int(match["top"])

    return DisplayRectangle(
        Point(int(match["left"]), int(match["top"])), width, height)

  def _type_characters(self, run: Run, _: Actions, characters: str) -> None:
    # TODO(kalutes) handle special characters and other whitespaces like '\t'

    # The 'input text' command cannot handle spaces directly. Replace space
    # characters with the encoding '%s'.
    characters = characters.replace(" ", "%s")
    run.browser_platform.sh("input", "keyboard", "text", characters)

  def _send_keyevent(self, run: Run, keyevent:str) -> None:
    run.browser_platform.sh("input", "keyevent", keyevent)

  def _click_ui_selector(self,
                         run: Run,
                         ui_selector: UiSelectorConfig,
                         timeout: dt.timedelta) -> None:
    ad = cast("AndroidAdbPlatform", run.browser_platform).uiautomator_device
    selector_dict = ui_selector.to_json()
    ui_object = ad.ui(**ui_selector.to_json())
    # This verification step verifies if the element exists.
    assert ui_object.wait.exists(timeout=timeout), (
      f"Element with selector {selector_dict} not found")
    ui_object.click()
