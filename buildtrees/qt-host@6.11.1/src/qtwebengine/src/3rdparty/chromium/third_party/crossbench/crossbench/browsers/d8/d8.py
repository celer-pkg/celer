# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import json
import logging
import urllib.request
from typing import TYPE_CHECKING, Any, Optional, Sequence, cast

from typing_extensions import override

from crossbench import exception
from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.browser import Browser
from crossbench.browsers.d8.shell import D8Shell
from crossbench.browsers.d8.url_mapper import D8URLMapper, DummyURLMapper
from crossbench.browsers.d8.version import D8Version
from crossbench.browsers.viewport import Viewport
from crossbench.flags.chrome import ChromeFlags
from crossbench.network.local_file_server import LocalFileNetwork

if TYPE_CHECKING:
  import datetime as dt

  import crossbench.path as pth
  from crossbench.browsers.settings import Settings
  from crossbench.flags.base import FlagsData
  from crossbench.flags.js_flags import JSFlags
  from crossbench.runner.groups.session import BrowserSessionRunGroup

class D8(Browser):

  @classmethod
  @override
  def type_name(cls) -> str:
    return "d8"

  @classmethod
  @override
  def attributes(cls) -> BrowserAttributes:
    return BrowserAttributes.D8 | BrowserAttributes.CHROMIUM_BASED

  @override
  def __init__(self,
               label: str,
               path: pth.AnyPath | None = None,
               settings: Settings | None = None) -> None:
    super().__init__(label, path, settings)
    if not self.network.is_local_file_server:
      raise RuntimeError("D8 wrapper only works with --local-file-server"
                         f"but got {self.network} network.")
    self._d8_shell: D8Shell | None = None
    self._url_mapper: D8URLMapper = DummyURLMapper(self)

  @property
  def network(self) -> LocalFileNetwork:
    return cast(LocalFileNetwork, super().network)

  @property
  def d8_path(self) -> pth.LocalPath:
    return self.platform.local_path(self.app_path)

  @override
  def _setup_cache_dir(self) -> Optional[pth.AnyPath]:
    pass

  @override
  def _extract_version(self) -> D8Version:
    # Some d8 versions don't support --version
    shell = D8Shell(self.platform, self.d8_path)
    version: str = shell.version
    shell.quit()
    return D8Version.parse(version)

  @property
  def viewport(self) -> Viewport:
    return Viewport.HEADLESS

  @viewport.setter
  def viewport(self, value: Viewport) -> None:
    raise NotImplementedError("Cannot set viewport")

  def user_agent(self) -> str:
    return "V8"

  @classmethod
  @override
  def default_flags(cls,
                    initial_data: FlagsData = None,
                    milestone: int = 0) -> ChromeFlags:
    del milestone
    return ChromeFlags(initial_data)

  @property
  @override
  def js_flags(self) -> JSFlags:
    return cast(ChromeFlags, self._flags).js_flags

  def start(self, session: BrowserSessionRunGroup) -> None:
    super().start(session)
    js_flags_copy = self.js_flags.copy()
    js_flags_copy.update(session.extra_js_flags)
    self._log_browser_start(tuple(js_flags_copy))

    self._url_mapper = D8URLMapper.create(self, session)
    self._d8_shell = D8Shell(
        self.platform,
        self.d8_path,
        flags=list(js_flags_copy),
        cwd=self.network.path)

    self._pid = self._d8_shell.pid
    self._is_running = True
    self._install_d8_mocks()

  def _install_d8_mocks(self) -> None:
    with exception.annotate("D8 setup"):
      if shell := self._d8_shell:
        if setup_file := self._url_mapper.setup_file:
          shell.load(setup_file)

  def force_quit(self) -> None:
    if not self._is_running:
      return
    logging.info("Browser.force_quit()")
    if d8_shell := self._d8_shell:
      d8_shell.quit()
    self._is_running = False

  @override
  def js(
      self,
      script: str,
      timeout: Optional[dt.timedelta] = None,
      arguments: Sequence[object] = ()
  ) -> Any:
    logging.debug("JS: %s", script)
    args_str: str = json.dumps(arguments)
    script = """JSON.stringify((function exceptionWrapper(){
        try {
          return [
            (function(...arguments){
              %(script)s
            }).apply(globalThis, %(args_str)s),
            true];
        } catch(e) {
          return [e + "", false];
        }
      })());""" % {
        "script": script,
        "args_str": args_str
    }
    result, is_success = "Not started", False
    if d8_shell := self._d8_shell:
      json_result: str = d8_shell.execute(script, eval=True, timeout=timeout)
      logging.debug("D8 Result: %s", json_result)
      # D8 always adds double quotes for every string
      if json_result[0] != '"' or json_result[-1] != '"':
        logging.debug("D8: JSON results has no double quotes")
        return None
      unquoted = json_result.strip()[1:-1]
      json_decoded = json.loads(unquoted)
      assert len(json_decoded) == 2, (
          f"Expectedtuple[Any, Any], got {type(json_decoded)}: {json_decoded}")
      result, is_success = json_decoded
    if not is_success:
      raise RuntimeError(f"D8 JS Exception: {result}")
    return result

  @override
  def show_url(self, url: str, target: Optional[str] = None) -> None:
    if url.startswith("data:text/html;"):
      self._print_url(url)
      return
    if file_path := self._url_mapper.lookup(url):
      if d8_shell := self._d8_shell:
        result = d8_shell.load(file_path)
        logging.debug("D8 Result: %s", result)
        return
    raise RuntimeError(f"D8 unsupported URL: {url}")

  def _print_url(self, data_url: str) -> None:
    logging.debug("D8: SKIPPING data url")
    with urllib.request.urlopen(data_url) as response:
      info = response.info()
      charset = info.get_content_charset() or "utf-8"
      data_bytes = response.read()
      data_content = data_bytes.decode(charset)
      logging.info(data_content)
