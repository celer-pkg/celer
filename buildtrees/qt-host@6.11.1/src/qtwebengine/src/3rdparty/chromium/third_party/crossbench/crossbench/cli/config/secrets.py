# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import dataclasses
import random
from typing import TYPE_CHECKING, Self

from typing_extensions import override

from crossbench.config import ConfigObject, ConfigParser
from crossbench.parse import NumberParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.types import JsonDict


@dataclasses.dataclass(frozen=True)
class Secrets(ConfigObject):
  """A set of predefined secrets for common logins.
  """
  google: UsernamePassword | None = None
  bond: ServiceAccount | None = None

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = ConfigParser(cls)
    parser.add_argument("google", type=GoogleUsernamePassword)
    parser.add_argument("bond", type=ServiceAccount)
    return parser

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    raise NotImplementedError("Cannot create secrets from string")

  def merge(self, fallback: Secrets) -> Self:
    return type(self)(self.google or fallback.google, self.bond or
                      fallback.bond)


class Secret(ConfigObject):
  """A single username / password combination. """

  @property
  def is_interactive(self) -> bool:
    return False

@dataclasses.dataclass(frozen=True)
class UsernamePassword(Secret):
  username: str
  password: str

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = ConfigParser(cls)
    parser.add_argument(
        "username",
        aliases=("user", "usr", "account", "account-name"),
        type=ObjectParser.non_empty_str,
        required=True)
    parser.add_argument(
        "password",
        aliases=("pass", "pw", "account-password"),
        type=ObjectParser.any_str,
        required=True)
    return parser

  @classmethod
  @override
  def parse_str(cls, value: str) -> UsernamePassword:
    if value == "interactive":
      return InteractiveUsernamePassword()
    # TODO: maybe support passwd style string format
    raise NotImplementedError("Cannot support")


class InteractiveUsernamePassword(UsernamePassword):
  """Interactive secret that defers the input to the user so we can 
  live test the login process. """

  def __init__(self):
    super().__init__("", "")

  @property
  def is_interactive(self) -> bool:
    return True


class CycledUsernamePassword(UsernamePassword):

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "use_range",
        aliases=("use-account-range", "enable-range"),
        type=ObjectParser.bool)
    parser.add_argument(
        "start",
        aliases=("account-range-start", "range-start"),
        type=NumberParser.positive_zero_int)
    parser.add_argument(
        "end",
        aliases=("account-range-end", "range-end"),
        type=NumberParser.positive_zero_int)
    return parser

  def __init__(self,
               username: str,
               password: str,
               use_range: bool = False,
               start: int = 0,
               end: int = 0) -> None:
    if use_range:
      account_selection = random.randint(start, end)
      username = username % account_selection

    super().__init__(username, password)


class GoogleUsernamePassword(CycledUsernamePassword):
  pass


@dataclasses.dataclass(frozen=True)
class ServiceAccount(Secret):
  type: str
  project_id: str
  private_key_id: str
  private_key: str
  client_email: str
  client_id: str
  auth_uri: str
  token_uri: str
  auth_provider_x509_cert_url: str
  client_x509_cert_url: str
  universe_domain: str

  @classmethod
  @override
  def config_parser(cls) -> ConfigParser[Self]:
    parser = ConfigParser(cls)
    parser.add_argument("type", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "project_id", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "private_key_id", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "private_key", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "client_email", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "client_id", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "auth_uri", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "token_uri", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "auth_provider_x509_cert_url",
        type=ObjectParser.non_empty_str,
        required=True)
    parser.add_argument(
        "client_x509_cert_url", type=ObjectParser.non_empty_str, required=True)
    parser.add_argument(
        "universe_domain", type=ObjectParser.non_empty_str, required=True)
    return parser

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    del value
    raise NotImplementedError("ServiceAccount from string not supported")

  def to_json(self) -> JsonDict:
    return {
        "type": self.type,
        "project_id": self.project_id,
        "private_key_id": self.private_key_id,
        "private_key": self.private_key,
        "client_email": self.client_email,
        "client_id": self.client_id,
        "auth_uri": self.auth_uri,
        "token_uri": self.token_uri,
        "auth_provider_x509_cert_url": self.auth_provider_x509_cert_url,
        "client_x509_cert_url": self.client_x509_cert_url,
        "universe_domain": self.universe_domain,
    }
