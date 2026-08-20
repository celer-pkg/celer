# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
from typing import Any, Self

from typing_extensions import override

from crossbench import exception
from crossbench.benchmarks.loading.config.blocks import (ActionBlock,
                                                         ActionBlockListConfig)
from crossbench.config import ConfigObject
from crossbench.parse import ObjectParser


@dataclasses.dataclass(frozen=True)
class CUJConfig:
  label: str
  blocks: tuple[ActionBlock, ...] = tuple()


@dataclasses.dataclass(frozen=True)
class CUJsConfig(ConfigObject):
  cujs: tuple[CUJConfig, ...] = ()

  @override
  def validate(self) -> None:
    super().validate()
    for index, cuj in enumerate(self.cujs):
      assert isinstance(cuj, CUJConfig), (
          f"cujs[{index}] is not an CUJConfig "
          f"but {type(cuj).__name__}")

  @classmethod
  @override
  def parse_str(cls, value: str):
    del value
    raise NotImplementedError("Cannot create CUJsConfig from string")

  @classmethod
  @override
  def parse_dict(cls, config: dict, **kwargs) -> Self:
    """
    Variant a):
      { "cujs": { "LABEL": CUJ_ACTION_CONFIG } }
    """
    with exception.annotate_argparsing("Parsing stories"):
      if "cujs" not in config:
        raise argparse.ArgumentTypeError(
            "Config does not provide a 'cujs' dict.")
      cujs_config = ObjectParser.non_empty_dict(config["cujs"], "cujs")
      with exception.annotate_argparsing("Parsing config 'cujs'"):
        cujs = cls._parse_cujs(cujs_config)
        return cls(cujs)
    raise exception.UnreachableError()

  @classmethod
  def _parse_cujs(cls, data: dict[str, Any]) -> tuple[CUJConfig, ...]:
    cujs = []
    for name, cuj_config in data.items():
      with exception.annotate_argparsing(f"Parsing story ...['{name}']"):
        cuj = CUJConfig(
          label=name,
          blocks=ActionBlockListConfig.parse(cuj_config).blocks
        )
        cujs.append(cuj)
    return tuple(cujs)
