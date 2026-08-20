# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import dataclasses
import datetime as dt
import shlex
import subprocess
from math import ceil
from typing import TYPE_CHECKING, Optional, Self

import crossbench.path as pth
from crossbench.action_runner.default_action_runner import DefaultActionRunner
from crossbench.action_runner.display_rectangle import DisplayRectangle
from crossbench.action_runner.element_not_found_error import \
    ElementNotFoundError
from crossbench.action_runner.screenshot_annotation import (
    ScreenshotPointAnnotation, ScreenshotRectAnnotation)
from crossbench.benchmarks.loading.point import Point
from crossbench.parse import NumberParser

if TYPE_CHECKING:
  from crossbench.action_runner.action import all as i_action
  from crossbench.runner.actions import Actions
  from crossbench.runner.run import Run

SCRIPTS_DIR = pth.LocalPath(__file__).parent / "chromeos_scripts"


class ChromeOSViewportInfo:

  def __init__(self, device_pixel_ratio: float, window_outer_width: int,
               window_inner_width: int, window_inner_height: int,
               screen_width: int, screen_height: int, screen_avail_width: int,
               screen_avail_height: int, window_offset_x: int,
               window_offset_y: int,
               element_rect: Optional[DisplayRectangle]) -> None:
    # The actual screen width and height in pixels.
    # Corrects for any zoom/scaling factors.
    # 80 is a common factor of most display pixel widths, so use it as a common
    # factor to ensure integer division.
    screen_width_pixels = round(
        screen_width * device_pixel_ratio /
        (window_outer_width / window_inner_width) / 80) * 80

    # 60 is a common factor of most display pixel heights, so use it as a common
    # factor to ensure integer division.
    screen_height_pixels = round(
        screen_height * device_pixel_ratio /
        (window_outer_width / window_inner_width) / 60) * 60

    self._actual_pixel_ratio = screen_width_pixels / screen_avail_width

    screen_avail_width = round(self.css_to_native_distance(screen_avail_width))
    screen_avail_height = round(
        self.css_to_native_distance(screen_avail_height))

    window_inner_width = round(self.css_to_native_distance(window_inner_width))
    window_inner_height = round(
        self.css_to_native_distance(window_inner_height))

    window_offset_x = round(self.css_to_native_distance(window_offset_x))

    window_offset_y = round(self.css_to_native_distance(window_offset_y))
    window_offset_y += (screen_avail_height - window_inner_height)

    visible_width = min(window_inner_width,
                        screen_avail_width - window_offset_x)
    visible_height = min(window_inner_height,
                         round(screen_avail_height - window_offset_y))

    self._native_screen = DisplayRectangle(
        Point(0, 0), screen_width_pixels, screen_height_pixels)

    self._browser_viewable = DisplayRectangle(
        Point(window_offset_x, window_offset_y), visible_width, visible_height)

    self._element_rect: DisplayRectangle | None = None
    if element_rect:
      self._element_rect = self._dom_rect_to_native_rect(element_rect)

  @property
  def browser_viewable(self) -> DisplayRectangle:
    return self._browser_viewable

  @property
  def native_screen(self) -> DisplayRectangle:
    return self._native_screen

  @property
  def element_rect(self) -> Optional[DisplayRectangle]:
    return self._element_rect

  def _dom_rect_to_native_rect(self,
                               dom_rect: DisplayRectangle) -> DisplayRectangle:
    browser_viewable = self.browser_viewable
    correct_ratio_rect = dom_rect * self._actual_pixel_ratio

    adjusted_left = correct_ratio_rect.left + browser_viewable.left
    adjusted_top = correct_ratio_rect.top + browser_viewable.top
    adjusted_width = min(correct_ratio_rect.width,
                         self._native_screen.width - correct_ratio_rect.left)
    adjusted_height = min(correct_ratio_rect.height,
                          self._native_screen.height - correct_ratio_rect.top)

    return DisplayRectangle(
        Point(adjusted_left, adjusted_top), adjusted_width, adjusted_height)

  def css_to_native_distance(self, distance: float) -> float:
    return distance * self._actual_pixel_ratio


@dataclasses.dataclass(frozen=True)
# Stores the configuration of the touchscreen device for the Chromebook.
class TouchDevice:
  # The path of the device.
  device_path: str
  # The maximum X value for a touch input.
  x_max: int
  # The maximum Y value for a touch input.
  y_max: int

  @classmethod
  def parse_str(cls, config: str) -> Self:
    # The first line of output is always 'Performing autotest_lib import'
    # Followed by the output we care about.
    touch_device_values = config.splitlines()[1].split(" ")

    return cls(touch_device_values[0],
               NumberParser.positive_zero_int(touch_device_values[1]),
               NumberParser.positive_zero_int(touch_device_values[2]))

  def __str__(self) -> str:
    return f"{self.device_path} {self.x_max} {self.y_max}"

  def is_valid_tap_position(self, position: Point) -> bool:
    return (0 <= position.x and position.x <= self.x_max and 0 <= position.y and
            position.y <= self.y_max)


@dataclasses.dataclass(frozen=True)
class ChromeOSTouchEvent:
  touch_device: TouchDevice

  # The viewport in which the start and end positions lie.
  viewport: DisplayRectangle
  # The start position in terms of the device's screen resolution
  start_position: Point
  # The end position in terms of the device's screen resolution
  end_position: Point | None = None

  duration: dt.timedelta = dt.timedelta()

  # Touch event data recorded with evemu-record on a dedede.
  # This has been tested to work on dedede, brya, and volteer.
  # Some devices, however, may use a different x-y orientation
  # (such as kukui in landscape mode) and are not currently supported.
  _TAP_DOWN = """E: <time> 0003 0039 0
E: <time> 0003 0035 <x>
E: <time> 0003 0036 <y>
E: <time> 0001 014a 1
E: <time> 0003 0000 <x>
E: <time> 0003 0001 <y>
E: <time> 0000 0000 0
"""

  _TAP_POSITION = """E: <time> 0003 0035 <x>
E: <time> 0003 0036 <y>
E: <time> 0003 0000 <x>
E: <time> 0003 0001 <y>
E: <time> 0000 0000 0
"""

  _TAP_UP = """E: <time> 0003 0039 -1
E: <time> 0001 014a 0
E: <time> 0000 0000 0
"""

  # For swipes, simulate the touch panel updating the position 60 times a
  # second.
  # This was chosen arbitrarily, but should balance a realistic swipe action
  # with the size of the playback file that needs to be pushed to the device.
  _TOUCH_UPDATE_HERTZ = 60

  def __str__(self) -> str:
    # Not sure why, but evemu-playback does not like it when the event time
    # starts at 0.X
    current_event_time_seconds: float = 1.0
    playback_script: str = ""

    start_position: Point = self._rereference_to_touch_coordinates(
        self.viewport, self.start_position)

    playback_script += self._format_script_block(self._TAP_DOWN,
                                                 current_event_time_seconds,
                                                 start_position)

    # Shortcut for long taps
    if not self.end_position:
      current_event_time_seconds += self.duration.total_seconds()
      playback_script += self._format_script_block(self._TAP_UP,
                                                   current_event_time_seconds,
                                                   start_position)
      return playback_script

    end_position: Point = self._rereference_to_touch_coordinates(
        self.viewport, self.end_position)

    num_position_updates: int = round(self.duration.total_seconds() *
                                      self._TOUCH_UPDATE_HERTZ)
    assert num_position_updates > 0, "Choose a longer scroll duration."

    increment_distance_x = (end_position.x -
                            start_position.x) / num_position_updates
    increment_distance_y = (end_position.y -
                            start_position.y) / num_position_updates

    current_position_x: float = start_position.x
    current_position_y: float = start_position.y

    for _ in range(num_position_updates):
      current_event_time_seconds += 1.0 / self._TOUCH_UPDATE_HERTZ
      current_position_x += increment_distance_x
      current_position_y += increment_distance_y
      playback_script += self._format_script_block(
          self._TAP_POSITION, current_event_time_seconds,
          Point(round(current_position_x), round(current_position_y)))

    playback_script += self._format_script_block(self._TAP_UP,
                                                 current_event_time_seconds,
                                                 end_position)
    return playback_script

  def _rereference(self, original: int, original_max: int, new_max: int) -> int:
    return round(float(original / original_max) * new_max)

  def _rereference_to_touch_coordinates(self,
                                        original_viewport: DisplayRectangle,
                                        point: Point) -> Point:
    x = self._rereference(point.x, original_viewport.width,
                          self.touch_device.x_max)
    y = self._rereference(point.y, original_viewport.height,
                          self.touch_device.y_max)

    return Point(x, y)

  def _format_script_block(self, script_block: str, time: float,
                           position: Point) -> str:
    if not self.touch_device.is_valid_tap_position(position):
      raise ValueError(f"Cannot tap on out of bounds position: {position}")

    return script_block.replace("<x>", str(round(position.x))).replace(
        "<y>", str(round(position.y))).replace("<time>", f"{time:.6f}")


class ChromeOSInputActionRunner(DefaultActionRunner):
  """Custom ActionRunner for chromeOS devices."""

  def __init__(self) -> None:
    super().__init__()
    self._touch_device: TouchDevice | None = None
    self._mouse_process: subprocess.Popen | None = None

    atexit.register(self._kill_mouse_process)

  def _kill_mouse_process(self) -> None:
    if self._mouse_process:
      self._mouse_process.kill()
      self._mouse_process.wait()

  def click_touch(self, run: Run, action: i_action.ClickAction) -> None:
    if not self._touch_device:
      self._touch_device = self._setup_touch_device(run)

    with run.actions("ClickAction", measure=False) as actions:

      click_location, viewport = self._get_click_location(actions, action)

      if not click_location:
        return

      self._execute_touch_playback(
          run,
          ChromeOSTouchEvent(
              self._touch_device,
              viewport.native_screen,
              click_location,
              end_position=None,
              duration=action.duration))

      if action.verify:
        self.wait_for_element_impl(
            actions,
            selector=action.verify,
            timeout=action.timeout,
            check_element_rect=True)

  def click_mouse(self, run: Run, action: i_action.ClickAction) -> None:
    with run.actions("ClickAction", measure=False) as actions:

      click_location, viewport = self._get_click_location(actions, action)

      if not self._mouse_process:
        self._mouse_process = self._setup_mouse_process(
            run, viewport.native_screen.width, viewport.native_screen.height)

      if not click_location:
        return

      click_string: str = (f"{action.duration.total_seconds()}\n"
                           f"{click_location.x}\n"
                           f"{click_location.y}\n")

      assert self._mouse_process.stdin
      self._mouse_process.stdin.write(click_string.encode("utf-8"))
      self._mouse_process.stdin.flush()

      assert self._mouse_process.stdout
      output = int(self._mouse_process.stdout.readline())

      if output != 0:
        raise RuntimeError(f"Failed to perform click: {output}")

      if action.verify:
        self.wait_for_element_impl(
            actions,
            selector=action.verify,
            timeout=action.timeout,
            check_element_rect=True)

  def scroll_touch(self, run: Run, action: i_action.ScrollAction) -> None:
    if not self._touch_device:
      self._touch_device = self._setup_touch_device(run)

    with run.actions("ScrollAction", measure=False) as actions:

      viewport_info: ChromeOSViewportInfo = self._get_viewport_info(
          actions, action.selector, False)

      scroll_area: DisplayRectangle = viewport_info.browser_viewable

      total_scroll_distance = viewport_info.css_to_native_distance(
          action.distance)

      if action.selector:
        if not viewport_info.element_rect:
          if action.required:
            raise ElementNotFoundError(action.selector)
          return
        scroll_area = viewport_info.element_rect

      (scrollable_top, scrollable_bottom,
       max_swipe_distance) = scroll_area.get_scrollable_area()

      swipe_count = ceil(abs(total_scroll_distance) / max_swipe_distance)
      swipe_distance = abs(total_scroll_distance) / swipe_count
      swipe_duration = action.duration / swipe_count

      for _ in range(swipe_count):
        if total_scroll_distance > 0:
          # If scrolling down, the swipe should start at the bottom and end
          # above.
          y_start: int = scrollable_bottom
          y_end: int = round(scrollable_bottom - swipe_distance)

        else:
          # If scrolling up, the swipe should start at the top and end below.
          y_start = scrollable_top
          y_end = round(scrollable_top + swipe_distance)

        self._execute_touch_playback(
            run,
            ChromeOSTouchEvent(
                self._touch_device,
                viewport_info.native_screen,
                Point(scroll_area.middle.x, y_start),
                end_position=Point(scroll_area.middle.x, y_end),
                duration=swipe_duration))

  def text_input_keyboard(self, run: Run,
                          action: i_action.TextInputAction) -> None:
    if action.keyevent:
      raise ValueError("Keyevents are currently not supported on ChromeOS")

    browser_platform = run.browser_platform

    script = (SCRIPTS_DIR / "text_input.py").read_text()

    with browser_platform.NamedTemporaryFile() as script_file:
      browser_platform.write_text(script_file, script)
      typing_process: subprocess.Popen | None = None
      try:
        typing_process = browser_platform.popen(
            "python3", script_file, bufsize=0, stdin=subprocess.PIPE)
        typing_stdin = typing_process.stdin
        assert typing_stdin, "Got no stdin"

        self._rate_limit_keystrokes(
            run, action,
            lambda run, actions, text: typing_stdin.write(text.encode("utf-8")))
      finally:
        if typing_stdin:
          typing_stdin.close()
        if typing_process:
          typing_process.wait(timeout=action.timeout.total_seconds())

  def _get_click_location(
      self, actions: Actions, action: i_action.ClickAction
  ) -> tuple[Optional[Point], ChromeOSViewportInfo]:
    if selector_config := action.position.selector:
      if selector_config.wait:
        self.wait_for_element_impl(
            actions,
            selector=selector_config.selector,
            timeout=action.timeout,
            scroll_into_view=selector_config.scroll_into_view,
            check_element_rect=True,
            required=selector_config.required)

      viewport_info = self._get_viewport_info(actions, selector_config.selector,
                                              selector_config.scroll_into_view)
      element_rect = viewport_info.element_rect
      if not element_rect:
        if selector_config.required:
          raise ElementNotFoundError(selector_config.selector)
        return (None, viewport_info)
      self.add_failure_screenshot_annotation(
          ScreenshotRectAnnotation(
              label=selector_config.selector, rect=element_rect))
      click_location = element_rect.middle
      self.add_failure_screenshot_annotation(
          ScreenshotPointAnnotation(label="click", point=click_location))
      return (click_location, viewport_info)
    if coordinates_config := action.position.coordinates:
      viewport_info = self._get_viewport_info(actions, None, False)
      click_location = coordinates_config.point()
      self.add_failure_screenshot_annotation(
          ScreenshotPointAnnotation(label="click", point=click_location))
      return (click_location, viewport_info)
    raise RuntimeError("Missing coordinates")

  def _get_viewport_info(
      self,
      actions: Actions,
      selector: Optional[str],
      scroll_into_view: bool = False) -> ChromeOSViewportInfo:

    script = ""
    if selector:
      selector, script = self.get_selector_script(selector)

    script += (SCRIPTS_DIR / "get_window_positions.js").read_text()

    (found_element, pixel_ratio, outer_width, inner_width, inner_height,
     screen_width, screen_height, avail_width, avail_height, screen_x, screen_y,
     element_left, element_top, element_width, element_height) = actions.js(
         script, arguments=[selector, scroll_into_view])

    element_rect: DisplayRectangle | None = None

    if found_element:
      element_rect = DisplayRectangle(
          Point(element_left, element_top), element_width, element_height)

    viewport_info: ChromeOSViewportInfo = ChromeOSViewportInfo(
        device_pixel_ratio=pixel_ratio,
        window_outer_width=outer_width,
        window_inner_width=inner_width,
        window_inner_height=inner_height,
        screen_width=screen_width,
        screen_height=screen_height,
        screen_avail_width=avail_width,
        screen_avail_height=avail_height,
        window_offset_x=screen_x,
        window_offset_y=screen_y,
        element_rect=element_rect)

    return viewport_info

  def _query_touch_device(self, run: Run) -> str:
    try:
      with (SCRIPTS_DIR / "query_touch_device.py").open() as file:
        return run.browser_platform.sh_stdout("python3", "-", stdin=file)
    except Exception as e:
      raise RuntimeError(
          "Failed to query touchscreen information from device.") from e

  def _setup_touch_device(self, run: Run) -> TouchDevice:
    touch_device_output = self._query_touch_device(run)
    return TouchDevice.parse_str(touch_device_output)

  def _setup_mouse_process(self, run: Run, screen_width: int,
                           screen_height: int) -> subprocess.Popen:
    browser_platform = run.browser_platform
    script = (SCRIPTS_DIR / "mouse.py").read_text()

    with browser_platform.NamedTemporaryFile() as script_file:
      browser_platform.write_text(script_file, script)

      mouse_process = browser_platform.popen(
          "python3",
          script_file,
          str(screen_width),
          str(screen_height),
          stdin=subprocess.PIPE,
          stdout=subprocess.PIPE)

      if mouse_process.poll() is not None:
        raise RuntimeError("Failed to start ChromeOS mouse process.")

      assert mouse_process.stdout
      output = int(mouse_process.stdout.readline())

      if output != 0:
        raise RuntimeError(f"Failed to start mouse process: {output}")

      return mouse_process

  def _execute_touch_playback(self, run: Run,
                              touch_event: ChromeOSTouchEvent) -> None:
    # Ideally the touch event data could just be sent to |input| of evemu-play,
    # but after a lot of testing, evemu-play *only* behaves when input is
    # redirected from a file such as with:
    # 'evemu-play touch-device < input-file.txt'
    # Using a pipe to redirect the input *does not work*:
    # 'cat input-file.txt | evemu-play touch-device'

    # Because of this weird behavior, create a temp file on the device first
    # that contains the touch events.
    assert self._touch_device, "Missing touch_device"

    touch_event_cmds = str(touch_event)

    browser_platform = run.browser_platform

    with browser_platform.NamedTemporaryFile() as playback_file:
      browser_platform.write_text(playback_file, touch_event_cmds)
      # Then run evemu-play with the input redirected from the temp file.
      run.browser_platform.sh(
          f"evemu-play --insert-slot0 "
          f"{shlex.quote(self._touch_device.device_path)} < "
          f"{playback_file}",
          shell=True)
