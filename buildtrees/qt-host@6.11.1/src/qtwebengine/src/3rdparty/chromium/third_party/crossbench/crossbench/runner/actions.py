# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import contextlib
import datetime as dt
import logging
import sys
import time as py_time
from typing import TYPE_CHECKING, Any, Callable, Optional, Sequence, Type

from crossbench.action_runner.action.enums import ReadyState
from crossbench.cli import ui
from crossbench.helper.durations import TimeScope
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  from types import TracebackType

  from crossbench import plt
  from crossbench.browsers.browser import Browser
  from crossbench.exception import ExceptionAnnotationScope
  from crossbench.helper.wait import WaitRange
  from crossbench.runner.run import Run
  from crossbench.runner.runner import Runner
  from crossbench.runner.timing import AnyTimeUnit, Timing


def _default_success_condition(js_result: Any) -> bool:
  if js_result is True:
    return True
  ObjectParser.bool(js_result, strict=True)
  return False

class Actions(TimeScope):

  _max_end_datetime: dt.datetime

  def __init__(
      self,
      message: str,
      run: Run,
      runner: Optional[Runner] = None,
      browser: Optional[Browser] = None,
      verbose: bool = False,
      measure: bool = True,
      timeout: dt.timedelta = dt.timedelta()) -> None:
    assert message, "Actions need a name"
    super().__init__(message)
    self._exception_annotation: ExceptionAnnotationScope = run.exceptions.info(
        f"Action: {message}")
    self._run: Run = run
    self._browser: Browser = browser or run.browser
    self._runner: Runner = runner or run.runner
    self._is_active: bool = False
    self._verbose: bool = verbose
    self._measure = measure
    if timeout:
      self._max_end_datetime = min(dt.datetime.now() + timeout,
                                   run.max_end_datetime())
    else:
      self._max_end_datetime = run.max_end_datetime()

  @property
  def timing(self) -> Timing:
    return self._runner.timing

  @property
  def platform(self) -> plt.Platform:
    return self._run.browser_platform

  def __enter__(self) -> Actions:
    self._exception_annotation.__enter__()
    super().__enter__()
    self._is_active = True
    logging.debug("Action begin: %s", self._message)
    if self._verbose:
      logging.info(self._message.ljust(30))
    else:
      # Print message that doesn't overlap with helper.Spinner
      sys.stdout.write(f"   {self._message.ljust(30)}\r")
    return self

  def __exit__(self, exc_type: Optional[Type[BaseException]],
               exc_value: Optional[BaseException],
               exc_traceback: Optional[TracebackType]) -> None:
    self._is_active = False
    self._exception_annotation.__exit__(exc_type, exc_value, exc_traceback)
    super().__exit__(exc_type, exc_value, exc_traceback)
    logging.debug("Action end: %s", self._message)
    if self._measure:
      self._run.durations[f"actions-duration {self.message}"] = self.duration

  def _assert_is_active(self) -> None:
    assert self._is_active, "Actions have to be used in a with scope"

  def current_window_id(self) -> str:
    return self._browser.current_window_id()

  def switch_window(self, window_id: str) -> None:
    self._browser.switch_window(window_id)

  def js(self,
         js_code: str,
         timeout: AnyTimeUnit = 10,
         absolute_time: bool = False,
         arguments: Sequence[object] = (),
         **kwargs) -> Any:
    self._assert_is_active()
    assert js_code, "js_code must be a valid JS script"
    if kwargs:
      js_code = js_code.format(**kwargs)
    delta = self.timing.timeout_timedelta(timeout, absolute_time)
    return self._browser.js(js_code, delta, arguments=arguments)

  def wait_js_condition(
      self,
      js_code: str,
      min_interval: AnyTimeUnit,
      timeout: AnyTimeUnit,
      delay: AnyTimeUnit = 0,
      absolute_time: bool = False,
      arguments: Sequence[object] = (),
      success_condition: Callable[[Any], bool] = _default_success_condition
  ) -> None:
    """
    Runs the `js_code` at a regular interval until either the `timeout` is
    reached or the return value is true. The poll interval is exponentially
    increasing with the WaitRange's default factor:
    1. sleep for `delay`,                    check `js_code`
    2. sleep for `min_interval`,             check `js_code`
    2. sleep for `min_interval * 1.01 ** 1`, check `js_code`
    ...
    N. sleep for `min_interval * 1.01 ** N`, check `js_code`
    """
    wait_range : WaitRange = self._run.wait_range(min_interval, timeout, delay)
    assert "return" in js_code, (
        f"Missing return statement in js-wait code: {js_code}")
    for _, _, time_left in wait_range.wait_with_backoff():
      time_units = self.timing.units(time_left, absolute_time)
      result = self.js(
          js_code,
          timeout=time_units,
          absolute_time=absolute_time,
          arguments=arguments)
      if success_condition(result):
        return

  def wait_for_ready_state(self, ready_state: ReadyState,
                           timeout: dt.timedelta) -> None:
    # Make sure we also finish if readyState jumps directly
    # from "loading" to "complete"
    self.wait_js_condition(
        f"""
          let state = document.readyState;
          return state === '{ready_state}' || state === "complete";
        """, 0.2, timeout)

  def show_url(
      self,
      url: str,
      target: Optional[str] = None,
      ready_state: ReadyState = ReadyState.ANY,
      timeout: dt.timedelta = dt.timedelta()
  ) -> None:
    self._assert_is_active()
    if target and target in ("_blank", "_parent", "_top"):
      # TODO: use target in the driver instead.
      self.js(f"window.open('{url}','{target}');")
    else:
      if target not in (None, "_self", "_new_tab", "_new_window"):
        raise ValueError(f"Invalid target: {target}")
      self._browser.show_url(url, target=target)

    if ready_state != ReadyState.ANY:
      self.wait_for_ready_state(ready_state, timeout)

  def current_url(self) -> str:
    return self.js("return document.URL;")

  def wait(self,
           time: AnyTimeUnit = dt.timedelta(seconds=1),
           absolute_time: bool = False) -> None:
    """"Wait for a fixed timeout. If you need to wait until a certain
    timeout passed independent of a previous action, use wait_until(...).

    | action 2s | wait 2s | => total time is 4s
    | action 4s | wait 2s | => total time is 6s
    """
    delta: dt.timedelta = self.timing.timeout_timedelta(time, absolute_time)
    with ui.countdown(delta):
      self._assert_is_active()
      self._runner.wait(time, absolute_time=absolute_time)

  @contextlib.contextmanager
  def wait_until(self,
                 timeout: AnyTimeUnit = dt.timedelta(seconds=1),
                 absolute_time: bool = False):
    """Wait until the given timeout elapsed.
    Unlike wait(...), this takes into account the time spent in the the
    wrapped block.

    | wait_until 6s | action 2s | => total time is 6s
    | wait_until 6s | action 4s | => total time is 6s
    """
    self._assert_is_active()
    if not timeout:
      # No wait necessary, don't show a warning.
      yield
      return
    delta: dt.timedelta = self.timing.timeout_timedelta(timeout, absolute_time)
    start_time: float = py_time.time()
    end_time: float = start_time + delta.total_seconds()
    with ui.countdown(delta):
      yield
      time_left = end_time - py_time.time()
      if time_left > 0:
        self._runner.wait(time_left, absolute_time=True)
      else:
        run_duration = dt.timedelta(seconds=py_time.time() - start_time)
        logging.info(
            "Action took longer (%s) than expected action duration (%s).",
            run_duration, delta)
