# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import asyncio
import enum
import json
import logging
import os
import secrets
import shlex
import sys
import tempfile
from typing import TYPE_CHECKING, Any, Coroutine, Optional

from websockets import server as websockets

from crossbench import path as pth
from crossbench import plt
from crossbench.helper.state import BaseState, StateMachine

if TYPE_CHECKING:
  import argparse
  from asyncio.subprocess import Process

  from crossbench.plt.types import ListCmdArgs
  from crossbench.types import JsonDict


CROSSBENCH_ROOT: pth.LocalPath = pth.LocalPath(__file__).parents[4]


@enum.unique
class State(BaseState):
  CONNECTED = enum.auto()
  RUNNING = enum.auto()


@enum.unique
class Response(enum.StrEnum):
  STATUS = "status"
  OUTPUT = "output"


class AuthenticationError(ValueError):
  pass


class CrossbenchDevToolsRecorderProxy:
  DEFAULT_PORT = 44645

  @classmethod
  def add_cli_parser(cls, subparsers) -> argparse.ArgumentParser:
    parser = subparsers.add_parser(
        "devtools-recorder-proxy",
        aliases=["devtools"],
        help=("Starts a local server to communicate with the "
              "DevTools Recorder extension."))
    parser.add_argument(
        "--disable-token-authentication",
        dest="use_auth_token",
        default=True,
        action="store_false",
        help=("Disable token-based authentication. "
              "Unsafe, only use for local development."))
    return parser

  @classmethod
  def run_subcommand(cls, args: argparse.Namespace) -> None:
    instance: CrossbenchDevToolsRecorderProxy = cls(
        use_auth_token=args.use_auth_token)
    instance.run()

  _websocket: websockets.WebSocketServerProtocol

  def __init__(self, use_auth_token: bool = True) -> None:
    self._token: str = secrets.token_hex(16)
    self._use_auth_token: bool = use_auth_token
    self._print_cmd_output: bool = False
    self._port: int = self.DEFAULT_PORT
    self._state = StateMachine(State.CONNECTED)
    self._crossbench_task: asyncio.Task | None = None
    self._crossbench_process: Process | None = None
    self._tmp_json = pth.LocalPath(
        tempfile.mkdtemp("crossbench_proxy")) / "devtools_recorder.json"

  def run(self) -> None:
    asyncio.run(self.run_server())

  async def run_server(self) -> None:
    try:
      serve = websockets.serve(self.handler, "localhost", self.DEFAULT_PORT)
    except Exception as e:  # pylint: disable=broad-except
      logging.exception(e)
      serve = websockets.serve(self.handler, "localhost")
    async with serve as server:
      self._port = list(server.sockets)[0].getsockname()[1]
      logging.info("#" * 80)
      logging.info("#" * 80)
      logging.info("# Crossbench DevTools Recorder Replay Server Started")
      logging.info("#")
      if self._port != self.DEFAULT_PORT:
        logging.warning("# Non-default port!")
      logging.info("# PORT:  %s", self._port)
      if not self._use_auth_token:
        logging.warning("# Token authentication has been disabled!")
      logging.info("# TOKEN: %s", self._token)
      logging.info("#")
      logging.info("#" * 80)
      logging.info("#" * 80)
      await asyncio.Future()  # run forever

  async def handler(self,
                    websocket: websockets.WebSocketServerProtocol) -> None:
    self._websocket = websocket
    async for message in websocket:
      await self._send_message(self._handle_message(message))

  async def _send_message(
      self, coroutine: Coroutine[Any, Any, Optional[tuple[Response,
                                                          Any]]]) -> None:
    response: JsonDict = {"success": False, "payload": None, "error": None}
    try:
      result: Optional[tuple[Response, Any]] = await coroutine
      response["success"] = True
      if result:
        response_type, payload = result
        response["payload"] = payload
        response["type"] = response_type.value
    except Exception as e:  # pylint: disable=broad-except
      logging.exception(e)
      response["error"] = str(type(e).__name__)
    try:
      response_json = json.dumps(response)
    except Exception as e:  # pylint: disable=broad-except
      logging.exception(e)
      response["success"] = False
      response["error"] = "Failed to encode message"
      response["payload"] = None
      response_json = json.dumps(response)
    logging.debug("SEND Response: %s", response_json)
    await self._websocket.send(response_json)

  async def _handle_message(
      self, message: bytearray | bytes | str) -> Optional[tuple[Response, Any]]:
    logging.debug("RECEIVE Message: %s", message)
    try:
      payload: dict[str, Any] = json.loads(message)
    except json.JSONDecodeError as e:
      logging.error("Could not parse JSON response: %s", e)
      raise e
    if self._use_auth_token:
      payload_token = payload["token"]
      if payload_token != self._token:
        logging.error("Invalid request token: %s", payload_token)
        raise AuthenticationError("Invalid Token")
    command = payload["command"]
    args = payload.get("args", None)
    if command == "run":
      return await self._run_command(args)
    if command == "stop":
      return await self._stop_command()
    if command == "status":
      return await self._status_command()
    logging.error("Unknown command: %s", command)
    return None

  async def _stop_command(self) -> tuple[Response, str]:
    if process := self._crossbench_process:
      logging.info("# CROSSBENCH COMMAND: KILL")
      plt.PLATFORM.terminate_gracefully(process)
    self._state.transition(State.CONNECTED, State.CONNECTED, to=State.CONNECTED)
    return await self._status_command()

  async def _run_command(self, args) -> tuple[Response, str]:
    self._state.transition(State.CONNECTED, to=State.RUNNING)
    assert self._crossbench_process is None
    cb_path = CROSSBENCH_ROOT / "cb.py"
    os.environ["PYTHONUNBUFFERED"] = "1"
    cmd: ListCmdArgs = []
    if args.get("cmd") == "--help":
      cmd = ["load", "--help"]
      self._print_cmd_output = False
    elif args.get("cmd") == "describe probes":
      cmd = ["describe", "probes"]
      self._print_cmd_output = False
    else:
      self._print_cmd_output = True
      with self._tmp_json.open("w", encoding="utf-8") as f:
        json.dump(args["json"], f)
      assert self._tmp_json.exists(), f"{self._tmp_json} does not exist."
      assert cb_path.exists(), f"{cb_path} does not exist."
      cmd = [
          "load", "--env-validation=warn", "--verbose",
          f"--devtools-recorder={self._tmp_json.absolute()}",
          *shlex.split(args.get("cmd"))
      ]
    logging.info("CROSSBENCH COMMAND: %s", cmd)
    self._crossbench_process = await asyncio.create_subprocess_exec(
        cb_path.absolute(),
        *cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE)
    self._crossbench_task = asyncio.create_task(self._wait_for_crossbench())
    return await self._status_command()

  async def _send_output(
      self, stdout_str: Optional[str],
      stderr_str: Optional[str]) -> Optional[tuple[Response, dict[str, str]]]:
    if self._state != State.RUNNING:
      return None
    if self._print_cmd_output:
      sys.stdout.write(stdout_str or "")
      sys.stderr.write(stderr_str or "")
    return Response.OUTPUT, {
        "stdout": stdout_str or "",
        "stderr": stderr_str or "",
    }

  async def _wait_for_crossbench(self) -> None:
    assert self._crossbench_process
    stdout_sender = asyncio.create_task(
        self._send_stdout_incrementally(self._crossbench_process.stdout))
    stderr_sender = asyncio.create_task(
        self._send_stderr_incrementally(self._crossbench_process.stderr))
    # TODO: Figure out why waiting on sending the output hangs when the
    # crossbench subprocess ends with exit!=0
    await asyncio.wait([stdout_sender, stderr_sender])
    stdout, stderr = await self._crossbench_process.communicate()
    await self._send_message(
        self._send_output(stdout.decode("utf-8"), stderr.decode("utf-8")))
    returncode = self._crossbench_process.returncode
    self._state.transition(State.RUNNING, to=State.CONNECTED)
    self._crossbench_task = None
    self._crossbench_process = None
    await self._send_message(self._status_command())
    logging.info("# CROSSBENCH COMMAND DONE: returncode=%s", returncode)

  _OUTPUT_BUFFER_SIZE = 128

  async def _send_stdout_incrementally(self, stdout) -> None:
    while self._crossbench_process:
      stdout_data = await stdout.read(self._OUTPUT_BUFFER_SIZE)
      if not stdout_data:
        return
      stdout_str = stdout_data.decode("utf-8")
      await self._send_message(self._send_output(stdout_str, None))

  async def _send_stderr_incrementally(self, stderr) -> None:
    while self._crossbench_process:
      stderr_data = await stderr.read(self._OUTPUT_BUFFER_SIZE)
      if not stderr_data:
        return
      stderr_str = stderr_data.decode("utf-8")
      await self._send_message(self._send_output(None, stderr_str))

  async def _status_command(self) -> tuple[Response, str]:
    return Response.STATUS, str(self._state.name)
