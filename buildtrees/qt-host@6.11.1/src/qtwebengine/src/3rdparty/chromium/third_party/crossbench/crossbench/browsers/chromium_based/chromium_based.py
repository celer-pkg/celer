# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import logging
from typing import TYPE_CHECKING, Final, Optional, Type, cast

from typing_extensions import override

from crossbench import path as pth
from crossbench.browsers.browser import Browser
from crossbench.browsers.browser_helper import convert_flags_to_label
from crossbench.browsers.chromium_based import helper
from crossbench.browsers.version import BrowserVersionChannel
from crossbench.browsers.viewport import Viewport
from crossbench.flags.chrome import ChromeFlags
from crossbench.types import JsonDict

if TYPE_CHECKING:
  from crossbench.browsers.chromium.version import ChromiumVersion
  from crossbench.browsers.settings import Settings
  from crossbench.browsers.version import BrowserVersion
  from crossbench.flags.base import Flags, FlagsData
  from crossbench.flags.chrome import ChromeFeatures
  from crossbench.flags.js_flags import JSFlags
  from crossbench.runner.groups.session import BrowserSessionRunGroup


class ChromiumBased(Browser):
  MIN_HEADLESS_NEW_VERSION: int = 112
  MIN_BENCHMARKING_EXTENSION_FLAG_MILESTONE: Final[int] = 139
  DEFAULT_FLAGS: tuple[str, ...] = (
      "--no-default-browser-check",
      "--disable-component-update",
      "--disable-sync",
      "--no-first-run",
      # This could be enabled via feature-flags as well.
      "--disable-search-engine-choice-screen",
  )
  FLAGS_FOR_DISABLING_BACKGROUND_INTERVENTIONS: tuple[str, ...] = (
      "--disable-background-timer-throttling",
      "--disable-renderer-backgrounding",
  )
  # Versions [M98, M100] don't respect the --no-first-run flag and always
  # display a "What's New" tab on startup.
  WHATS_NEW_UI_VERSION_RANGE: Final[range] = range(98, 100 + 1)


  @classmethod
  @abc.abstractmethod
  def version_cls(cls) -> Type[ChromiumVersion]:
    pass

  @classmethod
  @override
  def default_flags(cls,
                    initial_data: FlagsData = None,
                    milestone: int = 0) -> ChromeFlags:
    return ChromeFlags.for_milestone(initial_data, milestone)

  def __init__(self,
               label: str,
               path: pth.AnyPath,
               settings: Optional[Settings] = None) -> None:
    super().__init__(label, path, settings=settings)
    self._local_extension_tmp_dir: Optional[pth.LocalPath] = None
    self._remote_extension_tmp_dir: Optional[pth.AnyPath] = None
    assert isinstance(self._flags, ChromeFlags)

  @override
  def _extract_version(self) -> BrowserVersion:
    if path := self.path:
      self._is_local_build = helper.is_in_build_dir(path, self.platform)
    version = self.version_cls().parse(self.platform.app_version(self.path))
    # Locally-built chrome versions should not have a channel
    if self.is_local_build:
      version = version.with_channel(BrowserVersionChannel.ANY)
    return version

  @override
  def _init_flags(self, settings: Settings) -> ChromeFlags:
    flags: Flags = settings.flags
    js_flags: Flags = settings.js_flags
    self._flags = self.default_flags(self.DEFAULT_FLAGS, self.version.major)
    self._flags.update(flags)

    if not settings.extensions:
      self._flags.set("--disable-extensions")

    if "--allow-background-interventions" in self._flags.data:
      # The --allow-background-interventions flag should have no value.
      assert self._flags.get("--allow-background-interventions") is None
    else:
      logging.warning(
          "Disabling background interventions for chromium based browser. "
          "Tests that rely on correct tab discarding or prioritization "
          "behavior may not work as expected. Add "
          "--allow-background-interventions to bypass this.")
      self._flags.update(self.FLAGS_FOR_DISABLING_BACKGROUND_INTERVENTIONS)

    if self.version.major in self.WHATS_NEW_UI_VERSION_RANGE:
      whatsnew_ui_feature = "ChromeWhatsNewUI"
      if not self._flags.features:
        logging.warning("Disabling %s", whatsnew_ui_feature)
        self._flags.features.disable(whatsnew_ui_feature)
      elif whatsnew_ui_feature not in self._flags.features:
        logging.warning("Browser might show %s, hiding the main tab",
                        whatsnew_ui_feature)

    # Explicitly disable field-trials by default on all chrome flavours:
    # By default field-trials are disabled on non-Chrome branded builds, but
    # are auto-enabled on everything else. This gives very confusing results
    # when comparing local builds to official binaries.
    field_trial_flags: ChromeFlags = self._flags.field_trial_enable_flags
    if not field_trial_flags:
      logging.info("Disabling experiments/finch/field-trials for %s", self)
      for flag in ChromeFlags.FIELD_TRIAL_DISABLE_FLAGS:
        self._flags.set(flag)
    else:
      logging.warning("Running with field-trials or finch experiments.")

    self.js_flags.update(js_flags)
    self._maybe_disable_gpu_compositing()
    # Run early validation for conflicting command-line flags.
    self.validate_flags()
    return self._flags

  def _maybe_disable_gpu_compositing(self) -> None:
    # Chrome Remote Desktop provides no GPU and older chrome versions
    # don't handle this well.
    if self.version.major > 92 or ("CHROME_REMOTE_DESKTOP_SESSION"
                                   not in self.platform.environ):
      return
    self.flags.set("--disable-gpu-compositing")
    self.flags.set("--no-sandbox")

  @override
  def validate_flags(self) -> None:
    super().validate_flags()
    self.flags.validate()

  @override
  def _setup_cache_dir(self) -> Optional[pth.AnyPath]:
    # See documentation for more details:
    # https://chromium.googlesource.com/chromium/src/+/main/docs/user_data_dir.md
    # We only deal with the user-data-dir here and ignore the user-cache-dir.
    user_data_dir = self.settings.cache_dir
    if flag_user_data_dir := self._flags.get("--user-data-dir", None):
      if user_data_dir and str(user_data_dir) != str(flag_user_data_dir):
        raise ValueError("Conflicting cache_dir from "
                         f"settings.cache_dir={repr(str(user_data_dir))} and "
                         f"--user-data-dir={repr(str(flag_user_data_dir))}")
      return pth.AnyPath(flag_user_data_dir)

    if user_data_dir:
      return user_data_dir

    if self.platform.is_android:
      # On Android, not all apps have permission to write to /data/local/tmp,
      # so we can't just use a temp dir for user data as on other platforms.
      # We can create a subdir in Chromium's default data dir, but that will
      # be erased by chromedriver on session start.
      # Another option is a folder on external storage, but access to external
      # storage can be slow and this affects Chromium performance.
      # So the only reliable thing for now is to keep Chromium using default
      # user data dir. Note that unless --keep-browser-cache is specified,
      # all user data is cleared by chromedriver before each browser session.
      return None

    # Using a temp-dir on macos also forces the user-cache-dir to be there.
    user_data_dir = self.platform.mkdtemp(prefix=f"{self.type_name()}_")
    return user_data_dir

  @property
  def user_data_dir(self) -> Optional[pth.AnyPath]:
    # On chromium-based browsers we can have two separate caching dirs:
    # - user-data-dir containing all profile data
    # - cache-dir containing profile independent caches
    return self._cache_dir

  @property
  def is_headless(self) -> bool:
    return "--headless" in self._flags

  @property
  def chrome_log_file(self) -> pth.AnyPath:
    assert self.log_file
    return self.log_file.with_suffix(f".{self.type_name()}.log")

  @property
  @override
  def flags(self) -> ChromeFlags:
    return cast(ChromeFlags, self._flags)

  @property
  @override
  def js_flags(self) -> JSFlags:
    return cast(ChromeFlags, self._flags).js_flags

  @property
  @override
  def features(self) -> ChromeFeatures:
    return cast(ChromeFlags, self._flags).features

  @override
  def details_json(self) -> JsonDict:
    details: JsonDict = super().details_json()
    if self.log_file:
      log = cast(JsonDict, details["log"])
      log[self.type_name()] = str(self.chrome_log_file)
      log["stdout"] = str(self.stdout_log_file)
    details["js_flags"] = tuple(self.js_flags)
    return details

  def _process_extensions(self) -> dict[str, str]:
    assert not self._local_extension_tmp_dir
    self._local_extension_tmp_dir = pth.LocalPath(self.host_platform.mkdtemp())

    load_extension: list[str] = []
    extension_paths: list[pth.LocalPath] = []
    for extension in self.settings.extensions:
      unpacked = extension.get_unpacked(self.version.version_str,
                                        self._local_extension_tmp_dir,
                                        self.host_platform)
      extension_paths.append(unpacked)
      load_extension.append(str(unpacked))

    if self.platform.is_remote:
      # Create a folder to load the extensions from on the remote device.
      assert not self._remote_extension_tmp_dir
      self._remote_extension_tmp_dir = self.platform.mkdtemp()
      # Android needs executable permission on the temp folder.
      self.platform.chmod(self._remote_extension_tmp_dir, 0o755)

      # Push all the extensions to the device and update the paths we will pass
      # to Chrome.
      load_extension.clear()
      for extension_path in extension_paths:
        remote_path = self._remote_extension_tmp_dir / extension_path.name
        logging.info("Pushing extension to device: %s", extension_path)
        self.platform.push(extension_path, remote_path)
        load_extension.append(str(remote_path))
    assert not any("," in e for e in load_extension), "comma in extension path"
    load_extension_str = ",".join(load_extension)

    return {
        "--load-extension": load_extension_str,
        "--disable-features": "DisableLoadExtensionCommandLineSwitch",
    }

  @override
  def _get_browser_flags_for_session(
      self, session: BrowserSessionRunGroup) -> tuple[str, ...]:
    js_flags_copy = self.js_flags.copy()
    js_flags_copy.update(session.extra_js_flags)

    flags_copy = self.flags.copy()
    flags_copy.update(session.extra_flags)
    flags_copy.update(self.network.extra_flags(self.attributes()))
    self._handle_viewport_flags(flags_copy)

    if len(js_flags_copy):
      flags_copy["--js-flags"] = str(js_flags_copy)
    if user_data_dir := self.flags.get("--user-data-dir"):
      assert user_data_dir == str(
          self.cache_dir), (f"--user-data-dir path: {user_data_dir} was passed "
                            f"but does not match cache-dir: {self.cache_dir}")
    if self.cache_dir:
      flags_copy["--user-data-dir"] = str(self.cache_dir)
    if self.log_file:
      flags_copy.set("--enable-logging")
      flags_copy["--log-file"] = str(self.chrome_log_file)

    if self.settings.extensions:
      flags_copy.update(self._process_extensions())

    flags_copy = self._filter_flags_for_run(flags_copy)

    return tuple(flags_copy)

  def _handle_viewport_flags(self, flags: Flags) -> None:
    self._sync_viewport_flag(flags, "--start-fullscreen",
                             self.viewport.is_fullscreen, Viewport.FULLSCREEN)
    self._sync_viewport_flag(flags, "--start-maximized",
                             self.viewport.is_maximized, Viewport.MAXIMIZED)
    self._sync_viewport_flag(flags, "--headless", self.viewport.is_headless,
                             Viewport.HEADLESS)
    # M112 added --headless=new as replacement for --headless
    if "--headless" in flags and (self.version.major
                                  >= self.MIN_HEADLESS_NEW_VERSION):
      if flags["--headless"] is None:
        logging.info("Replacing --headless with --headless=new")
        flags.set("--headless", "new", should_override=True)

    if self.viewport.is_default:
      update_viewport = False
      width, height = self.viewport.size
      x, y = self.viewport.position
      if "--window-size" in flags:
        update_viewport = True
        width, height = map(int, flags["--window-size"].split(","))
      if "--window-position" in flags:
        update_viewport = True
        x, y = map(int, flags["--window-position"].split(","))
      if update_viewport:
        self.viewport = Viewport(width, height, x, y)
    if self.viewport.has_size:
      flags["--window-size"] = f"{self.viewport.width},{self.viewport.height}"
      flags["--window-position"] = f"{self.viewport.x},{self.viewport.y}"
    else:
      for flag in ("--window-position", "--window-size"):
        if flag in flags:
          flag_value = flags[flag]
          raise ValueError(f"Viewport {self.viewport} conflicts with flag "
                           f"{flag}={flag_value}")

  def get_label_from_flags(self) -> str:
    return convert_flags_to_label(*self.flags, *self.js_flags)

  @override
  def quit(self) -> None:
    try:
      super().quit()
    finally:
      if self._local_extension_tmp_dir:
        self.host_platform.rm(self._local_extension_tmp_dir, dir=True)
        self._local_extension_tmp_dir = None
      if self._remote_extension_tmp_dir:
        self.platform.rm(self._remote_extension_tmp_dir, dir=True)
        self._remote_extension_tmp_dir = None
