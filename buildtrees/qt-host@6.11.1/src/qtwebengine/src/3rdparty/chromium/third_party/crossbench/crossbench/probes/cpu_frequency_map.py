# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import argparse
import enum
import re
from typing import TYPE_CHECKING, Any, Hashable, Mapping, Pattern, TypeAlias

from immutabledict import immutabledict
from typing_extensions import override

from crossbench import exception
from crossbench import path as pth
from crossbench.config import ConfigObject
from crossbench.parse import NumberParser, ObjectParser

if TYPE_CHECKING:
  from crossbench.plt.base import Platform

# Directory exposing info & controls for the frequency of all CPUs.
_CPUS_DIR: pth.AnyPosixPath = pth.AnyPosixPath("/sys/devices/system/cpu")

# Used to specify behavior for all CPUs.
_WILDCARD_CONFIG_KEY = "*"

# Matches the CPU names exposed by the system in _CPUS_DIR.
_CPU_NAME_REGEX: Pattern[str] = re.compile("cpu[0-9]+$")


class _ExtremeFrequency(enum.StrEnum):
  MAX = "max"
  MIN = "min"


if TYPE_CHECKING:
  FrequencyType: TypeAlias = _ExtremeFrequency | int


class CPUFrequencyMap(ConfigObject, metaclass=abc.ABCMeta):

  @abc.abstractmethod
  def get_target_frequencies(
      self, platform: Platform) -> immutabledict[pth.AnyPosixPath, int]:
    raise NotImplementedError()

  @property
  @abc.abstractmethod
  def key(self) -> Hashable:
    raise NotImplementedError()

  @classmethod
  @override
  def parse_dict(cls, config: Mapping[str, Any], **kwargs) -> CPUFrequencyMap:
    if _WILDCARD_CONFIG_KEY in config:
      return WildcardCPUFrequencyMap(config)
    return ExplicitCPUFrequencyMap(config)

  @classmethod
  @override
  def parse_str(cls, value: str) -> CPUFrequencyMap:
    return CPUFrequencyMap.parse_dict({_WILDCARD_CONFIG_KEY: value})

  @classmethod
  def _parse_frequency(cls, value: Any) -> FrequencyType:
    if value == _ExtremeFrequency.MIN:
      return _ExtremeFrequency.MIN

    if value == _ExtremeFrequency.MAX:
      return _ExtremeFrequency.MAX

    try:
      return NumberParser.positive_zero_int(value)
    except argparse.ArgumentTypeError as e:
      raise argparse.ArgumentTypeError(
          f"Invalid value in CPU frequency map: {value}. Should "
          "have been one of \"max\"|\"min\"|<int>|\"<int>\"") from e

  def _get_target_frequency(self, platform: Platform, cpu_name: str,
                            frequency: FrequencyType) -> int:
    if not platform.exists(_CPUS_DIR):
      # TODO(crbug.com/372862708): If different devices indeed use different
      # dirs, consider making this configurable in the jSON.
      raise FileNotFoundError(
          f"{_CPUS_DIR} not found. Either {platform} does not support setting "
          "CPU frequency or the CPUs are exposed in another path and that "
          "requires extra support.")

    cpu_dir: pth.AnyPosixPath = self._get_cpu_dir(cpu_name)
    if not platform.is_dir(cpu_dir):
      raise ValueError(f"Invalid CPU name: {cpu_name}.")

    available_frequencies: list[int] = [
        NumberParser.positive_zero_int(f)
        for f in platform.cat(cpu_dir / "scaling_available_frequencies").rstrip(
            "\n").rstrip(" ").split(" ")
    ]
    if frequency == _ExtremeFrequency.MIN:
      return min(available_frequencies)
    if frequency == _ExtremeFrequency.MAX:
      return max(available_frequencies)
    if frequency in available_frequencies:
      assert isinstance(frequency, int)
      return frequency
    raise ValueError(f"Target frequency {frequency} for {cpu_name} "
                     f"not allowed in {platform}. Available frequencies: "
                     f"{available_frequencies}")

  def _get_cpu_dir(self, cpu_name: str) -> pth.AnyPosixPath:
    # Create new AnyPosixPath so pyfakefs is happy in tests.
    return pth.AnyPosixPath(_CPUS_DIR / cpu_name / "cpufreq")


class WildcardCPUFrequencyMap(CPUFrequencyMap):

  def __init__(self, frequencies: Mapping) -> None:
    if len(frequencies) != 1:
      raise argparse.ArgumentTypeError(
          f"A wildcard ({_WILDCARD_CONFIG_KEY}) in "
          "the CPU frequency map should be the only key.")

    self._target_frequency = CPUFrequencyMap._parse_frequency(
        list(frequencies.values())[0])

  @override
  def get_target_frequencies(
      self, platform: Platform) -> immutabledict[pth.AnyPosixPath, int]:
    return immutabledict({
        self._get_cpu_dir(p.name):
            self._get_target_frequency(platform, p.name, self._target_frequency)
        for p in platform.iterdir(_CPUS_DIR)
        if _CPU_NAME_REGEX.match(p.name)
    })

  @property
  @override
  def key(self) -> Hashable:
    return self._target_frequency


class ExplicitCPUFrequencyMap(CPUFrequencyMap):

  def __init__(self, frequencies: Mapping) -> None:
    typed_map: dict[str, FrequencyType] = {}
    for k, v in frequencies.items():
      with exception.annotate_argparsing(f"Parsing cpu frequency: {k}, {v}"):
        typed_map[ObjectParser.non_empty_str(k)] = (
            CPUFrequencyMap._parse_frequency(v))
    self._frequencies: immutabledict[str,
                                     FrequencyType] = immutabledict(typed_map)

  @override
  def get_target_frequencies(
      self, platform: Platform) -> immutabledict[pth.AnyPosixPath, int]:
    return immutabledict({
        self._get_cpu_dir(cpu_name):
            self._get_target_frequency(platform, cpu_name, config_frequeny)
        for cpu_name, config_frequeny in self._frequencies.items()
    })

  @property
  @override
  def key(self) -> Hashable:
    return self._frequencies
