# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import base64
from typing import TYPE_CHECKING, Any

from crossbench.helper.wait import WaitRange

if TYPE_CHECKING:
  from types import ModuleType

  from selenium import webdriver
  from selenium.webdriver.remote.websocket_connection import \
      WebSocketConnection


class DevToolsTracer:

  def __init__(self, driver: webdriver.Remote) -> None:
    module_and_socket = driver.start_devtools()
    self._devtools: ModuleType = module_and_socket[0]
    self._websocket: WebSocketConnection = module_and_socket[1]
    # It's a devtools.io.StreamHandle. The devtools module is imported
    # dynamically so adding a type annotation is infeasible.
    self._out_stream: Any | None = None

  def start(self) -> None:
    config = self._devtools.tracing.TraceConfig()
    config.included_categories = [
        "devtools.timeline",
        "v8.execute",
        "disabled-by-default-devtools.timeline",
        "disabled-by-default-devtools.timeline.frame",
        "toplevel",
        "blink.console",
        "blink.user_timing",
        "latencyInfo",
        "disabled-by-default-devtools.timeline.stack",
        "disabled-by-default-v8.cpu_profiler",
    ]
    self._websocket.on(self._devtools.tracing.TracingComplete,
                       self._on_tracing_complete)
    self._websocket.execute(
        self._devtools.tracing.start(
            transfer_mode="ReturnAsStream",
            trace_config=config,
            stream_format=self._devtools.tracing.StreamFormat.PROTO))

  def end(self) -> bytes:
    self._websocket.execute(self._devtools.tracing.end())
    for _ in WaitRange().wait_with_backoff():
      if self._out_stream:
        break
    output = bytearray()
    while True:
      base64_encoded, chunk, eof = self._websocket.execute(
          self._devtools.io.read(self._out_stream))
      if chunk:
        if base64_encoded:
          output += base64.b64decode(chunk)
        else:
          output += chunk.encode("utf-8")
      if eof:
        return output

  def _on_tracing_complete(self, event) -> None:
    self._out_stream = event.stream
