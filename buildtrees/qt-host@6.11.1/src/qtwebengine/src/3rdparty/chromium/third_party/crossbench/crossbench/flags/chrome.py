# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import logging
from typing import Final, Iterable, Optional

from typing_extensions import override

from crossbench import path as pth
from crossbench.flags.base import Flags, FlagsData
from crossbench.flags.chrome_extensions import ChromeExtensions
from crossbench.flags.chrome_features import (ChromeBlinkFeatures,
                                              ChromeFeatures)
from crossbench.flags.js_flags import JSFlags
from crossbench.flags.known_js_flags import KNOWN_JS_FLAGS


class ChromeFlags(Flags):
  """Specialized Flags for Chrome/Chromium-based browser.

  This has special treatment for --js-flags and the feature flags:
  --enable-features/--disable-features
  --enable-blink-features/--disable-blink-features
  """
  JS_FLAG: Final[str] = "--js-flags"

  # All flags that might affect how finch / field-trials are loaded.
  FIELD_TRIAL_ENABLE_FLAGS: tuple[str, ...] = (
      "--force-fieldtrials",
      "--variations-server-url",
      "--variations-insecure-server-url",
      "--variations-test-seed-path",
      "--enable-field-trial-config",
      "--disable-variations-safe-mode",
  )

  FIELD_TRIAL_DISABLE_FLAGS: tuple[str, ...] = ("--disable-field-trial-config",)

  USER_DATA_DIR_FLAG: Final[str] = "--user-data-dir"

  @classmethod
  def for_milestone(cls, initial_data: FlagsData = None, milestone: int = 0):
    if milestone in ChromePreM139Flags.VERSION_RANGE:
      return ChromePreM139Flags(initial_data)
    return ChromeFlags(initial_data)

  def __init__(self, initial_data: FlagsData = None) -> None:
    self._features: ChromeFeatures = ChromeFeatures()
    self._blink_features: ChromeBlinkFeatures = ChromeBlinkFeatures()
    self._js_flags: JSFlags = JSFlags()
    self._extensions: ChromeExtensions = ChromeExtensions()
    super().__init__(initial_data)

  def freeze(self) -> ChromeFlags:
    super().freeze()
    self._js_flags.freeze()
    self._features.freeze()
    self._blink_features.freeze()
    self._extensions.freeze()
    return self

  def __getitem__(self, key):
    if key == self.JS_FLAG and self._js_flags:
      return self._js_flags
    if key == ChromeFeatures.ENABLE_FLAG and self._features.enabled:
      return self._features.enabled_str()
    if key == ChromeFeatures.DISABLE_FLAG and self._features.disabled:
      return self._features.disabled_str()
    if key == ChromeBlinkFeatures.ENABLE_FLAG and self._blink_features.enabled:
      return self._blink_features.enabled_str()
    if (key == ChromeBlinkFeatures.DISABLE_FLAG and
        self._blink_features.disabled):
      return self._blink_features.disabled_str()
    if key in ChromeExtensions.FLAGS:
      return self._extensions[key]
    return super().__getitem__(key)

  @override
  def _set(self,
           flag_name: str,
           flag_value: Optional[str] = None,
           should_override: bool = False) -> None:
    self.assert_not_frozen()
    # pylint: disable=signature-differs
    if self._set_special_flags(flag_name, flag_value, should_override):
      return
    if candidate := self._find_misspelled_flag(flag_name):
      logging.error(
          "Potentially misspelled flag: '%s'. "
          "Did you mean to use %s ?", flag_name, candidate)
      # Retry setting special flags.
      if self._set_special_flags(candidate, flag_value, should_override):
        return
    super()._set(flag_name, flag_value, should_override)

  def _set_special_flags(self,
                         flag_name: str,
                         flag_value: Optional[str] = None,
                         should_override: bool = False) -> bool:
    if flag_name == ChromeFeatures.ENABLE_FLAG:
      if flag_value is None:
        self._features.clear_enabled()
      else:
        for feature in flag_value.split(","):
          self._features.enable(feature)
      return True
    if flag_name == ChromeFeatures.DISABLE_FLAG:
      if flag_value is None:
        self._features.clear_disabled()
      else:
        for feature in flag_value.split(","):
          self._features.disable(feature)
      return True
    if flag_name == ChromeBlinkFeatures.ENABLE_FLAG:
      if flag_value is None:
        self.blink_features.clear_enabled()
      else:
        for feature in flag_value.split(","):
          self._blink_features.enable(feature)
      return True
    if flag_name == ChromeBlinkFeatures.DISABLE_FLAG:
      if flag_value is None:
        self.blink_features.clear_disabled()
      else:
        for feature in flag_value.split(","):
          self._blink_features.disable(feature)
      return True
    if flag_name == self.JS_FLAG:
      if flag_value is None:
        self._js_flags.clear()
      else:
        self._set_js_flag(flag_value, should_override)
      return True
    if flag_name == self.USER_DATA_DIR_FLAG:
      self._set_user_data_dir(flag_value)
      return True
    if flag_name in ChromeExtensions.FLAGS:
      self._extensions.set(flag_name, flag_value, should_override)
      return True
    if candidate := self._find_js_flag(flag_name):
      js_flags = JSFlags()
      js_flags.set(candidate, flag_value)
      logging.error(
          "Got potential V8 flag %s that should be used as "
          "--js-flags=%s", repr(flag_name), js_flags)
      return False
    return False

  def _set_js_flag(self, raw_js_flags: str, should_override: bool) -> None:
    new_js_flags = JSFlags(self._js_flags)
    for js_flag_name, js_flag_value in JSFlags.parse(raw_js_flags).items():
      new_js_flags.set(
          js_flag_name, js_flag_value, should_override=should_override)
    self._js_flags.update(new_js_flags)

  def _find_misspelled_flag(self, name: str) -> Optional[str]:
    if name in ("--enable-feature", "--enabled-feature", "--enabled-features"):
      return "--enable-features"
    if name in ("--disable-feature", "--disabled-feature",
                "--disabled-features"):
      return "--disable-features"
    if name in ("--enable-blink-feature", "--enabled-blink-feature",
                "--enabled-blink-features"):
      return "--enable-blink-features"
    if name in ("--disable-blink-feature", "--disabled-blink-feature",
                "--disabled-blink-features"):
      return "--disable-blink-features"
    if name in ("--enable-field-trials", "--enable-field-trials-config"):
      return "--enable-field-trial-config"
    if name in ("--enable-extensions", "--load-extensions"):
      return "--load-extension"
    return None

  def _find_js_flag(self, name: str) -> Optional[str]:
    normalized_name = name
    if name.startswith("--no-"):
      normalized_name = f"--{name[5:]}"
    elif name.startswith("--no"):
      normalized_name = f"--{name[4:]}"
    if normalized_name in KNOWN_JS_FLAGS:
      return name
    return None

  def _set_user_data_dir(self, value: Optional[str]):
    if not value or not value.strip():
      raise ValueError("--user-data-dir cannot be the empty string.")
    # TODO: support remote platforms
    expanded_dir = str(pth.LocalPath(value).expanduser())
    if expanded_dir != value:
      logging.warning(
          "Chrome Flags: auto-expanding --user-data-dir from '%s' to '%s'",
          value, expanded_dir)
    self.data[self.USER_DATA_DIR_FLAG] = expanded_dir

  @property
  def features(self) -> ChromeFeatures:
    return self._features

  @property
  def blink_features(self) -> ChromeBlinkFeatures:
    return self._blink_features

  @property
  def extensions(self) -> ChromeExtensions:
    return self._extensions

  @property
  def js_flags(self) -> JSFlags:
    return self._js_flags

  @property
  def field_trial_enable_flags(self) -> ChromeFlags:
    return self.filtered(self.FIELD_TRIAL_ENABLE_FLAGS)

  @property
  def field_trial_disable_flags(self) -> ChromeFlags:
    return self.filtered(self.FIELD_TRIAL_DISABLE_FLAGS)

  def enable_benchmarking_api(self) -> None:
    self.set("--enable-benchmarking-api")

  @override
  def merge(self, other: FlagsData) -> None:
    if not isinstance(other, ChromeFlags):
      other = ChromeFlags(other)
    self.features.merge(other.features)
    self.blink_features.merge(other.blink_features)
    self.js_flags.merge(other.js_flags)
    self.extensions.merge(other.extensions)
    for name, value in other.base_items():
      self.set(name, value)

  def base_items(self) -> Iterable[tuple[str, str | None]]:
    yield from super().items()

  @override
  def items(self) -> Iterable[tuple[str, str | None]]:  # type: ignore
    yield from self.base_items()
    if self._js_flags:
      yield (self.JS_FLAG, str(self.js_flags))
    yield from self.features.items()
    yield from self.blink_features.items()
    yield from self.extensions.items()

  def __bool__(self) -> bool:
    return bool(self.data) or bool(self._js_flags) or bool(
        self._features) or bool(self._blink_features)

  @override
  def validate(self) -> None:
    field_trial_enable_flags = self.field_trial_enable_flags
    field_trial_disable_flags = self.field_trial_disable_flags
    if field_trial_enable_flags and field_trial_disable_flags:
      raise argparse.ArgumentTypeError(
          f"Conflicting {type(self).__name__} detected: "
          f"{field_trial_enable_flags} vs {field_trial_disable_flags}.\n"
          "Cannot enable and disable finch / field-trials at the same time.")


class ChromePreM139Flags(ChromeFlags):
  VERSION_RANGE: Final[range] = range(1, 139)
  FIELD_TRIAL_ENABLE_FLAGS: tuple[
      str, ...] = ChromeFlags.FIELD_TRIAL_ENABLE_FLAGS + (
          # The benchmarking flag without value is a no-field-trial flag.
          # However, when used as
          # '--enable-benchmarking=enable-field-trial-config' it works
          # with field trials.
          "--enable-benchmarking",)

  FIELD_TRIAL_DISABLE_FLAGS: tuple[
      str, ...] = ChromeFlags.FIELD_TRIAL_DISABLE_FLAGS + (
          # The benchmarking flag without value is a no-field-trial flag.
          # However, when used as
          # '--enable-benchmarking=enable-field-trial-config' it works
          # with field trials.
          "--enable-benchmarking",)

  def has_enable_benchmarking_field_trials(self):
    # Enable the benchmarking extension with field trial configs which
    # requires a special value. See `ShouldUseFieldTrialTestingConfig()`.
    # https://crsrc.org/c/components/variations/service/variations_field_trial_creator_base.cc;l=138;drc=27d34700b83f381c62e3a348de2e6dfdc08364b8
    return self.get("--enable-benchmarking") == "enable-field-trial-config"

  @property
  @override
  def field_trial_enable_flags(self) -> ChromeFlags:
    filtered = self.filtered(self.FIELD_TRIAL_ENABLE_FLAGS)
    if "--enable-benchmarking" in self and (
        not self.has_enable_benchmarking_field_trials()):
      del filtered["--enable-benchmarking"]
    return filtered

  @property
  @override
  def field_trial_disable_flags(self) -> ChromeFlags:
    filtered_copy = self.filtered(self.FIELD_TRIAL_DISABLE_FLAGS)
    # Special case for --enable-benchmarking which disables field trials
    # by default, unless it has a "enable-field-trial-config" value.
    if self.has_enable_benchmarking_field_trials():
      del filtered_copy["--enable-benchmarking"]
    return filtered_copy

  @override
  def enable_benchmarking_api(self) -> None:
    if self.field_trial_enable_flags:
      self.set("--enable-benchmarking", "enable-field-trial-config")
    else:
      self.set("--enable-benchmarking")
