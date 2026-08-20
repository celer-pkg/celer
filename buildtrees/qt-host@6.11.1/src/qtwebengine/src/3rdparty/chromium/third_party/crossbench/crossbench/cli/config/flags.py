# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import dataclasses
import functools
import logging
from typing import (TYPE_CHECKING, Any, Final, Iterable, Optional, Self,
                    Sequence, Set, Type)

from immutabledict import immutabledict
from ordered_set import OrderedSet
from typing_extensions import override

from crossbench import exception
from crossbench.browsers.browser_helper import convert_flags_to_label
from crossbench.config import ConfigError, ConfigObject
from crossbench.flags.chrome import ChromeFlags
from crossbench.flags.js_flags import JSFlags
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  from crossbench.flags.base import Flags, FlagsData


DEFAULT_LABEL: Final[str] = "default"


def _parse_flags(flag_data: str | list | tuple | FlagsData | None) -> Flags:
  if not flag_data:
    return ChromeFlags().freeze()
  if isinstance(flag_data, str):
    return ChromeFlags.parse_str(flag_data).freeze()
  if isinstance(flag_data, (list, tuple)):
    return _parse_flags_sequence(flag_data)
  return ChromeFlags.parse(flag_data).freeze()


def _parse_flags_sequence(flag_data: Iterable) -> Flags:
  split_flags = (ChromeFlags.split(flag) for flag in flag_data)
  return ChromeFlags(split_flags).freeze()


@dataclasses.dataclass(frozen=True)
class FlagsVariantConfig:
  label: str
  index: int = 0
  flags: Flags = dataclasses.field(
      default_factory=lambda: ChromeFlags().freeze())

  @classmethod
  def parse(cls, name: str, index: int, data: Any) -> FlagsVariantConfig:
    return cls(name, index, _parse_flags(data))

  def merge_copy(self,
                 other: FlagsVariantConfig,
                 label: Optional[str] = None,
                 index: int = -1) -> FlagsVariantConfig:
    index = self.index if index < 0 else index
    new_label = label or f"{self.label}_{other.label}"
    return FlagsVariantConfig(new_label, index,
                              self.flags.merge_copy(other.flags).freeze())

  def __hash__(self) -> int:
    return hash(self.flags)

  def __eq__(self, other: Any) -> bool:
    if not isinstance(other, FlagsVariantConfig):
      return False
    return self.flags == other.flags


class FlagsGroupConfig(tuple[FlagsVariantConfig, ...]):
  """
  Config container for a list of FlagsVariantConfig:
  FlagsGroupConfig(
    FlagsVariantConfig("default"),
    FlagsVariantConfig("max_opt_1", "--js-flags='--max-opt=1'),
    FlagsVariantConfig("max_opt_2", "--js-flags='--max-opt=2'),
    ...
  )
  """

  @classmethod
  def parse(cls, data: Any) -> Self:
    if data is None:
      return cls()
    if isinstance(data, str):
      return cls.parse_str(data)
    if isinstance(data, dict):
      return cls.parse_dict(data)
    if isinstance(data, (list, tuple)):
      return cls.parse_sequence(data)
    if isinstance(data, argparse.Namespace):
      return cls.parse_args(data)
    raise ConfigError(f"Invalid type {type(data)}: {repr(data)}")

  @classmethod
  def parse_dict(cls, config: dict) -> Self:
    if not config:
      return cls()
    all_flag_keys = all(key.startswith("-") for key in config.keys())
    all_str_values = all(isinstance(value, str) for value in config.values())
    if not all_flag_keys:
      return cls.parse_dict_with_labels(config)
    if all_str_values:
      return cls.parse_dict_simple(config)
    return cls._parse_variants_dict(config)

  @classmethod
  def parse_dict_with_labels(cls, config: dict) -> Self:
    variants: OrderedSet[FlagsVariantConfig] = OrderedSet()
    logging.debug("Using custom flag group labels")
    for label, value in config.items():
      with exception.annotate_argparsing(
          f"Parsing flag variant ...[{repr(label)}]:"):
        variant = FlagsVariantConfig.parse(label, len(variants), value)
        if variant in variants:
          raise ConfigError(f"Duplicate flag variant: {value}")
        variants.add(variant)
    return cls(tuple(variants))

  @classmethod
  def parse_dict_simple(cls, config: dict) -> Self:
    logging.debug("Using single flag group dict")
    variants = (FlagsVariantConfig.parse(DEFAULT_LABEL, 0, config),)
    return cls(variants)

  @classmethod
  def _parse_variants_dict(cls: Type[Self], data: dict[str, Any]) -> Self:
    # data == {
    #  "--flag": None,
    #  "--flag-b": "custom flag value",
    #  "--flag-c": (None, "value 2", "value 3"),
    # }
    cls._validate_variants_dict(data)
    # TODO: Use list[Self] once pytype supports it.
    per_flag_groups: list = []
    for flag_name, flag_data in data.items():
      group = cls._dict_variant_to_group(flag_name, flag_data)
      assert isinstance(group, cls)
      per_flag_groups.append(group)

    variants = per_flag_groups[0]
    for next_variant in per_flag_groups[1:]:
      variants = variants.product(next_variant)
    return variants

  @classmethod
  def _validate_variants_dict(cls, data: dict[str, Any]) -> None:
    flags = ChromeFlags()
    for flag_name, flag_value in data.items():
      with exception.annotate_argparsing(
          f"Parsing flag variant ...[{flag_name}]:"):
        if flag_value is None or isinstance(flag_value, str):
          cls._validate_variants_value(flags, flag_name, flag_value)
          continue
        if isinstance(flag_value, (list, tuple)):
          cls._validate_variants_sequence(flags, flag_name, flag_value)
          continue
        raise ConfigError(
            f"Invalid flag variant value (None, str or sequence): "
            f"{flag_name}={repr(flag_value)}")

  @classmethod
  def _validate_variants_sequence(cls, flags: ChromeFlags, flag_name: str,
                                  flag_values: Sequence) -> None:
    ObjectParser.unique_sequence(flag_values,
                                 f"flag {repr(flag_name)} variant values",
                                 ConfigError)
    for sequence_flag_value in flag_values:
      cls._validate_variants_value(flags.copy(), flag_name, sequence_flag_value)

  @classmethod
  def _validate_variants_value(cls, flags: ChromeFlags, flag_name: str,
                               flag_value: Any) -> None:
    if flag_value is None:
      flags.set(flag_name)
      return
    if isinstance(flag_value, str):
      flags.set(flag_name, flag_value)
      return
    raise ConfigError(f"Invalid flag variant value: "
                      f"{flag_name}={repr(flag_value)}")

  @classmethod
  def _dict_variant_to_group(cls, flag_name: str, data: Any) -> Self:
    if data is None:
      return cls.parse_str(flag_name)
    if isinstance(data, str):
      data_str: str = data.strip()
      if not data_str:
        return cls.parse_str(flag_name)
      data = (data_str,)
    assert isinstance(data, (list, tuple)), "Invalid flag variant type"
    flags: OrderedSet[Flags | None] = OrderedSet()
    for variant in data:
      if variant is None:
        flag = None
      elif not variant.strip():
        flag = ChromeFlags((flag_name,))
      else:
        cls._validate_variant_flag(flag_name, variant)
        flag = ChromeFlags({flag_name: variant})
      if flag in flags:
        raise ConfigError("Same flag variant was specified more than once: "
                          f"{repr(flag)} for entry {repr(flag_name)}")
      flags.add(flag)
    return cls.parse_sequence(flags)

  @classmethod
  def _validate_variant_flag(cls, flag_name: str, flag_value: Any) -> None:
    if flag_value == "None,":
      raise ConfigError("Please use null (from json) instead of "
                        f"None (from python) for flag {repr(flag_name)}")

  @classmethod
  def parse_sequence(cls, data: Sequence) -> Self:
    variants: list[FlagsVariantConfig] = []
    duplicates: Set[str] = set()
    for flag_data in data:
      flags = _parse_flags(flag_data)
      if flag_data in duplicates:
        raise ConfigError(f"Duplicate variant: {flags}")
      duplicates.add(flag_data)
      label = convert_flags_to_label(*flags)
      variants.append(FlagsVariantConfig(label, len(variants), flags))
    return cls(tuple(variants))

  @classmethod
  def parse_str(cls, value: str) -> Self:
    if not value.strip():
      return cls()
    variants = (FlagsVariantConfig.parse(DEFAULT_LABEL, 0, value),)
    return cls(variants)

  @classmethod
  def parse_args(cls, args: argparse.Namespace) -> Self:
    args_config = cls.config_from_args_flags(args)
    if not args_config:
      # Special case empty args: we should have an empty group config
      return cls((FlagsVariantConfig(DEFAULT_LABEL),))
    return cls.parse(args_config)

  @classmethod
  def config_from_args_flags(
      cls, args: argparse.Namespace) -> dict[str, list[str] | str | None]:
    initial_flags = ChromeFlags(_parse_flags(args.other_browser_args))
    if args.enable_features:
      initial_flags["--enable-features"] = args.enable_features
    if args.disable_features:
      initial_flags["--disable-features"] = args.disable_features
    match args.enable_field_trial_config:
      case True:
        initial_flags.set("--enable-field-trial-config")
      case False:
        initial_flags.set("--disable-field-trial-config")
      case None:
        pass
      case _:
        raise ValueError(
            "Invalid field-trial-config value: {args.enable_field_trial_config}"
        )
    match args.sandbox:
      case False:
        initial_flags.set("--no-sandbox")
      case None:
        pass
      case _:
        raise ValueError(f"Unknown sandbox value: {args.sandbox}")
    # Convert flags back to dict-based config object:
    args_config: dict[str, list[str] | str | None] = dict(initial_flags.items())
    base_js_flags = initial_flags.js_flags
    if args.js_flags:
      # Create a variant for every js flag:
      merged_js_flags: list[JSFlags] = []
      for flags in args.js_flags:
        js_flags = JSFlags.parse(flags)
        js_flags.update(base_js_flags)
        merged_js_flags.append(js_flags)
      args_config["--js-flags"] = list(map(str, merged_js_flags))
    return args_config


  def product(self, *args: Self) -> Self:
    return functools.reduce(lambda a, b: a.inner_product(b), args, self)

  def inner_product(self, other: Self) -> Self:
    """Create a new FlagsGroupConfig as the combination of
    self.variants x other.variants"""
    new_variants: list[FlagsVariantConfig] = []
    new_labels: Set[str] = set()
    if not other:
      return self
    if not self:
      return other
    for variant in self:
      for variant_other in other:
        new_label = self._unique_product_label(new_labels, variant,
                                               variant_other)
        new_labels.add(new_label)
        new_variant: FlagsVariantConfig = variant.merge_copy(
            variant_other, index=len(new_variants), label=new_label)
        new_variants.append(new_variant)

    return type(self)(tuple(new_variants))

  def _unique_product_label(self, label_set: Set[str],
                            variant_a: FlagsVariantConfig,
                            variant_b: FlagsVariantConfig) -> str:
    default = f"{variant_a.label}_{variant_b.label}"
    if variant_a.label == DEFAULT_LABEL:
      default = variant_b.label
    if variant_b.label == DEFAULT_LABEL:
      default = variant_a.label
    label = default
    if not variant_a.flags:
      label = variant_b.label
    if not variant_b.flags:
      label = variant_a.label
    if label not in label_set:
      return label
    if default not in label_set:
      return default
    return f"{default}_{len(label_set)}"


class FlagsConfig(ConfigObject, immutabledict[str, FlagsGroupConfig]):

  @classmethod
  @override
  def parse_str(cls, value: str) -> Self:
    if not value:
      raise ConfigError("Cannot parse empty string")
    return cls({"default": FlagsGroupConfig.parse_str(value)})

  @classmethod
  @override
  def parse_dict(cls, config: dict[str, Any], **kwargs) -> Self:
    groups: dict[str, FlagsGroupConfig] = {}
    for group_name, group_data in config.items():
      with exception.annotate(f"Parsing flag-group: flags[{repr(group_name)}]"):
        groups[group_name] = FlagsGroupConfig.parse(group_data)
    return cls(groups)
