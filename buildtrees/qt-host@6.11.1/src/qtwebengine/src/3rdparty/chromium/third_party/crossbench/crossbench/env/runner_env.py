# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import datetime as dt
import logging
import os
from typing import TYPE_CHECKING, Iterable, Optional

from crossbench import plt
from crossbench.cli.config.env import EnvConfig, ValidationMode
from crossbench.env.base import BaseEnv, ValidationError
from crossbench.helper import collection_helper, url_helper
from crossbench.parse import ObjectParser

if TYPE_CHECKING:
  from crossbench import path as pth
  from crossbench.browsers.browser import Browser
  from crossbench.plt.base import Platform
  from crossbench.probes.probe import Probe

STALE_RESULT_ICONS = {
    75: "👻",
    100: "👾",
    125: "🎃",
    150: "👹",
    200: "💀",
    250: "😱",
    500: "🤯",
    1000: "🧙🏼‍♂️",
}


class RunnerEnv(BaseEnv):
  """
  RunnerEnvironment can check and enforce certain settings on the host where
  the benchmarks runner is.

  Use ValidationMode to change how warnings/errors are handled.
  """

  def __init__(self,
               platform: Platform,
               out_dir: pth.LocalPath,
               browsers: Iterable[Browser],
               probes: Iterable[Probe],
               repetitions: int,
               config: Optional[EnvConfig] = None,
               validation_mode: ValidationMode = ValidationMode.THROW) -> None:
    super().__init__(platform, config, validation_mode)
    self._wait_until: dt.datetime = dt.datetime.now()
    self._out_dir: pth.LocalPath = out_dir
    self._browsers: tuple[Browser, ...] = tuple(browsers)
    self._probes = tuple(probes)
    self._repetitions: int = repetitions

  @property
  def repetitions(self) -> int:
    return self._repetitions

  @property
  def browsers(self) -> tuple[Browser, ...]:
    return self._browsers

  def _add_min_delay(self, seconds: float) -> None:
    end_time = dt.datetime.now() + dt.timedelta(seconds=seconds)
    self._wait_until = max(self._wait_until, end_time)

  def _wait_min_time(self) -> None:
    delta = self._wait_until - dt.datetime.now()
    if delta > dt.timedelta(0):
      self._platform.sleep(delta)

  def validate_url(self,
                   url: str,
                   platform: Optional[plt.Platform] = None) -> bool:
    if self._validation_mode == ValidationMode.SKIP:
      return True
    platform = platform or plt.PLATFORM
    result = ObjectParser.url(url)
    if result.scheme == "file":
      return platform.exists(result.path)
    if platform.is_remote and result.hostname in ("localhost", "127.0.0.1"):
      # TODO: support remote URL verification, for now we just assume that
      # checking a live site is ok.
      return True
    if not all([result.scheme in ["http", "https"], result.netloc]):
      return False
    if self._validation_mode != ValidationMode.PROMPT:
      return True
    try:
      url_helper.get(url, timeout=5)
      return True
    except url_helper.HTTPError as e:
      logging.debug("Could not load URL '%s', got %s", url, e)
      return False

  def _check_system_monitoring(self) -> None:
    # TODO(cbruni): refactor to use list_... and disable_system_monitoring api
    if self._platform.is_macos:
      any_browser_on_macos = any(
          browser.platform.is_macos for browser in self.browsers)
      if any_browser_on_macos:
        self._check_crowdstrike()

  def _check_crowdstrike(self) -> None:
    """Crowdstrike security monitoring (for googlers go/crowdstrike-falcon) can
    have quite terrible overhead for each file-access. Disable it to reduce
    flakiness. """
    is_disabled = False
    force_disable = self._config.system_allow_monitoring is False
    try:
      # TODO(cbruni): refactor to use list_... and disable_system_monitoring api
      is_disabled = self._platform.check_system_monitoring(force_disable)
      if force_disable:
        # Add cool-down period, crowdstrike caused CPU usage spikes
        self._add_min_delay(5)
    except plt.SubprocessError as e:
      self.handle_validation_warning(
          "Could not disable go/crowdstrike-falcon monitor which can cause"
          f" high background CPU usage: {e}")
      return
    if not is_disabled:
      self.handle_validation_warning(
          "Crowdstrike monitoring is running, "
          "which can impact startup performance drastically.\n"
          "Use the following command to disable it manually:\n"
          "sudo /Applications/Falcon.app/Contents/Resources/falconctl unload\n")

  def _check_disk_space(self) -> None:
    limit = self._config.disk_min_free_space_gib
    if limit is EnvConfig.IGNORE:
      return
    # Check the remaining disk space on the FS where we write the results.
    usage = self._platform.disk_usage(self._out_dir)
    free_gib = round(usage.free / 1024 / 1024 / 1024, 2)
    if free_gib < limit:
      self.handle_validation_warning(
          f"Only {free_gib}GiB disk space left, expected at least {limit}GiB.")

  def _check_power(self) -> None:
    use_battery = self._config.power_use_battery
    if use_battery is EnvConfig.IGNORE:
      return
    battery_probes = []
    # Certain probes may require battery power:
    for probe in self._probes:
      if probe.BATTERY_ONLY:
        battery_probes.append(probe)
    if not use_battery and battery_probes:
      probes_str = ",".join(probe.name for probe in battery_probes)
      self.handle_validation_warning(
          "Requested battery_power=False, "
          f"but probes={probes_str} require battery power.")
    sys_use_battery = self._platform.is_battery_powered
    if sys_use_battery != use_battery:
      self.handle_validation_warning(
          f"Expected battery_power={use_battery}, "
          f"but the system reported battery_power={sys_use_battery}")

  def _check_cpu_usage(self) -> None:
    max_cpu_usage = self._config.cpu_max_usage_percent
    if max_cpu_usage is EnvConfig.IGNORE:
      return
    cpu_usage_percent = round(100 * self._platform.cpu_usage(), 1)
    if cpu_usage_percent > max_cpu_usage:
      self.handle_validation_warning(
          f"CPU usage={cpu_usage_percent}% is higher than "
          f"requested max={max_cpu_usage}%.")

  def _check_cpu_temperature(self) -> None:
    min_relative_speed = self._config.cpu_min_relative_speed
    if min_relative_speed is EnvConfig.IGNORE:
      return
    cpu_speed = self._platform.get_relative_cpu_speed()
    if cpu_speed < min_relative_speed:
      self.handle_validation_warning(
          "CPU thermal throttling is active. "
          f"Relative speed is {cpu_speed}, "
          f"but expected at least {min_relative_speed}.")

  def _check_system_min_uptime(self) -> None:
    min_uptime = self._config.system_min_uptime
    if min_uptime is EnvConfig.IGNORE:
      return
    if uptime := self._platform.uptime():
      if uptime < min_uptime:
        self.handle_validation_warning(
            f"Expected min system uptime {min_uptime} but got {uptime}. "
            "The OS might not be ready for a clean measurement.")

  def _check_forbidden_system_process(self) -> None:
    # Verify that no terminals are running.
    # They introduce too much overhead. (As measured with powermetrics)
    system_forbidden_process_names = self._config.system_forbidden_process_names
    if system_forbidden_process_names is EnvConfig.IGNORE:
      return
    process_found = self._platform.process_running(
        system_forbidden_process_names)
    if process_found:
      self.handle_validation_warning(
          f"Process:{process_found} found. "
          "Make sure not to have a terminal opened. Use SSH.")

  def _check_screen_autobrightness(self) -> None:
    auto_brightness = self._config.screen_allow_autobrightness
    if auto_brightness is not False:
      return
    if self._platform.check_autobrightness():
      self.handle_validation_warning(
          "Auto-brightness was found to be ON. "
          "Deactivate it in 'System Preferences/Displays'")

  def _check_cpu_power_mode(self) -> bool:
    # TODO Implement checks for performance mode
    return True

  def _check_running_binaries(self) -> None:
    if self._config.browser_allow_existing_process:
      return
    grouped_browsers: dict[plt.Platform,
                           list[Browser]] = collection_helper.group_by(
                               self.browsers,
                               key=lambda browser: browser.platform)
    for platform, browsers in grouped_browsers.items():
      self._check_running_binaries_on_platform(platform, browsers)

  def _check_running_binaries_on_platform(
      self, platform: plt.Platform, platform_browsers: list[Browser]) -> None:
    # On Android, an app's process lifetime is not controlled by the user or
    # the app itself. OS can start/terminate processes in the background, so
    # we don't check for those.
    if platform.is_android:
      return

    browser_binaries: dict[str, list[Browser]] = collection_helper.group_by(
        platform_browsers, key=lambda browser: os.fspath(browser.path))
    own_pid = os.getpid()
    for proc_info in platform.processes(["cmdline", "exe", "pid", "name"]):
      if not browser_binaries:
        return
      # Skip over this python script which might have the binary path as
      # part of the command line invocation.
      if proc_info["pid"] == own_pid:
        continue
      cmdline = " ".join(proc_info.get("cmdline") or "")
      exe = proc_info.get("exe") or proc_info.get("name")
      if not exe:
        continue
      # Windows uses some intermediate processes that contains the binary name
      # on the command line.
      if (platform.is_win and
          proc_info.get("name") in ("cmd.exe", "vpython3.exe")):
        continue
      for binary, browsers in list(browser_binaries.items()):
        # Add a white-space to get less false-positives
        if f"{binary} " not in cmdline and binary != exe:
          continue
        # Use the first in the group
        browser: Browser = browsers[0]
        logging.debug("Binary=%s Platform=%s", binary, platform)
        logging.debug("PS status output:")
        logging.debug("proc(pid=%s, name=%s, cmd=%s)", proc_info["pid"],
                      proc_info["name"], cmdline)
        self.handle_validation_warning(
            f"{browser.app_name} {browser.version} "
            f"seems to be already running on {platform}.")
        # Avoid re-checking the same binary once we've allowed it to be running.
        del browser_binaries[binary]

  def _check_screen_brightness(self) -> None:
    brightness = self._config.screen_brightness_percent
    if brightness is EnvConfig.IGNORE:
      return
    assert 0 <= brightness <= 100, f"Invalid brightness={brightness}"
    self._platform.set_main_display_brightness(brightness)
    current = self._platform.get_main_display_brightness()
    if current != brightness:
      self.handle_validation_warning(
          f"Requested main display brightness={brightness}%, "
          "but got {brightness}%")

  def _check_screen_refresh_rate(self) -> None:
    refresh_rate = self._config.screen_refresh_rate
    if not self._platform.is_macos or refresh_rate is EnvConfig.IGNORE:
      return
    success, log_msg = self._platform.set_display_refresh_rate(refresh_rate)
    if success:
      logging.debug(log_msg)
    else:
      self.handle_validation_warning(log_msg)

  def _check_headless(self) -> None:
    self._check_config_headless()
    self._check_browser_headless()

  def _check_config_headless(self) -> None:
    requested_headless = self._config.browser_is_headless
    if requested_headless is EnvConfig.IGNORE:
      return
    # Check that browsers are running in the requested headless mode:
    for browser in self.browsers:
      if browser.viewport.is_headless != requested_headless:
        self.handle_validation_warning(
            f"Requested browser_is_headless={requested_headless},"
            f"but browser {browser.unique_name} has conflicting "
            f"headless={browser.viewport.is_headless}.")
      if browser.platform.is_headless != requested_headless:
        self.handle_validation_warning(
            "Requested browser_is_headless=False, "
            f"but no display is available to run with a UI for {browser}.")

  def _check_browser_headless(self) -> None:
    for browser in self.browsers:
      if browser.viewport.is_headless:
        continue
      if browser.platform.has_display:
        continue
      self.handle_validation_warning(
          f"{browser} requires a {browser.viewport} "
          f"but no display is available on {browser.platform}. "
          "Use --headless to run chrome without a display.")

  def _check_probes(self) -> None:
    for probe in self._probes:
      try:
        probe.validate_env(self)
      except Exception as e:
        raise ValidationError(
            f"Probe='{probe.NAME}' validation failed: {e}") from e
    require_probes = self._config.require_probes
    if require_probes is EnvConfig.IGNORE:
      return
    if self._config.require_probes and not self._probes:
      self.handle_validation_warning("No probes specified.")

  def _check_results_dir(self) -> None:
    results_dir = self._out_dir.parent
    if not results_dir.exists():
      return
    results = [path for path in results_dir.iterdir() if path.is_dir()]
    num_results = len(results)
    if num_results < 20:
      return
    message = (f"Found {num_results} existing crossbench results. "
               f"Consider cleaning stale results in '{results_dir}'")
    for count, icon in reversed(STALE_RESULT_ICONS.items()):
      if num_results > count:
        message = f"{icon} {message}"
        break
    if num_results > 50:
      logging.error(message)
    else:
      logging.warning(message)

  def _check_macos_terminal(self) -> None:
    if not self._platform.is_macos or (
        self._platform.environ.get("TERM_PROGRAM") != "Apple_Terminal"):
      return
    any_not_headless = any(
        not browser.viewport.is_headless for browser in self.browsers)
    if any_not_headless:
      self.handle_validation_warning(
          "Terminal.app does not launch apps in the foreground.\n"
          "Please use iTerm.app for a better experience.")

  def _check_file_access(self) -> None:
    if self._platform.is_macos:
      has_safari = any(
          browser.attributes().is_safari for browser in self.browsers)
      if has_safari:
        self._check_safari_cache_dir_access()
    self._check_results_dir_access()

  def _check_safari_cache_dir_access(self) -> None:
    safari_cache_dir = (
        self.platform.home() /
        "Library/Containers/com.apple.Safari/Data/Library/Caches")
    if not self._has_read_write_access(safari_cache_dir):
      self._file_access_access_warning("Safari's cache directory")

  def _check_results_dir_access(self) -> None:
    out_dir = self._out_dir.parent
    if self._has_read_write_access(out_dir):
      return
    self._file_access_access_warning(f"the parent result dir: {out_dir})")

  def _has_read_write_access(self, test_dir: pth.AnyPathLike) -> bool:
    try:
      self.platform.mkdir(test_dir, exist_ok=True, parents=True)
      with self.platform.NamedTemporaryFile(
          prefix="crossbench_file_access_test", dir=test_dir) as test_file:
        self.platform.write_text(test_file, test_file.name)
        assert self.platform.read_text(test_file) == test_file.name
        self.platform.rm(test_file)
        return True
    except Exception as e:  # pylint: disable=broad-except
      logging.debug("Failed file access test: %s", e)
      return False

  def _file_access_access_warning(self, dir_name: str) -> None:
    if not self.platform.is_macos:
      self.handle_validation_warning(f"Could not modify {dir_name}")
      return

    term_program = self._platform.environ.get("TERM_PROGRAM",
                                              "the current terminal App")
    self.handle_validation_warning(
        f"Could not modify {dir_name}.\n"
        "Likely missing 'Full Disk Access' macOS Privacy & Security "
        f"permission for {term_program}.")

  def check_browser_focused(self, browser: Browser) -> None:
    if (self._config.browser_allow_background or not browser.pid or
        browser.viewport.is_headless):
      return
    info = browser.platform.foreground_process()
    if not info:
      return
    if info["pid"] != browser.pid:
      self.handle_warning(
          f"Browser(name={browser.unique_name} pid={browser.pid})) "
          "was not in the foreground at the end of the benchmark. "
          "Background apps and tabs can be heavily throttled.",
          allow_interactive=False)

  def validate(self) -> None:
    logging.info("-" * 80)
    message = "🌤️  VALIDATE ENVIRONMENT"
    if self._validation_mode == ValidationMode.SKIP:
      logging.info("%s: SKIP", message)
      return
    if self._validation_mode != ValidationMode.WARN:
      message += " (--env-validation=warn for soft warnings)"
    message += ": %s"
    logging.info(message, self._validation_mode.name)
    self._check_system_monitoring()
    self._check_power()
    self._check_disk_space()
    self._check_cpu_usage()
    self._check_cpu_temperature()
    self._check_cpu_power_mode()
    self._check_system_min_uptime()
    self._check_running_binaries()
    self._check_screen_brightness()
    self._check_screen_refresh_rate()
    self._check_headless()
    self._check_results_dir()
    self._check_probes()
    self._wait_min_time()
    self._check_forbidden_system_process()
    self._check_screen_autobrightness()
    self._check_macos_terminal()
    self._check_file_access()
