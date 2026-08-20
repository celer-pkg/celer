# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import logging
from typing import TYPE_CHECKING, Any, Iterable, Self, Sequence

from typing_extensions import override

from crossbench import exception
from crossbench.cli.config.probe import ProbeConfig, ProbeConfigError
from crossbench.config import ConfigObject
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  import argparse

  import crossbench.path as pth
  from crossbench.probes.probe import Probe


class ProbeListConfig(ConfigObject):

  @classmethod
  def from_cli_args(cls, args: argparse.Namespace) -> Self:
    with exception.annotate_argparsing():
      config_from_args = cls(args.probe)
      if not args.probe_config:
        return config_from_args
      probe_config_path: pth.LocalPath = args.probe_config
      config_from_file = cls.parse(probe_config_path)
      with exception.annotate(
          f"Merging probe config ({probe_config_path.name}) with cli --probe:"):
        return config_from_file.merge(config_from_args, should_override=True)
    raise exception.UnreachableError()

  @classmethod
  def parse_other(cls, value: Any) -> Self:
    if isinstance(value, (tuple, list)):
      return cls.parse_sequence(value)
    return super().parse_other(value)

  @classmethod
  def parse_sequence(cls, config: Sequence[dict[str, Any]]) -> Self:
    probe_configs: list[ProbeConfig] = []
    for index, probe_config in enumerate(config):
      with exception.annotate(f"Parsing probes[{index}]"):
        probe_configs.append(ProbeConfig.parse(probe_config))
    return cls(probe_configs)

  @classmethod
  @override
  def parse_dict(cls, config: dict[str, Any], **kwargs) -> Self:
    # Support global configs with {"probes": ...}
    if "probes" in config:
      config = config["probes"]
      if isinstance(config, (tuple, list)):
        return cls.parse_sequence(config)
    elif "browsers" in config or "flags" in config:
      raise ProbeConfigError("Missing 'probes' property in global config.")
    config = ObjectParser.dict(config, "probes")
    probe_configs: list[ProbeConfig] = []
    for probe_name, config_data in config.items():
      with exception.annotate(f"Parsing probe config probes['{probe_name}']"):
        probe_configs.append(
            ProbeConfig.parse_probe_dict(probe_name, config_data))
    return cls(probe_configs)

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    raise NotImplementedError()

  def __init__(
      self,
      probe_configs: Iterable[ProbeConfig] = tuple(),
      probes: Iterable[Probe] = tuple()
  ) -> None:
    self._probes: dict[str, Probe] = {}
    if not probe_configs and not probes:
      return
    for probe_config in probe_configs:
      with exception.annotate(f"Parsing --probe={probe_config.name}"):
        self._add_probe_config(probe_config)
    for probe in probes:
      self._add_probe(probe)

  @property
  def probes(self) -> list[Probe]:
    return list(self._probes.values())

  def _add_probe_config(self, probe_config: ProbeConfig) -> None:
    probe: Probe = probe_config.probe_cls.from_config(probe_config.config)
    self._add_probe(probe)

  def _add_probe(self, probe: Probe) -> None:
    if probe.name in self._probes:
      raise ValueError(f"Duplicate probe: {probe.name}")
    self._probes[probe.name] = probe

  def merge(self, other: Self, should_override: bool = False) -> Self:
    merged_probes = {probe.name: probe for probe in self.probes}
    for probe in other.probes:
      name = probe.name
      if name in merged_probes:
        if not should_override:
          raise ValueError(f"Duplicate probe: {name}")
        logging.warning("PROBES: Overriding existing probe %s!", name)
      merged_probes[name] = probe

    merged = type(self)(probes=merged_probes.values())
    return merged
