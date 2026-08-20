# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import abc
import enum
from typing import TYPE_CHECKING, Optional, Self, cast

from selenium.webdriver.safari.options import Options as SafariOptions
from typing_extensions import override

import crossbench.probes.perfetto.traceconv as cb_traceconv
from crossbench.browsers.chromium.webdriver import ChromiumBasedWebDriver
from crossbench.helper.path_finder import TraceconvFinder
from crossbench.probes.probe import (Probe, ProbeConfigParser, ProbeContext,
                                     ProbeKeyT)
from crossbench.probes.probe_error import (ProbeIncompatibleBrowser,
                                           ProbeValidationError)
from crossbench.probes.result_location import ResultLocation
from crossbench.str_enum_with_help import StrEnumWithHelp

if TYPE_CHECKING:
  from selenium.webdriver.common.options import BaseOptions

  import crossbench.path as pth
  from crossbench.browsers.browser import Browser
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.probes.results import ProbeResult
  from crossbench.runner.run import Run


@enum.unique
class MozProfilerStartupFeatures(StrEnumWithHelp):
  """Options for MOZ_PROFILER_STARTUP_FEATURES env var.
    Extracted via MOZ_PROFILER_HELP=1 ./firefox-nightly-en/firefox
    """
  JAVA = ("java", "Profile Java code, Android only")
  JS = ("js", "Get the JS engine to expose the JS stack to the profiler")
  LEAF = ("leaf", "Include the C++ leaf node if not stackwalking")
  MAINTHREADIO = ("mainthreadio", "Add main thread file I/O")
  FILEIO = ("fileio",
            "Add file I/O from all profiled threads, implies mainthreadio")
  FILEIOALL = ("fileioall", "Add file I/O from all threads, implies fileio")
  NOIOSTACKS = ("noiostacks",
                "File I/O markers do not capture stacks, to reduce overhead")
  SCREENSHOTS = ("screenshots",
                 "Take a snapshot of the window on every composition")
  SEQSTYLE = ("seqstyle", "Disable parallel traversal in styling")
  STACKWALK = ("stackwalk",
               "Walk the C++ stack, not available on all platforms")
  TASKTRACER = ("tasktracer", "Start profiling with feature TaskTracer")
  THREADS = ("threads", "Profile the registered secondary threads")
  JSTRACER = ("jstracer", "Enable tracing of the JavaScript engine")
  JSALLOCATIONS = ("jsallocations",
                   "Have the JavaScript engine track allocations")
  NOSTACKSAMPLING = (
      "nostacksampling",
      "Disable all stack sampling: Cancels 'js', 'leaf', 'stackwalk' and labels"
  )
  PREFERENCEREADS = ("preferencereads", "Track when preferences are read")
  NATIVEALLOCATIONS = (
      "nativeallocations",
      "Collect the stacks from a smaller subset of all native allocations, "
      "biasing towards collecting larger allocations")
  IPCMESSAGES = ("ipcmessages",
                 "Have the IPC layer track cross-process messages")
  AUDIOCALLBACKTRACING = ("audiocallbacktracing", "Audio callback tracing")
  CPU = ("cpu", "CPU utilization")


@enum.unique
class FirefoxProfilerEnvVars(enum.StrEnum):
  # If set to any value other than '' or '0'/'N'/'n', starts the
  # profiler immediately on start-up.
  STARTUP = "MOZ_PROFILER_STARTUP"
  # Contains a comma-separated list of MozProfilerStartupFeatures.
  STARTUP_FEATURES = "MOZ_PROFILER_STARTUP_FEATURES"
  # If set, the profiler saves a profile to the named file on shutdown.
  SHUTDOWN = "MOZ_PROFILER_SHUTDOWN"


class BrowserProfilingProbe(Probe):
  """
  Browser profiling for generating in-browser performance profiles:
  - Firefox https://profiler.firefox.com/
  - Chrome: https://developer.chrome.com/docs/devtools/
  - Safari: Timelines https://developer.apple.com/safari/tools
  """
  NAME = "browser-profiling"
  RESULT_LOCATION = ResultLocation.BROWSER
  IS_GENERAL_PURPOSE = True

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "moz_profiler_startup_features",
        type=MozProfilerStartupFeatures,
        is_list=True,
        default=[])
    cb_traceconv.add_argument(parser)
    return parser

  def __init__(self,
               moz_profiler_startup_features: Optional[
                   list[MozProfilerStartupFeatures]] = None,
               traceconv: Optional[pth.LocalPath] = None) -> None:
    super().__init__()
    self._moz_profiler_startup_features: list[
        MozProfilerStartupFeatures] = moz_profiler_startup_features or []
    self._traceconv: pth.LocalPath | None = traceconv
    if not traceconv:
      self._traceconv = TraceconvFinder(self.host_platform).local_path

  @property
  @override
  def key(self) -> ProbeKeyT:
    return super().key + (
        ("moz_profiler_startup_features",
         tuple(map(str, self.moz_profiler_startup_features))),
        ("traceconv", str(self._traceconv)),
    )

  @property
  def moz_profiler_startup_features(self) -> list[MozProfilerStartupFeatures]:
    return self._moz_profiler_startup_features

  @property
  def traceconv(self) -> pth.LocalPath | None:
    return self._traceconv

  @override
  def validate_browser(self, env: RunnerEnv, browser: Browser) -> None:
    super().validate_browser(env, browser)
    if browser.platform.is_remote:
      raise ProbeValidationError(
          self, f"Only works on local browser, but got {browser}.")
    attributes = browser.attributes()
    if attributes.is_chromium_based or attributes.is_safari:
      return
    if attributes.is_firefox:
      self._validate_firefox(env, browser)
    raise ProbeIncompatibleBrowser(self, browser)

  def _validate_firefox(self, env: RunnerEnv, browser: Browser) -> None:
    browser_env = browser.platform.environ
    for env_var in list(FirefoxProfilerEnvVars):
      env_var_str = str(env_var)
      if env_var_str in browser_env:
        env.handle_warning(f"Probe({self}) conflicts with existing "
                           f"env[{env_var_str}]={browser_env[env_var_str]}")

  def get_context(self, run: Run) -> BrowserProfilingProbeContext:
    attributes = run.browser.attributes()
    if attributes.is_chromium_based:
      return ChromiumWebDriverBrowserProfilingProbeContext(self, run)
    if attributes.is_firefox:
      return FirefoxBrowserProfilingProbeContext(self, run)
    if attributes.is_safari:
      return SafariWebdriverBrowserProfilingProbeContext(self, run)
    raise NotImplementedError(
        f"Probe({self}): Unsupported browser: {run.browser}")


class BrowserProfilingProbeContext(
    ProbeContext[BrowserProfilingProbe], metaclass=abc.ABCMeta):

  @override
  def setup(self) -> None:
    pass

  def start(self) -> None:
    pass

  def stop(self) -> None:
    pass


class ChromiumWebDriverBrowserProfilingProbeContext(BrowserProfilingProbeContext
                                                   ):

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    return (super().get_default_result_path().parent /
            f"{self.browser.type_name()}.profile.pb.gz")

  @property
  def chromium(self) -> ChromiumBasedWebDriver:
    return cast(ChromiumBasedWebDriver, self.browser)

  def start(self) -> None:
    self.chromium.start_profiling()

  def stop(self) -> None:
    with self.run.actions(f"Probe({self.probe}): extract DevTools profile."):
      profile_bytes = self.chromium.stop_profiling()
      self.local_result_path.write_bytes(profile_bytes)

  def teardown(self) -> ProbeResult:
    trace_file = self.local_result_path
    if legacy_json_file := cb_traceconv.convert_to_json(self.host_platform,
                                                        self.probe.traceconv,
                                                        trace_file):
      return self.local_result(trace=(trace_file,), json=(legacy_json_file,))
    return self.local_result(trace=(trace_file,))


class FirefoxBrowserProfilingProbeContext(BrowserProfilingProbeContext):

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    return super().get_default_result_path().parent / "firefox.profile.json"

  @override
  def setup(self) -> None:
    env = self.browser.platform.environ
    env[FirefoxProfilerEnvVars.STARTUP] = "y"
    if self.probe.moz_profiler_startup_features:
      env[FirefoxProfilerEnvVars.STARTUP_FEATURES] = ",".join(
          str(feature) for feature in self.probe.moz_profiler_startup_features)
    env[FirefoxProfilerEnvVars.SHUTDOWN] = str(self.result_path)

  @override
  def teardown(self) -> ProbeResult:
    env = self.browser.platform.environ
    del env[FirefoxProfilerEnvVars.STARTUP]
    del env[FirefoxProfilerEnvVars.STARTUP_FEATURES]
    del env[FirefoxProfilerEnvVars.SHUTDOWN]
    return self.browser_result(json=[self.result_path])


class SafariWebdriverBrowserProfilingProbeContext(BrowserProfilingProbeContext):

  @override
  def get_default_result_path(self) -> pth.AnyPath:
    return super().get_default_result_path().parent / "safari.timeline.json"

  @override
  def setup_selenium_options(self, options: BaseOptions) -> None:
    assert isinstance(options, SafariOptions)
    cast(SafariOptions, options).automatic_profiling = True

  @override
  def stop(self) -> None:
    # TODO: Update this mess when Safari supports a command-line option
    # to download the profile.
    # Manually safe the profile using apple script to navigate the safari UI
    # Stop profiling.
    self.browser_platform.exec_apple_script("""
      tell application "System Events"
        keystroke "T" using {command down, option down, shift down}
      end tell""")
    # TODO: explicitly focus the developer pane
    # Focus the Developer Tools split pane and use CMD-S to save the profile.
    self.browser_platform.exec_apple_script(f"""
      tell application "System Events"
        keystroke "S" using command down
        tell window "Save As"
          delay 0.5
          keystroke "g" using {{command down, shift down}}
          delay 0.5
          # Send DELETE key input to clear the current text input.
          key code 51
          keystroke "{self.result_path}"
          delay 0.5
          keystroke return
          delay 0.5
          keystroke return
        end tell
      end tell""")

  @override
  def teardown(self) -> ProbeResult:
    return self.browser_result(json=[self.result_path])
