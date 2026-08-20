# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
import urllib.parse as urlparse
from typing import TYPE_CHECKING, Any, Callable, Mapping, Optional

import requests

if TYPE_CHECKING:
  from crossbench.runner.timing import AnyTime

DEFAULT_REQUEST_TIMEOUT = dt.timedelta(seconds=10)

RequestException = requests.RequestException
HTTPError = requests.HTTPError

Response = requests.Response


def retry_timeout_request(
    url: str, timeout: AnyTime, retry: int, verbose: bool, method: str,
    send_request: Callable[[float], requests.Response]) -> requests.Response:
  deadline = dt.datetime.now() + dt.timedelta(seconds=to_seconds(timeout))
  request_timeout_seconds = to_seconds(timeout)
  i = 0
  while True:
    try:
      if verbose:
        logging.debug("%s: url: %s", method, url)
      response = send_request(request_timeout_seconds)
      response.raise_for_status()
      return response
    except requests.RequestException as e:
      request_timeout_seconds = (deadline - dt.datetime.now()).total_seconds()
      if i < retry and request_timeout_seconds > 0:
        i += 1
        if verbose:
          logging.warning("%s request failed url=%s, retrying: %s", method, url,
                          e)
        continue
      if verbose:
        logging.error("%s request failed url=%s", method, url)
      logging.debug("RequestException: %s", e)
      raise e


def get(url: str,
        timeout: AnyTime = DEFAULT_REQUEST_TIMEOUT,
        retry: int = 0,
        verbose: bool = True) -> requests.Response:
  return retry_timeout_request(
      url, timeout, retry, verbose, "GET",
      lambda request_timeout_seconds: requests.get(
          url, timeout=request_timeout_seconds))

def post(url: str,
         body_json: Optional[Any] = None,
         headers: Optional[Mapping[str, str]] = None,
         timeout: AnyTime = DEFAULT_REQUEST_TIMEOUT,
         retry: int = 0,
         verbose: bool = True) -> requests.Response:
  return retry_timeout_request(
      url, timeout, retry, verbose, "POST",
      lambda request_timeout_seconds: requests.post(
          url, headers=headers, json=body_json, timeout=request_timeout_seconds
      ))


def to_seconds(delta: AnyTime) -> float:
  if isinstance(delta, dt.timedelta):
    return delta.total_seconds()
  return delta


def update_url_query(url: str, query_params: Mapping[str, str]) -> str:
  parsed_url = urlparse.urlparse(url)
  query = dict(urlparse.parse_qsl(parsed_url.query))
  query.update(query_params)
  parsed_url = parsed_url._replace(query=urlparse.urlencode(query, doseq=True))
  return parsed_url.geturl()


def quote(value: str) -> str:
  return urlparse.quote(value)
