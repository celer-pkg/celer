# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import argparse
import contextlib
import dataclasses
import logging
from typing import (TYPE_CHECKING, Any, Final, Iterator, Optional, Self,
                    Sequence, Set, TextIO, Type, cast)

from typing_extensions import override

import crossbench.browsers.all as all_browsers
from crossbench import exception
from crossbench import hjson as cb_hjson
from crossbench import path as pth
from crossbench import plt
from crossbench.browsers.browser_helper import convert_flags_to_label
from crossbench.browsers.chrome.downloader import ChromeDownloader
from crossbench.browsers.firefox.downloader import FirefoxDownloader
from crossbench.browsers.settings import Settings
from crossbench.cli.config.browser import SUPPORTED_EMBEDDER, BrowserConfig
from crossbench.cli.config.driver_type import BrowserDriverType
from crossbench.cli.config.flags import (DEFAULT_LABEL, FlagsConfig,
                                         FlagsGroupConfig, FlagsVariantConfig)
from crossbench.cli.config.network import NetworkConfig
from crossbench.config import ConfigError
from crossbench.flags.base import Flags
from crossbench.helper.cwd import ChangeCWD
from crossbench.parse import LateArgumentError, ObjectParser

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.cli.config.env import EnvConfig
  from crossbench.network.base import Network

  FlagGroupItemT = tuple[str, str | None] | None
  BrowserLookupTableT = dict[str, tuple[Type[Browser], "BrowserConfig"]]

# Add some slack for buffer for browser + platform names. Note that ultimately
# this is going to get cropped to MAX_PART_LEN.
MAX_LABEL_LEN: Final[int] = pth.MAX_PART_LEN - 50

@contextlib.contextmanager
def late_argument_type_error_wrapper(flag: str) -> Iterator[None]:
  """Converts raised ValueError and ArgumentTypeError to LateArgumentError
  that are associated with the given flag.
  """
  try:
    yield
  except Exception as e:
    raise LateArgumentError(flag, str(e)) from e


def _flags_to_label(flags: Flags) -> str:
  return convert_flags_to_label(*flags)


@dataclasses.dataclass(frozen=True)
class BrowserVariantConfig():
  label: str
  browser_cls: Type[Browser]
  browser_config: BrowserConfig
  settings: Settings

  @property
  def path(self) -> pth.AnyPath:
    return self.browser_config.path

  @property
  def js_flags(self) -> Flags:
    return self.settings.js_flags

  @property
  def flags(self) -> Flags:
    return self.settings.flags

  @property
  def platform(self) -> plt.Platform:
    return self.settings.platform


class BaseBrowserVariantsConfig(abc.ABC):

  @classmethod
  @abc.abstractmethod
  def parse_args(cls, args: argparse.Namespace) -> BaseBrowserVariantsConfig:
    pass

  def __init__(
      self,
      browser_lookup_override: Optional[BrowserLookupTableT] = None) -> None:
    self.flags_config: FlagsConfig = FlagsConfig()
    self._variants: list[BrowserVariantConfig] = []
    self._unique_labels: Set[str] = set()
    self._browser_lookup_override = browser_lookup_override or {}

  @property
  def variants(self) -> list[BrowserVariantConfig]:
    assert self._variants
    return list(self._variants)

  @property
  def browsers(self) -> list[Browser]:
    browsers = [
        variant.browser_cls(variant.label, variant.path, variant.settings)
        for variant in self._variants
    ]
    self._ensure_unique_browser_names(browsers)
    return browsers

  def __len__(self) -> int:
    return len(self._variants)

  def __bool__(self) -> bool:
    return bool(self._variants)

  def extend(self, other: BaseBrowserVariantsConfig) -> None:
    if self is other:
      raise ValueError(f"Cannot extend {type(self)} with itself.")
    self._variants.extend(other.variants)

  def _ensure_unique_browser_names(self, browsers: list[Browser]) -> None:
    if self._has_unique_variant_names(browsers):
      return
    # Expand to full version names
    for browser in browsers:
      browser.unique_name = (
          f"{browser.type_name()}_{browser.version}_{browser.label}")
    if self._has_unique_variant_names(browsers):
      return
    logging.info("Got unique browser names and versions, "
                 "please use --browser-config for more meaningful names")
    # Last resort, add index
    for index, browser in enumerate(browsers):
      browser.unique_name = f"{browser.unique_name[:MAX_LABEL_LEN]}_{index}"
    assert self._has_unique_variant_names(browsers)

  def _has_unique_variant_names(self, browsers: list[Browser]) -> bool:
    names = [browser.unique_name for browser in browsers]
    unique_names = set(names)
    return len(unique_names) == len(names)

  def _is_valid_browser_path(self, browser_config: BrowserConfig) -> bool:
    if browser_config.is_remote:
      # TODO: add remote path validation
      return True
    return pth.LocalPath(browser_config.path).exists()

  def _flags_to_label(self, name: str, flags: Flags) -> str:
    return f"{name}_{convert_flags_to_label(*flags)}"

  def _create_unique_variant_labels(
      self, name: str, raw_browser_data: str | dict[str, Any],
      flag_variants: FlagsGroupConfig) -> dict[FlagsVariantConfig, str]:
    labels_lookup: dict[FlagsVariantConfig, str] = {}
    group_labels = set(variant.label for variant in flag_variants)
    use_unique_variant_label = len(group_labels) == len(flag_variants)

    for variant in flag_variants:
      label = name
      if isinstance(raw_browser_data, dict):
        label = raw_browser_data.get("label", name)
      if len(flag_variants) > 1:
        if use_unique_variant_label:
          label = f"{name}_{variant.label}"
        else:
          # TODO: This case might not happen anymore
          label = self._flags_to_label(name, variant.flags)
      labels_lookup[variant] = label[:MAX_LABEL_LEN]
    return labels_lookup

  def _check_unique_label(self, label: str) -> bool:
    if label in self._unique_labels:
      return False
    self._unique_labels.add(label)
    return True

  def _validate_flags(self, browser_name: str,
                      flag_group_names: list[str]) -> None:
    if isinstance(flag_group_names, str):
      flag_group_names = [flag_group_names]
    if not isinstance(flag_group_names, list):
      raise ConfigError(
          f"'flags' is not a list for browser={repr(browser_name)}")
    ObjectParser.unique_sequence(flag_group_names, error_cls=ConfigError)

  def _log_browser_variants(self, name: str,
                            flag_variants: FlagsGroupConfig) -> None:
    logging.info("🌐 SELECTED BROWSER: '%s' with %s flag variants:", name,
                 len(flag_variants))
    for i, variant in enumerate(flag_variants):
      logging.info("   %s: %s", i, variant.flags)

  @classmethod
  def get_browser_cls(cls, browser_config: BrowserConfig) -> Type[Browser]:
    driver = browser_config.driver.type
    path: pth.AnyPath = browser_config.path
    assert not isinstance(path, str), "Invalid path"
    if not BrowserConfig.is_supported_browser_path(path):
      raise argparse.ArgumentTypeError(f"Unsupported browser path='{path}'")
    path_str = str(browser_config.path).lower()
    if "safari" in path_str:
      return cls.get_safari_browser_cls(browser_config)
    if "webview" in path_str:
      return all_browsers.WebviewBrowser
    if "chrome" in path_str:
      return cls.get_chrome_browser_cls(browser_config)
    if "chromium" in path_str:
      return cls.get_chromium_browser_cls(browser_config)
    if "firefox" in path_str:
      if driver == BrowserDriverType.WEB_DRIVER:
        return all_browsers.FirefoxWebDriver
    if "edge" in path_str:
      return all_browsers.EdgeWebDriver
    if any(embedder in path_str for embedder in SUPPORTED_EMBEDDER):
      return all_browsers.WebviewEmbedder
    if "d8" in path_str:
      return all_browsers.D8
    raise argparse.ArgumentTypeError(f"Unsupported browser path='{path}'")

  @classmethod
  def get_safari_browser_cls(cls,
                             browser_config: BrowserConfig) -> Type[Browser]:
    driver = browser_config.driver.type
    if driver == BrowserDriverType.IOS:
      return all_browsers.SafariWebdriverIOS
    if driver == BrowserDriverType.WEB_DRIVER:
      return all_browsers.SafariWebDriver
    if driver == BrowserDriverType.APPLE_SCRIPT:
      return all_browsers.SafariAppleScript
    raise argparse.ArgumentTypeError(f"Unsupported Safari driver: {driver}")

  @classmethod
  def get_chrome_browser_cls(cls,
                             browser_config: BrowserConfig) -> Type[Browser]:
    driver = browser_config.driver.type
    if driver == BrowserDriverType.WEB_DRIVER:
      return all_browsers.ChromeWebDriver
    if driver == BrowserDriverType.APPLE_SCRIPT:
      return all_browsers.ChromeAppleScript
    if driver == BrowserDriverType.ANDROID:
      if all_browsers.LocalChromeWebDriverAndroid.is_apk_helper(
          browser_config.path):
        return all_browsers.LocalChromeWebDriverAndroid
      return all_browsers.ChromeWebDriverAndroid
    if driver == BrowserDriverType.LINUX_SSH:
      return all_browsers.ChromeWebDriverSsh
    if driver == BrowserDriverType.CHROMEOS_SSH:
      return all_browsers.ChromeWebDriverChromeOsSsh
    raise argparse.ArgumentTypeError(f"Unsupported Chrome driver: {driver}")

  @classmethod
  def get_chromium_browser_cls(cls,
                               browser_config: BrowserConfig) -> Type[Browser]:
    driver = browser_config.driver.type
    # TODO: technically this should be ChromiumWebDriver
    if driver == BrowserDriverType.WEB_DRIVER:
      return all_browsers.ChromiumWebDriver
    if driver == BrowserDriverType.APPLE_SCRIPT:
      return all_browsers.ChromiumAppleScript
    if driver == BrowserDriverType.ANDROID:
      if all_browsers.LocalChromiumWebDriverAndroid.is_apk_helper(
          browser_config.path):
        return all_browsers.LocalChromiumWebDriverAndroid
      return all_browsers.ChromiumWebDriverAndroid
    if driver == BrowserDriverType.LINUX_SSH:
      return all_browsers.ChromiumWebDriverSsh
    if driver == BrowserDriverType.CHROMEOS_SSH:
      return all_browsers.ChromiumWebDriverChromeOsSsh
    raise argparse.ArgumentTypeError(f"Unsupported chromium driver: {driver}")

  def _get_browser_platform(self,
                            browser_config: BrowserConfig) -> plt.Platform:
    return browser_config.get_platform()

  def _config_for_maybe_downloaded_binary(self,
                               browser_config: BrowserConfig) -> BrowserConfig:
    path_or_identifier = browser_config.browser
    if isinstance(path_or_identifier, pth.AnyPath):
      return browser_config
    browser_platform: plt.Platform = self._get_browser_platform(browser_config)
    if ChromeDownloader.is_valid(path_or_identifier, browser_platform):
      downloaded = ChromeDownloader.load(path_or_identifier, browser_platform)
    elif FirefoxDownloader.is_valid(path_or_identifier, browser_platform):
      downloaded = FirefoxDownloader.load(path_or_identifier, browser_platform)
    else:
      raise ValueError(
          f"No version-download support for browser: {path_or_identifier}")
    return BrowserConfig(downloaded, browser_config.driver)

  def _get_driver_path(self, args: argparse.Namespace,
                       browser_config: BrowserConfig) -> Optional[pth.AnyPath]:
    if browser_config.driver.is_remote:
      return args.remote_driver_path or browser_config.driver.path
    return args.driver_path or browser_config.driver.path


  def _append_variant(self, args: argparse.Namespace, label: str,
                      browser_cls: Type[Browser], browser_config: BrowserConfig,
                      flags: Flags, browser_platform: plt.Platform,
                      network: Network,
                      env_config: EnvConfig) -> BrowserVariantConfig:
    if not self._is_valid_browser_path(browser_config):
      raise ConfigError(f"Browser binary does not exist: {browser_config.path}")
    assert label
    browser_cache_dir = args.browser_cache_dir or browser_config.cache_dir
    clear_cache_dir: bool | None = args.clear_browser_cache_dir
    if clear_cache_dir is None:
      clear_cache_dir = browser_config.clear_cache
    if clear_cache_dir is None:
      clear_cache_dir = True
    settings = Settings(
        cache_dir=browser_cache_dir,
        clear_cache_dir=clear_cache_dir,
        flags=flags,
        network=network,
        driver_path=self._get_driver_path(args, browser_config),
        viewport=args.viewport,
        splash_screen=args.splash_screen,
        platform=browser_platform,
        secrets=args.secrets,
        driver_logging=args.driver_logging,
        wipe_system_user_data=args.wipe_system_user_data,
        http_request_timeout=args.http_request_timeout,
        env_config=env_config,
        extensions=browser_config.extensions)
    browser_variant = BrowserVariantConfig(label, browser_cls, browser_config,
                                           settings)
    if not self._check_unique_label(label):
      raise ConfigError(f"Got non-unique label: {repr(label)}")
    self._variants.append(browser_variant)
    return browser_variant

  def _get_browser_network(self, args: argparse.Namespace,
                           browser_config: BrowserConfig,
                           browser_platform: plt.Platform) -> Network:
    with exception.annotate_argparsing("Creating network config"):
      network_config = browser_config.network or args.network
      if not isinstance(network_config, NetworkConfig):
        network_config = NetworkConfig.parse(network_config)
      return network_config.create(browser_platform)
    raise exception.UnreachableError()

  def _get_browser_env_config(self, args: argparse.Namespace,
                              browser_config: BrowserConfig) -> EnvConfig:
    if env_config := browser_config.env:
      return env_config
    return args.env


class BrowserVariantsConfig(BaseBrowserVariantsConfig):

  @classmethod
  @override
  def parse_args(cls, args: argparse.Namespace) -> BaseBrowserVariantsConfig:
    browser_variants = cls()
    if args.browser_config:
      browser_variants.extend(BrowserVariantsConfigDict.parse_args(args))
    if args.browser:
      browser_variants.extend(BrowserVariantConfigArgs.parse_args(args))
    if browser_variants:
      return browser_variants
    return cls.default(args)

  @classmethod
  def default(cls, args: argparse.Namespace) -> BrowserVariantConfigArgs:
    # Make sure we have at least one default browser as variant.
    default_variants = BrowserVariantConfigArgs()
    default_variants.parse_sequence(args, [BrowserConfig.default()])
    return default_variants


class BrowserVariantsConfigDict(BaseBrowserVariantsConfig):

  @classmethod
  @override
  def parse_args(cls, args: argparse.Namespace) -> Self:
    config_variants = cls()
    with late_argument_type_error_wrapper("--browser-config"):
      path = args.browser_config.expanduser().absolute()
      config_variants.parse_config_path(path, args)
    return config_variants

  def __init__(self,
               raw_config_data: Optional[dict[str, Any]] = None,
               browser_lookup_override: Optional[BrowserLookupTableT] = None,
               args: Optional[argparse.Namespace] = None) -> None:
    super().__init__(browser_lookup_override)
    if raw_config_data:
      assert args, "args object needed when loading from dict."
      self.parse_dict(raw_config_data, args)

  def parse_config_path(self, path: pth.LocalPath,
                        args: argparse.Namespace) -> None:
    with ChangeCWD(path.parent):
      with path.open(encoding="utf-8") as f:
        self.parse_text_io(f, args)

  def parse_text_io(self, f: TextIO, args: argparse.Namespace) -> None:
    with exception.annotate(f"Loading browser config file: {f.name}"):
      config = {}
      with exception.annotate("Parsing hjson"):
        config = cb_hjson.load_unique_keys(f)
      with exception.annotate(f"Parsing config file: {f.name}"):
        self.parse_dict(config, args)

  def parse_dict(self, config: dict[str, Any],
                 args: argparse.Namespace) -> None:
    with exception.annotate(
        f"Parsing {type(self).__name__} dict", throw_cls=ConfigError):
      if "flags" in config:
        with exception.annotate("Parsing config['flags']"):
          self.flags_config = FlagsConfig.parse(config["flags"])
      if "browsers" not in config:
        raise ConfigError("Config does not provide a 'browsers' dict.")
      if not config["browsers"]:
        raise ConfigError("Config contains empty 'browsers' dict.")
      with exception.annotate("Parsing config['browsers']"):
        self._parse_browsers(config["browsers"], args)

  def _parse_browsers(self, data: dict[str, Any],
                      args: argparse.Namespace) -> None:
    for name, browser_config in data.items():
      with exception.annotate(f"Parsing browsers[{repr(name)}]"):
        self._parse_browser(name, browser_config, args)

  def _parse_browser(self, name: str, raw_browser_data: Any,
                     args: argparse.Namespace) -> None:
    if not isinstance(raw_browser_data, (dict, str)):
      raise argparse.ArgumentTypeError(
          f"Expected str or dict, got {type(raw_browser_data).__name__}: "
          f"{repr(raw_browser_data)}")

    path_or_identifier: str | None = None
    if isinstance(raw_browser_data, dict):
      path_or_identifier = raw_browser_data.get("path")
    else:
      path_or_identifier = raw_browser_data
    browser_cls: Type[Browser] | None = None
    if path_or_identifier and (path_or_identifier
                               in self._browser_lookup_override):
      browser_cls, browser_config = self._browser_lookup_override[
          path_or_identifier]
    else:
      browser_config = self._config_for_maybe_downloaded_binary(
          cast(BrowserConfig, BrowserConfig.parse(raw_browser_data)))
      browser_cls = self.get_browser_cls(browser_config)
    assert browser_cls

    flag_variants: FlagsGroupConfig = self._get_browser_variants(
        args, name, raw_browser_data)
    self._log_browser_variants(name, flag_variants)
    browser_platform: plt.Platform = self._get_browser_platform(browser_config)
    labels_lookup: dict[FlagsVariantConfig,
                        str] = self._create_unique_variant_labels(
                            name, raw_browser_data, flag_variants)
    for variant in flag_variants:
      label = labels_lookup[variant]
      # This will take the newest flag implementation by default.
      browser_flags = browser_cls.default_flags(variant.flags)
      network: Network = self._get_browser_network(args, browser_config,
                                                   browser_platform)
      env_config: EnvConfig = self._get_browser_env_config(args, browser_config)
      self._append_variant(args, label, browser_cls, browser_config,
                           browser_flags, browser_platform, network, env_config)

  def _get_browser_variants(
      self, args: argparse.Namespace, browser_name: str,
      raw_browser_data: str | dict[str, Any]) -> FlagsGroupConfig:
    default_variant = FlagsVariantConfig(DEFAULT_LABEL)
    flag_variants = FlagsGroupConfig((default_variant,))
    if not isinstance(raw_browser_data, dict):
      return flag_variants
    flag_groups: list[FlagsGroupConfig] = []
    with exception.annotate(f"Parsing browsers[{repr(browser_name)}].flags"):
      flag_groups = self._parse_browser_flags(browser_name, raw_browser_data)
    with exception.annotate(
        f"Expand browsers[{repr(browser_name)}].flags into full variants"):
      flag_variants = flag_variants.product(*flag_groups)

    if args.browser:
      # If there are additional --browser arguments, all browser flags are
      # consumed there
      return flag_variants
    # Create variants for the existing browser command line flags and
    # create the product.
    args_flag_groups: FlagsGroupConfig = FlagsGroupConfig.parse_args(args)
    flag_variants = flag_variants.product(args_flag_groups)
    return flag_variants

  def _parse_browser_flags(self, browser_name: str,
                           data: dict[str, Any]) -> list[FlagsGroupConfig]:
    flag_group_names = data.get("flags", [])
    if isinstance(flag_group_names, str):
      flag_group_names = [flag_group_names]
    self._validate_flags(browser_name, flag_group_names)
    inline_flags = Flags()
    flag_groups: list[FlagsGroupConfig] = []
    for flag_group_name in flag_group_names:
      if flag_group_name.startswith("--"):
        inline_flags.update(Flags.parse(flag_group_name))
      else:
        maybe_flag_group = self.flags_config.get(flag_group_name, None)
        if maybe_flag_group is None:
          raise ConfigError(
              f"group={repr(flag_group_name)} "
              f"for browser={repr(browser_name)} does not exist.\n"
              f"Choices are: {list(self.flags_config.keys())}")
        flag_groups.append(maybe_flag_group)
    if inline_flags:
      flag_data = {"inline": inline_flags}
      flag_groups.append(FlagsGroupConfig.parse_dict(flag_data))
    return flag_groups


class BrowserVariantConfigArgs(BaseBrowserVariantsConfig):

  @classmethod
  @override
  def parse_args(cls, args: argparse.Namespace) -> Self:
    args_variants = cls()
    with late_argument_type_error_wrapper("--browser"):
      args_variants.parse_sequence(args, args.browser)
    return args_variants

  def parse_sequence(self, args: argparse.Namespace,
                     browsers: Sequence[BrowserConfig]) -> None:
    browsers = ObjectParser.unique_sequence(browsers, "--browser arguments")
    for i, browser in enumerate(browsers):
      with exception.annotate(f"Append browser {i}"):
        self._append_browser(args, browser)
    self._verify_browser_flags(args)

  def _append_browser(self, args: argparse.Namespace,
                      browser_config: BrowserConfig) -> None:
    assert browser_config, "Expected non-empty BrowserConfig."
    browser_config = self._config_for_maybe_downloaded_binary(browser_config)
    browser_cls: Type[Browser] = self.get_browser_cls(browser_config)
    args_variants = FlagsGroupConfig.parse_args(args)
    browser_platform: plt.Platform = self._get_browser_platform(browser_config)
    network: Network = self._get_browser_network(args, browser_config,
                                                 browser_platform)
    env_config: EnvConfig = self._get_browser_env_config(args, browser_config)

    name = f"{browser_platform}_{len(self._unique_labels)}"
    for flag_variant in args_variants:
      label: str = name
      flags: Flags = flag_variant.flags
      if len(args_variants) > 1:
        label = self._flags_to_label(label, flags)
      browser_variant = self._append_variant(args, label, browser_cls,
                                             browser_config, flags,
                                             browser_platform, network,
                                             env_config)
      logging.info("🌐 SELECTED BROWSER: name=%s path='%s' ",
                   browser_variant.label, browser_variant.path)

  def _verify_browser_flags(self, args: argparse.Namespace) -> None:
    args_variants = FlagsGroupConfig.parse_args(args)
    for flag_variant in args_variants:
      chrome_flags = flag_variant.flags
      for flag_name, value in chrome_flags.items():
        if not value:
          continue
        for variant in self._variants:
          browser_cls = variant.browser_cls
          if not browser_cls.attributes().is_chromium_based:
            raise argparse.ArgumentTypeError(
                f"Used chrome/chromium-specific flags {flag_name} "
                f"for non-chrome {browser_cls.type_name()}.\n"
                "Use --browser-config for complex variants.")
    browser_types = set(
        variant.browser_cls.type_name() for variant in self._variants)
    if len(browser_types) == 1:
      return
    if args.driver_path:
      raise argparse.ArgumentTypeError(
          f"Cannot use custom --driver-path='{args.driver_path}' "
          f"for multiple browser {browser_types}.")
    if args.remote_driver_path:
      raise argparse.ArgumentTypeError(
          f"Cannot use custom --remote-driver-path='{args.remote_driver_path}' "
          f"for multiple browser {browser_types}.")
    if args.other_browser_args:
      raise argparse.ArgumentTypeError(
          f"Multiple browser types {browser_types} "
          "cannot be used with common extra browser flags: "
          f"{args.other_browser_args}.\n"
          "Use --browser-config for complex variants.")
