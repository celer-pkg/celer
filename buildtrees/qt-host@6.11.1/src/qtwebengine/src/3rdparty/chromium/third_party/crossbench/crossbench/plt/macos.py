# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import ctypes
import ctypes.util
import functools
import json
import logging
import plistlib
import re
import socket
import traceback as tb
from subprocess import SubprocessError
from typing import TYPE_CHECKING, Any, Iterator, Optional, Type

import psutil
from typing_extensions import override

from crossbench import path as pth
from crossbench.parse import NumberParser
from crossbench.plt.posix import PosixPlatform
from crossbench.plt.signals import MacOSSignals

if TYPE_CHECKING:
  from crossbench.plt.base import CPUFreqInfo
  from crossbench.plt.display_info import DisplayInfo

DISPLAY_NDRV_RE = re.compile(
    "(?P<resX>[0-9]+) x (?P<resY>[0-9]+) @ (?P<freq>[0-9.]+)Hz")


def parse_display_ndrvs(spdisplays_ndrvs: dict) -> Iterator[DisplayInfo]:
  """
  Parses `system_profiler SPDisplaysDataType` output.
  "SPDisplaysDataType" : [
    {
      ...
      "spdisplays_ndrvs" : [
        {
          ...
          "_spdisplays_resolution" : "1728 x 1117 @ 60.00Hz",
          "spdisplays_ambient_brightness" : "spdisplays_no",
          "spdisplays_pixelresolution" : "spdisplays_3456x2234Retina"
          ...
        },
        {
          ...
          "_spdisplays_resolution" : "3360 x 1890 @ 30.00Hz",
          "spdisplays_pixelresolution" : "6720 x 3780",
          "spdisplays_resolution" : "3360 x 1890 @ 30.00Hz",
          ...
        }
      ],
      ...
    }
  ]
  """
  for spdisplay_ndrv in spdisplays_ndrvs:
    # Use virtual pixel resolution of the monitor:
    freq_str = spdisplay_ndrv.get("_spdisplays_resolution", "")
    if match := DISPLAY_NDRV_RE.search(freq_str):
      yield {
          "resolution": (NumberParser.positive_int(match.group("resX")),
                         NumberParser.positive_int(match.group("resY"))),
          "refresh_rate": NumberParser.positive_float(match.group("freq")),
      }


class MacOSPlatform(PosixPlatform):
  SEARCH_PATHS: tuple[pth.AnyPath, ...] = (
      pth.AnyPosixPath("."),
      pth.AnyPosixPath("/Applications"),
      # TODO: support remote platforms
      pth.LocalPath.home() / "Applications",
  )

  LSAPPINFO_IN_FRONT_LINE_RE = r".*\(in front\)\s*"
  LSAPPINFO_PID_LINE_RE = r"\s*pid = ([0-9]+).*"

  @property
  @override
  def is_macos(self) -> bool:
    return True

  @property
  @override
  def name(self) -> str:
    return "macos"

  @property
  def signals(self) -> Type[MacOSSignals]:
    return MacOSSignals

  @functools.cached_property
  @override
  def version(self) -> str:
    return self.sh_stdout("sw_vers", "-productVersion").strip()

  @functools.cached_property
  def version_parts(self) -> tuple[int, ...]:
    return tuple(map(int, self.version.split(".")))

  @functools.cached_property
  @override
  def device(self) -> str:  #pylint: disable=invalid-overridden-method
    return self.sh_stdout("sysctl", "-n", "hw.model").strip()

  @functools.cached_property
  @override
  def cpu(self) -> str:  #pylint: disable=invalid-overridden-method
    brand = self.sh_stdout("sysctl", "-n", "machdep.cpu.brand_string").strip()
    num_cores = self.cpu_cores(logical=True)
    return f"{brand} {num_cores} cores"

  @functools.lru_cache(maxsize=2)
  @override
  def cpu_cores(self, logical: bool) -> int:
    if self.is_local:
      return super().cpu_cores(logical)
    sysctl_name = "hw.logicalcpu_max" if logical else "hw.physicalcpu_max"
    cores = self.sh_stdout("sysctl", "-n", sysctl_name).strip()
    return int(cores)

  @property
  @override
  def is_battery_powered(self) -> bool:
    if self.is_local:
      return super().is_battery_powered
    return "Battery Power" in self.sh_stdout("pmset", "-g", "batt")

  def get_relative_cpu_speed(self) -> float:
    try:
      lines = self.sh_stdout("pmset", "-g", "therm").split()
      for index, line in enumerate(lines):
        if line == "CPU_Speed_Limit":
          return int(lines[index + 2]) / 100.0
    except SubprocessError:
      pass
    logging.debug("Could not get relative CPU speed: %s", tb.format_exc())
    return 1

  @functools.lru_cache(maxsize=1)
  @override
  def system_details(self) -> dict[str, Any]:
    details = super().system_details()
    details.update(self._macos_system_details())
    return details

  def _macos_system_details(self) -> dict[str, Any]:
    return {
        "system_profiler":
            self.sh_stdout("system_profiler", "SPHardwareDataType"),
        "sysctl_machdep_cpu":
            self.sh_stdout("sysctl", "machdep.cpu"),
        "sysctl_hw":
            self.sh_stdout("sysctl", "hw"),
    }

  @functools.lru_cache(maxsize=1)
  def display_details(self) -> tuple[DisplayInfo, ...]:
    display_info_raw = self.sh_stdout("system_profiler", "-json",
                                      "SPDisplaysDataType").strip()
    display_info = json.loads(display_info_raw)
    if spdisplays_data := display_info.get("SPDisplaysDataType"):
      if spdisplays_ndrvs := spdisplays_data[0].get("spdisplays_ndrvs"):
        return tuple(parse_display_ndrvs(spdisplays_ndrvs))
    return tuple()

  def display_resolution(self) -> tuple[int, int]:
    return self.display_details()[0]["resolution"]

  def _cpu_freq(self) -> Optional[CPUFreqInfo]:
    if self.is_remote:
      return super()._cpu_freq()
    # BUG(394337121): older macOs versions on arm segfault with python 3.11
    if self.is_arm64 and self.version_parts < (12, 0):
      return None
    try:
      return super()._cpu_freq()
    except FileNotFoundError as e:
      logging.debug("psutil.cpu_freq() failed (normal on macOS M1): %s", e)
      return None

  def _find_app_binary_path(self, app_path: pth.AnyPath) -> pth.AnyPath:
    assert app_path.suffix == ".app", f"Expected .app but got {app_path}"
    bin_path = app_path / "Contents" / "MacOS" / app_path.stem
    if self.exists(bin_path):
      return bin_path
    if not self.exists(bin_path.parent):
      raise ValueError(f"Binary does not exist: {bin_path}")
    self.assert_is_local()
    binaries = [
        path for path in self.iterdir(bin_path.parent) if self.is_file(path)
    ]
    if len(binaries) == 1:
      return binaries[0]
    # Fallback to read plist
    plist_path = app_path / "Contents" / "Info.plist"
    if not self.is_file(plist_path):
      raise ValueError(f"Could not find Info.plist in app bundle: {app_path}")
    # TODO: support remote platform
    with self.local_path(plist_path).open("rb") as f:
      plist = plistlib.load(f)
    bin_path = (
        app_path / "Contents" / "MacOS" /
        plist.get("CFBundleExecutable", app_path.stem))
    if self.is_file(bin_path):
      return bin_path
    if not binaries:
      raise ValueError(f"No binaries found in {app_path}")
    raise ValueError(f"Invalid number of binaries found: {binaries}")

  def search_binary(self, app_or_bin: pth.AnyPathLike) -> Optional[pth.AnyPath]:
    app_or_bin_path: pth.AnyPath = self.path(app_or_bin)
    if not app_or_bin_path.parts:
      raise ValueError("Got empty path")
    is_app = app_or_bin_path.suffix == ".app"
    if not is_app:
      # Look up basic binaries with `which` if possible.
      if result_path := self.which(app_or_bin_path):
        assert self.exists(result_path), f"{result_path} does not exist."
        return result_path
    if app_path := self.lookup_binary_override(app_or_bin_path):
      if app_path := self._validate_search_binary_candidate(is_app, app_path):
        return app_path
    for search_path in self.SEARCH_PATHS:
      # Recreate Path object for easier pyfakefs testing
      result_path = self.path(search_path) / app_or_bin_path
      if app_path := self._validate_search_binary_candidate(
          is_app, result_path):
        return app_path
    return None

  def _validate_search_binary_candidate(
      self, is_app: bool, result_path: pth.AnyPath) -> Optional[pth.AnyPath]:
    if not is_app:
      if self.is_file(result_path):
        return result_path
      return None
    if not self.is_dir(result_path):
      return None
    result_path = self._find_app_binary_path(result_path)
    if self.exists(result_path):
      return result_path
    return None

  def search_app(self, app_or_bin: pth.AnyPathLike) -> Optional[pth.AnyPath]:
    app_or_bin_path: pth.AnyPath = self.path(app_or_bin)
    if not app_or_bin_path.parts:
      raise ValueError("Got empty path")
    self.assert_is_local()
    if app_or_bin_path.suffix != ".app":
      raise ValueError("Expected app name with '.app' suffix, "
                       f"but got: '{app_or_bin_path.name}'")
    binary = self.search_binary(app_or_bin_path)
    if not binary:
      return None
    # input: /Applications/Safari.app/Contents/MacOS/Safari
    # output: /Applications/Safari.app
    app_path = binary.parents[2]
    assert app_path.suffix == ".app", f"Expected .app but got {app_path}"
    assert self.is_dir(app_path)
    return app_path

  @override
  def app_version(self, app_or_bin: pth.AnyPathLike) -> str:
    app_or_bin = self.path(app_or_bin)
    if not self.exists(app_or_bin):
      raise ValueError(f"Binary {app_or_bin} does not exist.")

    app_path = None
    for current in (app_or_bin, *app_or_bin.parents):
      if current.suffix == ".app" and current.stem == app_or_bin.stem:
        app_path = current
        break
    if not app_path:
      # Most likely just a cli tool"
      return self.sh_stdout(app_or_bin, "--version").strip()
    info_plist = app_path / "Contents/Info.plist"
    if self.exists(info_plist):
      plist = plistlib.loads(self.cat_bytes(info_plist))
      if version_string := plist.get("CFBundleShortVersionString"):
        display_name = plist.get("CFBundleDisplayName")
        if not display_name:
          # Fallback. Apps like Firefox have no CFBundleDisplayName.
          display_name = plist.get("CFBundleName")
        return f"{display_name} {version_string}"


    # Backup solution use the binary (not the .app bundle) with --version.
    maybe_bin_path: pth.AnyPath | None = app_or_bin
    if app_or_bin.suffix == ".app":
      maybe_bin_path = self.search_binary(app_or_bin)
    if not maybe_bin_path:
      raise ValueError(f"Could not extract app version: {app_or_bin}")
    try:
      return self.sh_stdout(maybe_bin_path, "--version").strip()
    except SubprocessError as e:
      raise ValueError(f"Could not extract app version: {app_or_bin}") from e

  def exec_apple_script(self, script: str, *args: str) -> str:
    if args:
      script = f"""on run argv
        {script.strip()}
      end run"""
    return self.sh_stdout("/usr/bin/osascript", "-e", script, *args)

  def foreground_process(self) -> Optional[dict[str, Any]]:
    foreground_process_info = self.sh_stdout("lsappinfo", "front").strip()
    if not foreground_process_info:
      return None
    foreground_info = self.sh_stdout("lsappinfo", "info", "-only", "pid",
                                     foreground_process_info).strip()
    foreground_info_split = foreground_info.split("=")

    pid = None

    if len(foreground_info_split) == 2:
      pid = foreground_info_split[1]
    else:
      # On macOS 14.0 Beta, "lsappinfo info" returns an empty result. Fall back
      # to parsing the output of "lsappinfo list" to obtain the front app's
      # info.
      app_list = self.sh_stdout("lsappinfo", "list")
      found_front_app = False
      for app_list_line in app_list.splitlines():
        if re.match(self.LSAPPINFO_IN_FRONT_LINE_RE, app_list_line):
          found_front_app = True
        elif found_front_app:
          match = re.match(self.LSAPPINFO_PID_LINE_RE, app_list_line)
          if match:
            pid = match.group(1)
            break

    if pid and pid.isdigit():
      return psutil.Process(int(pid)).as_dict()

    return None

  def check_system_monitoring(self, disable: bool = False) -> bool:
    return self.check_crowdstrike(disable)

  def check_autobrightness(self) -> bool:
    output = self.sh_stdout("system_profiler", "SPDisplaysDataType",
                            "-json").strip()
    data = json.loads(output)
    if spdisplays_data := data.get("SPDisplaysDataType"):
      for data in spdisplays_data:
        if spdisplays_ndrvs := data.get("spdisplays_ndrvs"):
          for display in spdisplays_ndrvs:
            if auto_brightness := display.get("spdisplays_ambient_brightness"):
              return auto_brightness == "spdisplays_yes"
        raise ValueError(
            "Could not find 'spdisplays_ndrvs' from SPDisplaysDataType. "
            f"Output={output}")
    raise ValueError("Could not get 'SPDisplaysDataType' form system profiler. "
                     f"Output={output}")

  def check_crowdstrike(self, disable: bool = False) -> bool:
    falconctl = self.path(
        "/Applications/Falcon.app/Contents/Resources/falconctl")
    if not self.exists(falconctl):
      logging.debug("You're fine, falconctl or %s are not installed.",
                    falconctl)
      return True
    if not disable:
      for process in self.processes(attrs=["exe"]):
        exe = process["exe"]
        if exe and exe.endswith("/com.crowdstrike.falcon.Agent"):
          return False
      return True
    try:
      logging.warning("Checking falcon sensor status:")
      status = self.sh_stdout("sudo", falconctl, "stats", "agent_info")
    except SubprocessError as e:
      logging.debug("Could not probe falconctl, assuming it's not running: %s",
                    e)
      return True
    if "operational: true" not in status:
      # Early return if not running, no need to disable the sensor.
      return True
    # Try disabling the process
    logging.warning("Disabling crowdstrike monitoring:")
    self.sh("sudo", falconctl, "unload")
    return True

  def _get_main_display(self) -> tuple[ctypes.CDLL, Any]:
    assert self.is_local, "Operation not supported on remote platforms"
    core_graphics = ctypes.CDLL(
        "/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics")
    main_display = core_graphics.CGMainDisplayID()
    return main_display, core_graphics

  def _get_display_service(self) -> tuple[ctypes.CDLL, Any]:
    main_display, _ = self._get_main_display()
    display_services = ctypes.CDLL(
        "/System/Library/PrivateFrameworks/DisplayServices.framework"
        "/DisplayServices")
    display_services.DisplayServicesSetBrightness.argtypes = [
        ctypes.c_int, ctypes.c_float
    ]
    display_services.DisplayServicesGetBrightness.argtypes = [
        ctypes.c_int, ctypes.POINTER(ctypes.c_float)
    ]
    return display_services, main_display

  def set_main_display_brightness(self, brightness_level: int) -> None:
    """Sets the main display brightness at the specified percentage by
    brightness_level.

    This function imitates the open-source "brightness" tool at
    https://github.com/nriley/brightness.
    Since the benchmark doesn't care about older MacOSen, multiple displays
    or other complications that tool has to consider, setting the brightness
    level boils down to calling this function for the main display.

    Args:
      brightness_level: Percentage at which we want to set screen brightness.

    Raises:
      AssertionError: An error occurred when we tried to set the brightness
    """
    display_services, main_display = self._get_display_service()
    ret = display_services.DisplayServicesSetBrightness(main_display,
                                                        brightness_level / 100)
    assert ret == 0

  def get_main_display_brightness(self) -> int:
    """Gets the current brightness level of the main display .

    This function imitates the open-source "brightness" tool at
    https://github.com/nriley/brightness.
    Since the benchmark doesn't care about older MacOSen, multiple displays
    or other complications that tool has to consider, setting the brightness
    level boils down to calling this function for the main display.

    Returns:
      An int of the current percentage value of the main screen brightness

    Raises:
      AssertionError: An error occurred when we tried to set the brightness
    """

    display_services, main_display = self._get_display_service()
    display_brightness = ctypes.c_float()  # pylint: disable=no-value-for-parameter
    ret = display_services.DisplayServicesGetBrightness(
        main_display, ctypes.byref(display_brightness))
    assert ret == 0, f"ret={ret}, display_brightness={display_brightness}"
    return round(display_brightness.value * 100)

  def _core_graphics_types(self, core_graphics) -> None:
    # https://developer.apple.com/documentation/coregraphics/1455620-cgmaindisplayid?language=objc
    core_graphics.CGMainDisplayID.argtypes = ()
    core_graphics.CGMainDisplayID.restype = ctypes.c_uint32
    # https://developer.apple.com/documentation/coregraphics/1454099-cgdisplaycopydisplaymode?language=objc
    core_graphics.CGDisplayCopyDisplayMode.argtypes = (ctypes.c_uint32,)
    core_graphics.CGDisplayCopyDisplayMode.restype = ctypes.c_void_p
    # https://developer.apple.com/documentation/coregraphics/1455537-cgdisplaycopyalldisplaymodes?language=objc
    core_graphics.CGDisplayCopyAllDisplayModes.argtypes = (ctypes.c_uint32,
                                                           ctypes.c_void_p)
    core_graphics.CGDisplayCopyAllDisplayModes.restype = ctypes.c_void_p
    # https://developer.apple.com/documentation/coregraphics/1454442-cgdisplaymodegetwidth?language=objc
    core_graphics.CGDisplayModeGetWidth.argtypes = (ctypes.c_void_p,)
    core_graphics.CGDisplayModeGetWidth.restype = ctypes.c_size_t
    # https://developer.apple.com/documentation/coregraphics/1455380-cgdisplaymodegetheight?language=objc
    core_graphics.CGDisplayModeGetHeight.argtypes = (ctypes.c_void_p,)
    core_graphics.CGDisplayModeGetHeight.restype = ctypes.c_size_t
    # https://developer.apple.com/documentation/coregraphics/1454661-cgdisplaymodegetrefreshrate?language=objc
    core_graphics.CGDisplayModeGetRefreshRate.argtypes = (ctypes.c_void_p,)
    core_graphics.CGDisplayModeGetRefreshRate.restype = ctypes.c_double
    # https://developer.apple.com/documentation/coregraphics/1454760-cgdisplaysetdisplaymode?language=objc
    core_graphics.CGDisplaySetDisplayMode.argtypes = (ctypes.c_uint32,
                                                      ctypes.c_void_p)
    core_graphics.CGDisplaySetDisplayMode.restype = ctypes.c_int32

  def _core_foundation_types(self, core_foundation) -> None:
    # https://developer.apple.com/documentation/corefoundation/1388772-cfarraygetcount?language=objc
    core_foundation.CFArrayGetCount.argtypes = (ctypes.c_void_p,)
    core_foundation.CFArrayGetCount.restype = ctypes.c_long
    # https://developer.apple.com/documentation/corefoundation/1388767-cfarraygetvalueatindex?language=objc
    core_foundation.CFArrayGetValueAtIndex.argtypes = (ctypes.c_void_p,
                                                       ctypes.c_long)
    core_foundation.CFArrayGetValueAtIndex.restype = ctypes.c_void_p
    # https://developer.apple.com/documentation/corefoundation/cfdictionarycreate(_:_:_:_:_:_:)?language=objc
    core_foundation.CFDictionaryCreate.argtypes = (
        ctypes.c_void_p,  # allocator
        ctypes.c_void_p,  # **keys
        ctypes.c_void_p,  # **values
        ctypes.c_long,  # numValues
        ctypes.c_void_p,  # *keyCallBacks
        ctypes.c_void_p,  # *valueCallBacks
    )
    core_foundation.CFDictionaryCreate.restype = ctypes.c_void_p
    # https://developer.apple.com/documentation/corefoundation/1521153-cfrelease/
    core_foundation.CFRelease.argtypes = (ctypes.c_void_p,)

  def set_display_refresh_rate(self,
                               refresh_rate: int,
                               retry: int = 3) -> tuple[bool, str]:
    """Sets the refresh rate if the main display supports it.

    This function uses CoreGraphics and CoreFoundtation libraries:
    https://developer.apple.com/documentation/coregraphics/
    https://developer.apple.com/documentation/corefoundation
    If this function detects a display mode with the same width and height, it
    sets the refresh rate to the requested rate.

    Args:
      refresh_rate: Target display refresh rate to set.
      retry: How many times to try if something goes wrong in setting the rate.

    Returns:
      A tuple of boolean values to indicate success or failure, and a message.
    """
    refresh_rate = NumberParser.int_range(30, 240)(refresh_rate)
    # Getting the current main display info.
    main_display, core_graphics = self._get_main_display()
    self._core_graphics_types(core_graphics)

    # Get the current refresh rate and verify if it needs to be set.
    display_mode = core_graphics.CGDisplayCopyDisplayMode(main_display)
    main_refresh_rate = core_graphics.CGDisplayModeGetRefreshRate(display_mode)
    if main_refresh_rate == refresh_rate:
      return True, f"The display refresh rate is already {refresh_rate}Hz"
    main_width = round(core_graphics.CGDisplayModeGetWidth(display_mode))
    main_height = round(core_graphics.CGDisplayModeGetHeight(display_mode))
    log_msg = (f"\nMain display: ID={main_display}, "
               f"width={main_width}, height={main_height}, "
               f"main_refresh_rate={refresh_rate}, "
               f"refresh_rate={refresh_rate}")

    core_foundation = ctypes.CDLL(ctypes.util.find_library("CoreFoundation"))
    self._core_foundation_types(core_foundation)

    # Finding all available display modes.
    keys = (ctypes.c_void_p * 1)()
    keys[0] = ctypes.c_void_p.in_dll(
        core_graphics, "kCGDisplayShowDuplicateLowResolutionModes")
    values = (ctypes.c_void_p * 1)()
    values[0] = ctypes.c_void_p.in_dll(core_foundation, "kCFBooleanTrue")
    options = core_foundation.CFDictionaryCreate(None, keys, values, 1, None,
                                                 None)
    display_modes = core_graphics.CGDisplayCopyAllDisplayModes(
        main_display, options)

    # Finding a display with original size and capable of the refresh rate.
    set_mode = None
    for idx in range(core_foundation.CFArrayGetCount(display_modes)):
      display_mode = core_foundation.CFArrayGetValueAtIndex(display_modes, idx)
      width = round(core_graphics.CGDisplayModeGetWidth(display_mode))
      height = round(core_graphics.CGDisplayModeGetHeight(display_mode))
      rate = int(core_graphics.CGDisplayModeGetRefreshRate(display_mode))
      log_msg += (f"\nDetected: display_mode={display_mode}, "
                  f"width={width}, height={height}, "
                  f"refresh_rate={rate}")
      if (main_width == width and main_height == height and
          refresh_rate == rate):
        set_mode = display_mode
        break

    # Set the refresh rate if the suitable display mode is found.
    if set_mode is not None:
      for _ in range(retry):
        core_graphics.CGDisplaySetDisplayMode(main_display, set_mode, None)
        rate = int(core_graphics.CGDisplayModeGetRefreshRate(display_mode))
        if refresh_rate == rate:
          return True, f"The refresh rate was successfully set!\n{log_msg}"
        log_msg += "\nFailed to set the refresh rate!"
        self.sleep(2)
    else:
      log_msg += "\nFailed to find a match for display size and refresh rate!"

    return False, log_msg

  def screenshot(self, result_path: pth.AnyPath) -> None:
    self.sh("screencapture", "-x", result_path)

  @override
  def is_port_used(self, port: int) -> bool:
    # We need a custom solution for macos:
    # - psutil.net_connections requires root access on macos
    # - 'ss' is not available by default on macos
    # This is a semi-ideal solution as it creates a temporary local server.
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
      return s.connect_ex(("localhost", port)) == 0

  @override
  def last_modified(self, path: pth.AnyPathLike) -> float:
    if self.is_local:
      return super().last_modified(path)
    # Get seconds since epoch
    return float(self.sh_stdout("stat", "-f", "%m", self.path(path)))
