# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import contextlib
import email.parser
import http.server
import json
import logging
import os
import threading
from typing import (TYPE_CHECKING, Final, Iterator, Mapping, Optional, Type,
                    TypeVar)

from immutabledict import immutabledict
from typing_extensions import override

from crossbench import exception
from crossbench.network.base import Network
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  from crossbench import plt
  from crossbench.network.traffic_shaping.base import TrafficShaper
  from crossbench.path import LocalPath
  from crossbench.runner.groups.session import BrowserSessionRunGroup

_DEFAULT_HOST = "localhost"
_DEFAULT_PORT = 8000

# List of known headers that are served by the default HTTPServer and might
# be accidentally overridden by provided extra headers.
_CONFLICTING_EXTRA_HEADERS: Final[frozenset[str]] = frozenset(
    map(lambda header: header.lower(),
        ("Content-Type", "Content-Length", "Last-Modified", "Server", "Date",
         "Connection", "Location")))

# Enable cross original isolation for high-precision timers.
# This can be easily override by profiling a custom HEADER.txt file in the
# served directory.
_DEFAULT_HEADERS: Final[immutabledict[str, str]] = immutabledict({
    "Cross-Origin-Opener-Policy": "same-origin",
    "Cross-Origin-Embedder-Policy": "require-corp"
})

class CustomHeadersRequestHandler(http.server.SimpleHTTPRequestHandler):

  @classmethod
  def bind(
      cls,
      server_dir: LocalPath,
      extra_headers: Mapping[str, str],
  ) -> Type[http.server.SimpleHTTPRequestHandler]:
    # Use a temporary class to bind arguments.
    class BoundDirectoryRequestHandler(cls):  # type: ignore

      def __init__(self, *args, **kwargs):
        super().__init__(
            *args,
            directory=os.fspath(server_dir),
            extra_headers=extra_headers,
            **kwargs)

    return BoundDirectoryRequestHandler

  def __init__(self,
               *args,
               directory: Optional[str] = None,
               extra_headers: Optional[Mapping[str, str]] = None,
               **kwargs) -> None:
    self._extra_headers: immutabledict[str, str] = (
        immutabledict(extra_headers) if extra_headers else immutabledict())
    super().__init__(*args, directory=directory, **kwargs)

  def end_headers(self) -> None:
    if self._extra_headers:
      self._send_custom_headers()
    super().end_headers()

  def _send_custom_headers(self) -> None:
    for key, value in self._extra_headers.items():
      self.send_header(key, value)


LocalFileNetworkT = TypeVar("LocalFileNetworkT", bound="LocalFileNetwork")

class LocalFileNetwork(Network):

  def __init__(self,
               path: LocalPath,
               url: Optional[str],
               traffic_shaper: Optional[TrafficShaper] = None,
               browser_platform: Optional[plt.Platform] = None) -> None:
    super().__init__(traffic_shaper, browser_platform)
    self._path = path
    self._host, self._port = self._parse_url(url)
    # TODO: support custom headers via command line
    self._extra_headers: immutabledict[str, str] = self._try_parse_headers()
    if self._extra_headers:
      self._validate_extra_headers()

  @property
  @override
  def is_local_file_server(self) -> bool:
    return True

  @property
  def path(self) -> LocalPath:
    return self._path

  def _parse_url(self, url: Optional[str]) -> tuple[str, int]:
    host: str = _DEFAULT_HOST
    port: int = _DEFAULT_PORT
    if not url:
      return host, port
    parsed_url = ObjectParser.url(url)
    if parsed_url.hostname:
      host = parsed_url.hostname
    if parsed_url.port is not None:
      port = parsed_url.port
    return host, port

  def _try_parse_headers(self) -> immutabledict[str, str]:
    for name in ("HEADERS", "HEADERS.txt"):
      header_file = self._path / name
      if header_file.exists():
        return self._read_headers_file(header_file)
    return _DEFAULT_HEADERS

  def _read_headers_file(self,
                         header_file: LocalPath) -> immutabledict[str, str]:
    # Reuse python's email message library to parse headers
    message = email.parser.BytesParser().parsebytes(header_file.read_bytes())
    return immutabledict(message)

  def _validate_extra_headers(self) -> None:
    for key, value in self._extra_headers.items():
      if key.lower() in _CONFLICTING_EXTRA_HEADERS:
        logging.error(
            "BROWSER Network: Extra header overrides server defaults: '%s: %s'",
            key, value)

  @contextlib.contextmanager
  @override
  def open(self: LocalFileNetworkT,
           session: BrowserSessionRunGroup) -> Iterator[LocalFileNetworkT]:
    with super().open(session):
      with self._open_local_file_server():
        # TODO: properly hook up traffic shaper for the local http server
        with self._traffic_shaper.open(self, session):
          with self._forward_ports(session):
            yield self

  @contextlib.contextmanager
  def _open_local_file_server(self):
    # TODO: write request log file to session results folder.
    # TODO: support  https server using SSLContext.wrap_socket(httpd.socket)
    request_handler_cls = CustomHeadersRequestHandler.bind(
        self._path, self._extra_headers)
    with exception.annotate(
        f"Starting fileserver on {self.host}:{self.http_port}"):
      server = http.server.ThreadingHTTPServer((self._host, self._port),
                                               request_handler_cls)
    with self._server_thread(server):
      logging.info("%s custom host=%s, port=%s",
                   type(self).__name__, self.host, self.http_port)
      yield

  @contextlib.contextmanager
  def _server_thread(self, server: http.server.HTTPServer) -> Iterator[None]:
    with server:
      server_thread = threading.Thread(target=server.serve_forever)
      server_thread.daemon = True
      server_thread.start()
      self._port = server.server_port
      try:
        yield
      finally:
        server.shutdown()
        server_thread.join()

  @contextlib.contextmanager
  def _forward_ports(self, session: BrowserSessionRunGroup) -> Iterator:
    browser_platform = session.browser_platform
    ports = browser_platform.ports
    if browser_platform.is_remote:
      logging.info("REMOTE PORT FORWARDING: %s <= %s", self.host_platform,
                   browser_platform)
      # TODO: make ports configurable
      ports.reverse_forward(self._port, self._port)
    try:
      yield
    finally:
      if browser_platform.is_remote:
        ports.stop_reverse_forward(self._port)

  @property
  @override
  def http_port(self) -> Optional[int]:
    return self._port

  @property
  @override
  def https_port(self) -> Optional[int]:
    # TODO: support https locally
    return None

  @property
  @override
  def host(self) -> Optional[str]:
    return self._host

  def __str__(self) -> str:
    extra_headers_str = ""
    if self._extra_headers:
      formatted_headers = json.dumps(dict(self._extra_headers))
      extra_headers_str = f" extra_headers={formatted_headers}"
    return (f"LOCAL(path={self._path}, "
            f"speed={self.traffic_shaper}{extra_headers_str})")
