# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

from typing import TYPE_CHECKING, Type

from typing_extensions import override

from crossbench.browsers.attributes import BrowserAttributes
from crossbench.browsers.chromium.devtools import DevToolsClient
from crossbench.probes.json import JsonResultProbe, JsonResultProbeContext
from crossbench.probes.probe_error import ProbeMissingDataError

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.runner.actions import Actions
  from crossbench.runner.run import Run
  from crossbench.types import Json


class VideoAccelerationProbe(JsonResultProbe):
  """
  Chromium-only probe to detect if hardware video acceleration is used.
  """
  NAME = "video_acceleration"

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    self.expect_browser(browser, BrowserAttributes.CHROMIUM_BASED)

  @override
  def attach(self, browser: Browser) -> None:
    self.expect_browser(browser, BrowserAttributes.CHROMIUM_BASED)
    super().attach(browser)

  @override
  def get_context_cls(self) -> Type[VideoAccelerationProbeContext]:
    return VideoAccelerationProbeContext

  @property
  @override
  def result_path_name(self) -> str:
    return "cb.video_acceleration.json"


class VideoAccelerationProbeContext(
    JsonResultProbeContext[VideoAccelerationProbe]):

  def __init__(self, probe: VideoAccelerationProbe, run: Run):
    super().__init__(probe, run)
    self._devtools_client: DevToolsClient | None = None
    self._is_hw_accelerated: bool | None = None

  def _get_devtools_client(self) -> DevToolsClient:
    if not self._devtools_client:
      self._devtools_client = DevToolsClient(
          platform=self.browser_platform,
          requested_local_port=0,
          remote_devtools_identifier="chrome_devtools_remote")
    return self._devtools_client

  def _get_page_target_id(self, client: DevToolsClient) -> str | None:
    success, targets = client.send_command({
        "id": 0,
        "method": "Target.getTargets"
    })
    if not success:
      raise RuntimeError("Failed to query target")
    if not targets or "targetInfos" not in targets.get("result", {}):
      return None
    for target in targets["result"]["targetInfos"]:
      if target["type"] == "page":
        return target["targetId"]
    return None

  def _is_hw_acceleration_undetermined(self) -> bool:
    return self._is_hw_accelerated is None

  def _process_media_event(self, response: dict) -> None:
    if response.get("method") == "Media.playerPropertiesChanged":
      event = response.get("params", {})
      if not self._is_hw_acceleration_undetermined():
        return
      for prop in event.get("properties", []):
        if prop["name"] == "kIsPlatformVideoDecoder":
          self._is_hw_accelerated = prop["value"] == "true"
          break

  @override
  def stop(self) -> None:
    # We override stop() to perform the data collection.
    # The base class stop() will then call to_json() to get the data.
    client = self._get_devtools_client()
    with client:

      target_id = self._get_page_target_id(client)
      if not target_id:
        raise RuntimeError("Could not find page target")

      # Attach to the page target
      _, response = client.send_command({
          "id": 1,
          "method": "Target.attachToTarget",
          "params": {
              "targetId": target_id,
              "flatten": True,
          }
      })
      session_id = response.get("params", {}).get("sessionId")
      if not session_id:
        raise ValueError(f"Could not find 'sessionId' in response: {response}")

      client.dispatch_command({
          "sessionId": session_id,
          "id": 2,
          "method": "Media.enable"
      })

      # Listen for events until either no event is received within timeout, or
      # hw_acceleration status is gathered.
      success = client.poll_for_response(
          condition_fn=self._is_hw_acceleration_undetermined,
          process_fn=self._process_media_event,
      )
      if not success:
        raise RuntimeError("Error polling for media properties")

      client.send_command({
          "sessionId": session_id,
          "id": 3,
          "method": "Media.disable"
      })
    super().stop()

  @override
  def to_json(self, actions: Actions) -> Json:
    del actions  # Unused
    if self._is_hw_accelerated is None:
      raise ProbeMissingDataError(
          "Could not determine hardware acceleration status.")
    return {"is_hw_accelerated": self._is_hw_accelerated}
