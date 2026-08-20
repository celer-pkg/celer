# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import datetime as dt
import itertools
import logging
import sys
from typing import TYPE_CHECKING, Any, Optional, Sequence, Type

from typing_extensions import override

from crossbench import exception
from crossbench import path as pth
from crossbench import plt
from crossbench.benchmarks.base import Benchmark
from crossbench.browsers.splash_screen import SplashScreen
from crossbench.browsers.viewport import Viewport, ViewportMode
from crossbench.cli.config.browser import BrowserConfig
from crossbench.cli.config.browser_variants import BrowserVariantsConfig
from crossbench.cli.config.env import (ENV_CONFIG_PRESETS, EnvConfig,
                                       ValidationMode)
from crossbench.cli.config.network import NetworkConfig
from crossbench.cli.config.probe import PROBE_LOOKUP, ProbeConfig
from crossbench.cli.config.probe_list import ProbeListConfig
from crossbench.cli.config.secrets import Secrets
from crossbench.cli.subcommand.base import CrossbenchSubcommand
from crossbench.helper.wake_lock import WakeLock
from crossbench.parse import (DurationParser, LateArgumentError, ObjectParser,
                              PathParser)
from crossbench.probes.debugger import DebuggerProbe
from crossbench.probes.internal.errors import ErrorsProbe
from crossbench.probes.thermal_monitor import ThermalStatus
from crossbench.runner.run_annotation import RunAnnotation
from crossbench.runner.runner import Runner
from crossbench.runner.timing import Timing

if TYPE_CHECKING:
  from crossbench.browsers.browser import Browser
  from crossbench.cli.cli import CrossBenchCLI
  from crossbench.probes.probe import Probe


class EnableFastAction(argparse.Action):
  """Custom action to enable fast test runs"""

  def __call__(self,
               parser: argparse.ArgumentParser,
               namespace: argparse.Namespace,
               values: str | Sequence[Any] | None,
               option_string: Optional[str] = None) -> None:
    setattr(namespace, "cool_down_time", dt.timedelta())
    setattr(namespace, "splash_screen", SplashScreen.NONE)
    setattr(namespace, "env_validation", ValidationMode.SKIP)


class AppendDebuggerProbeAction(argparse.Action):
  """Custom action to set multiple args when --gdb or --lldb are set:
  - Add a DebuggerProbe config.
  - Increase --timeout-unit to a large value to keep debug session alive for a
    longer time.
  """

  def __call__(self,
               parser: argparse.ArgumentParser,
               namespace: argparse.Namespace,
               values: str | Sequence[Any] | None,
               option_string: Optional[str] = None) -> None:
    probes: list[ProbeConfig] = getattr(namespace, self.dest, [])
    probe_settings = {"debugger": "gdb"}
    if option_string and "lldb" in option_string:
      probe_settings["debugger"] = "lldb"
    probes.append(ProbeConfig(DebuggerProbe, probe_settings))
    if not getattr(namespace, "timeout_unit", None):
      # Set a very large --timeout-unit to allow for very slow debugging without
      # causing timeouts (for instance when waiting on a breakpoint).
      setattr(namespace, "timeout_unit", dt.timedelta.max)


class BenchmarkSubcommand(CrossbenchSubcommand):

  def __init__(self, cli: CrossBenchCLI,
               benchmark_cls: Type[Benchmark]) -> None:
    self._benchmark_cls = benchmark_cls
    self._runner_cls: Type[Runner] = Runner
    self._runner: Runner | None = None
    super().__init__(cli)

  @property
  def runner(self) -> Runner:
    assert self._runner, "No runner"
    return self._runner

  @override
  def add_cli_parser(self) -> argparse.ArgumentParser:
    parser = self._benchmark_cls.add_cli_parser(self.cli.subparsers)
    assert isinstance(parser, argparse.ArgumentParser), (
        f"Benchmark class {self._benchmark_cls}.add_cli_parser did not return "
        f"an ArgumentParser: {parser}")
    self._runner_cls.add_cli_parser(self._benchmark_cls, parser)
    self._add_timing_arguments(parser)
    self._add_network_arguments(parser)
    self._add_env_arguments(parser)
    self._add_browser_arguments(parser)
    self._add_browser_cache_arguments(parser)
    self._add_chrome_arguments(parser)
    self._add_probe_arguments(parser)
    self._add_debugging_arguments(parser)
    self.cli.add_base_arguments(parser)
    parser.add_argument("other_browser_args", nargs="*")
    return parser

  def _add_browser_cache_arguments(self,
                                   parser: argparse.ArgumentParser) -> None:
    browser_cache_group = parser.add_argument_group(
        "Browser Options: Caches",
        "By default tmp caches are auto-created and cleared at startup.")
    cache_options = browser_cache_group.add_mutually_exclusive_group()
    cache_options.add_argument(
        "--keep-browser-cache",
        "--no-clear-browser-cache",
        dest="clear_browser_cache_dir",
        action="store_false",
        default=None,
        help=("Do not clear the browser cache dir after every run. "
              "This will affect performance and leak user data across runs."))
    cache_options.add_argument(
        "--clear-browser-cache",
        "--clear-browser-cache-dir",
        dest="clear_browser_cache_dir",
        action="store_true",
        help=("Force clear browser cache dir (default). "
              "Use this flag to override browser config values"))

  def _add_probe_arguments(self, parser: argparse.ArgumentParser) -> None:
    probe_group = parser.add_argument_group("Probe Options")
    probe_group.add_argument(
        "--probe",
        action="append",
        type=ProbeConfig.parse,
        default=[],
        help=(
            "Enable general purpose probes to measure data on all cb.stories. "
            "This argument can be specified multiple times to add more probes. "
            "Use inline hjson (e.g. --probe=\"$NAME{$CONFIG}\") "
            "to configure probes. "
            "Individual probe configs can be specified in files as well: "
            "--probe='path/to/config.hjson'. "
            "Use 'describe probes' or 'describe probe $NAME' for probe "
            "configuration details."
            f"\n\nChoices: {', '.join(PROBE_LOOKUP.keys())}"))
    probe_group.add_argument(
        "--probe-config",
        type=PathParser.hjson_file_path,
        default=self._benchmark_cls.default_probe_config_path(),
        help=("Browser configuration.json file. "
              "Use this config file to specify more complex Probe settings. "
              "See config/doc/probe.config.hjson on how to set up a complex "
              "configuration file."))

  def _add_chrome_arguments(self, parser: argparse.ArgumentParser) -> None:
    chrome_args = parser.add_argument_group(
        "Browsers Options: Chrome/Chromium",
        "For convenience these arguments are directly forwarded "
        "directly to chrome. ")
    chrome_args.add_argument(
        "--js-flags", dest="js_flags", action="append", default=[])

    chrome_args.add_argument(
        "--no-sandbox",
        "--nosandbox",
        dest="sandbox",
        action="store_false",
        default=None,
        help=("Disables the sandbox for all process types that are "
              "normally sandboxed. Use for testing purposes only."))

    doc_str = "See chrome's base/feature_list.h source file for more details"
    chrome_args.add_argument(
        "--enable-features",
        help="Comma-separated list of enabled chrome features. " + doc_str,
        default="")
    chrome_args.add_argument(
        "--disable-features",
        help="Command-separated list of disabled chrome features. " + doc_str,
        default="")

    field_trial_group = chrome_args.add_mutually_exclusive_group()
    field_trial_group.add_argument(
        "--enable-field-trial-config",
        "--enable-field-trials",
        default=None,
        action="store_true",
        help=("Use chrome's field-trial configs, "
              "disabled by default by crossbench"))
    field_trial_group.add_argument(
        "--disable-field-trial-config",
        "--disable-field-trials",
        dest="enable_field_trial_config",
        action="store_false",
        help=("Explicitly disable field-trial configs. "
              "Off by default on official builds, "
              "and disabled by default by crossbench."))

  def _add_browser_arguments(self, parser: argparse.ArgumentParser) -> None:
    browser_group = parser.add_argument_group(
        "Browser Options", "Any other browser option can be passed "
        "after the '--' arguments separator.")
    browser_config_group = browser_group.add_mutually_exclusive_group()
    browser_config_group.add_argument(
        "--browser",
        "-b",
        type=BrowserConfig.parse_with_range,
        action="extend",
        default=[],
        help=(
            "Browser binary, defaults to 'chrome-stable'."
            "Use this to test a simple browser variant. "
            "Use [chrome, chrome-stable, chrome-dev, chrome-canary, "
            "safari, safari-tp, "
            "firefox, firefox-stable, firefox-dev, firefox-nightly, "
            "edge, edge-stable, edge-beta, edge-dev, edge-canary] "
            "for system default browsers or a full path. \n"
            "* Use --browser=chrome-M107 to download the latest version for a "
            "specific milestone\n"
            "* Use ... to test milestone ranges --browser=chr-M100...M125"
            "* Use --browser=chrome-100.0.4896.168 to download a specific "
            "chrome version (macOS and linux for googlers and chrome only). \n"
            "* Use --browser=path/to/archive.dmg on macOS or "
            "--browser=path/to/archive.rpm on linux "
            "for locally cached versions (chrome only).\n"
            "* Use --browser=\"${ADB_SERIAL}:chrome\" "
            "(e.g. --browser='0a388e93:chrome') for specific "
            "android devices or --browser='adb:chrome' if only once device is "
            "attached.\n"
            "Repeat for adding multiple browsers. "
            "The browser result dir's name is "
            "'${BROWSER}_${PLATFORM}_${INDEX}' "
            "$INDEX corresponds to the order on the command line."
            "Cannot be used together with --browser-config"))
    browser_config_group.add_argument(
        "--browser-config",
        type=PathParser.hjson_file_path,
        help=("Browser configuration.json file. "
              "Use this to run multiple browsers and/or multiple "
              "flag configurations. "
              "See config/doc/browser.config.hjson on how to set up a complex "
              "configuration file. "
              "Cannot be used together with --browser."))
    browser_group.add_argument(
        "--driver-path",
        type=PathParser.file_path,
        help=("Use the same custom driver path for all specified browsers. "
              "Version mismatches might cause crashes."))
    browser_group.add_argument(
        "--remote-driver-path",
        type=PathParser.any_path,
        help=("Use the same custom driver path for all specified remote"
              " browsers. Version mismatches might cause crashes."))
    browser_group.add_argument(
        "--config",
        type=PathParser.hjson_file_path,
        help=("Specify a common config for --probe-config, --browser-config, "
              "--network-config and --env-config."))
    browser_group.add_argument(
        "--secrets",
        dest="secrets",
        type=Secrets.parse,
        default=Secrets(),
        help="Path to file containing login secrets")

    browser_group.add_argument(
        "--wipe-system-user-data",
        default=False,
        action="store_true",
        help="Clear user data at the beginning of the test "
        "(Android-only, be careful using it).")
    browser_group.add_argument(
        "--browser-cache-dir",
        "--browser-cache",
        "--user-data-dir",
        type=pth.AnyPath,
        help="Set an explicit browser cache dir")

    browser_group.add_argument(
        "--http-request-timeout",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta(),
        help=("Set the timeout of http request. "
              f"Format: {DurationParser.help()}. "
              "When not specified, there will be no timeout."))

    splashscreen_group = browser_group.add_mutually_exclusive_group()
    splashscreen_group.add_argument(
        "--splash-screen",
        "--splashscreen",
        "--splash",
        type=SplashScreen.parse,
        default=SplashScreen.DETAILED,
        help=("Set the splashscreen shown before each run. "
              "Choices: 'default', 'none', 'minimal', 'detailed,' or "
              "a path or a URL."))
    splashscreen_group.add_argument(
        "--no-splash",
        "--nosplash",
        dest="splash_screen",
        const=SplashScreen.NONE,
        action="store_const",
        help="Shortcut for --splash-screen=none")

    viewport_group = browser_group.add_mutually_exclusive_group()
    # pytype: disable=missing-parameter
    viewport_group.add_argument(
        "--viewport",
        default=Viewport.DEFAULT,
        type=Viewport.parse,
        help=("Set the browser window position."
              "Options: size and position, "
              f"{', '.join(str(e) for e in ViewportMode)}. "
              "Examples: --viewport=1550x300 --viewport=fullscreen. "
              f"Default: {Viewport.DEFAULT}"))
    # pytype: enable=missing-parameter
    viewport_group.add_argument(
        "--headless",
        dest="viewport",
        const=Viewport.HEADLESS,
        action="store_const",
        help=("Start the browser in headless if supported. "
              "Equivalent to --viewport=headless."))

  def _add_env_arguments(self, parser: argparse.ArgumentParser) -> None:
    env_group = parser.add_argument_group("Environment Options")
    env_settings_group = env_group.add_mutually_exclusive_group()
    env_settings_group.add_argument(
        "--env",
        type=EnvConfig.parse,
        help=("Set default runner environment settings. "
              f"Possible values: {', '.join(ENV_CONFIG_PRESETS.keys())} "
              "or an inline hjson configuration (see --env-config). "
              "Mutually exclusive with --env-config"))
    env_settings_group.add_argument(
        "--env-config",
        type=EnvConfig.parse_config_path,
        help=("Path to an env.config.hjson file that specifies detailed "
              "runner environment settings and requirements. "
              "See config/env.config.hjson for more details. "
              "Mutually exclusive with --env"))

    env_group.add_argument(
        "--env-validation",
        default=ValidationMode.PROMPT,
        type=ValidationMode,  # type: ignore
        help=(
            "Set how runner env is validated (see also --env-config/--env):\n" +
            ValidationMode.help_text(indent=2)))
    env_group.add_argument(
        "--dry-run",
        action="store_true",
        default=False,
        help="Don't run any browsers or probes")

  def _add_network_arguments(self, parser: argparse.ArgumentParser) -> None:
    network_group = parser.add_argument_group("Network Options")
    network_settings_group = network_group.add_mutually_exclusive_group()
    network_settings_group.add_argument(
        "--network",
        type=NetworkConfig.parse,
        help=("Either an inline network config or a file path to full "
              "network config hjson file (see --network-config or "
              "'help network')."))
    network_settings_group.add_argument(
        "--network-config",
        metavar="DIR",
        type=NetworkConfig.parse_config_path,
        help=("Path to a full network config file. See `help network` "
              "for all options."))
    network_settings_group.add_argument(
        "--local-file-server",
        "--local-fileserver",
        "--file-server",
        "--fileserver",
        type=NetworkConfig.parse_local,
        metavar="DIR",
        dest="network",
        help=("Start a local http file server at the given directory. "
              "See `help network` for more options."))
    network_settings_group.add_argument(
        "--wpr",
        "--web-page-replay",
        type=NetworkConfig.parse_wpr,
        metavar="WPR_ARCHIVE",
        dest="network",
        help=("Use wpr.archive to replay network requests "
              "via a local proxy server. "
              "Archives can be recorded with --probe=wpr. "
              "WPR_ARCHIVE can be a local file or a gs:// google storage url. "
              "See `help network` for more options."))

  def _add_timing_arguments(self, parser: argparse.ArgumentParser) -> None:
    timing_group = parser.add_argument_group("Time & Timeout Options")
    cooldown_group = timing_group.add_mutually_exclusive_group()
    cooldown_group.add_argument(
        "--cool-down-threshold",
        type=ThermalStatus.parse,
        help=("Pause execution when the device reaches this thermal status. "
              "Execution resumes once the status drops below the threshold. "
              "Only available on Android."))
    cooldown_group.add_argument(
        "--cool-down-time",
        "--cool-down",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta(seconds=2),
        help=("Wait between repetitions for a fixed amount of time. "
              f"Format: {DurationParser.help()}"))
    cooldown_group.add_argument(
        "--no-cool-down",
        action="store_const",
        dest="cool_down_time",
        const=dt.timedelta(seconds=0),
        help=("Disable cool-down between runs (might cause CPU throttling), "
              "equivalent to --cool-down=0."))
    cooldown_group.add_argument(
        "--fast",
        action=EnableFastAction,
        nargs=0,
        help=("Switch to a fast run mode "
              "which might yield unstable performance results. "
              "Equivalent to --cool-down=0 --no-splash --env-validation=skip."))

    timing_group.add_argument(
        "--time-unit",
        type=DurationParser.any_duration,
        default=dt.timedelta(seconds=1),
        help=("Absolute duration of 1 time unit in the runner. "
              "Increase this for slow builds or machines. "
              f"Format: {DurationParser.help()}"))
    timing_group.add_argument(
        "--timeout-unit",
        type=DurationParser.any_duration,
        default=dt.timedelta(),
        help=("Absolute duration of 1 time unit for timeouts in the runner. "
              "Unlike --time-unit, this does only apply for timeouts, "
              "as opposed to say initial wait times or sleeps. "
              f"Format: {DurationParser.help()}"))
    timing_group.add_argument(
        "--run-timeout",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta(),
        help=("Sets the same timeout per run on all browsers. "
              "Runs will be aborted after the given timeout. "
              f"Format: {DurationParser.help()}"))
    timing_group.add_argument(
        "--start-delay",
        "--startup-delay",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta(),
        help=("Delay before running the core workload, "
              "after a story's/workload's setup, "
              "and after starting the browser."))
    timing_group.add_argument(
        "--stop-delay",
        type=DurationParser.positive_or_zero_duration,
        default=dt.timedelta(),
        help=("Delay after running the core workload, "
              "before story's/workload's teardown, "
              "and before quitting the browser."))

  def _add_debugging_arguments(self, parser: argparse.ArgumentParser) -> None:
    debug_group = self.cli.add_debugging_arguments(parser)
    debug_group.add_argument(
        "--driver-logging",
        "--verbose-driver",
        action="store_true",
        default=False,
        help=("Enable verbose webdriver logging. "
              "Disabled by default, automatically enable with --debug"))
    debugger_group = debug_group.add_mutually_exclusive_group()
    debugger_group.add_argument(
        "--gdb",
        action=AppendDebuggerProbeAction,
        nargs=0,
        dest="probe",
        help=("Launch chrome with gdb or lldb attached to all processes. "
              " See 'describe probe debugger' for more options."))
    debugger_group.add_argument(
        "--lldb",
        action=AppendDebuggerProbeAction,
        nargs=0,
        dest="probe",
        help=("Launch chrome with lldb attached to all processes."
              " See 'describe probe debugger' for more options."))

  @override
  def run(self, args: argparse.Namespace) -> None:
    benchmark: Benchmark | None = None
    if args.cache_dir:
      plt.PLATFORM.set_cache_dir(args.cache_dir)
    self._helper(args)
    try:
      self._process_args(args)
      benchmark = self._get_benchmark(args)
      with plt.PLATFORM.TemporaryDirectory(
          prefix="crossbench") as tmp_dirname, WakeLock(plt.PLATFORM):
        self._run(args, benchmark, tmp_dirname)
    except KeyboardInterrupt:
      sys.exit(2)
    except LateArgumentError as e:
      if args.throw:
        raise
      self.cli.handle_late_argument_error(e)
    except Exception as e:  # pylint: disable=broad-except
      if args.throw:
        raise
      self._log_benchmark_subcommand_failure(benchmark, self._runner, e)
      sys.exit(3)

  def _run(self, args: argparse.Namespace, benchmark: Benchmark,
           tmp_dirname: pth.AnyPath) -> None:
    if args.dry_run:
      args.out_dir = pth.LocalPath(tmp_dirname) / "results"
    args.browser = self._get_browsers(args)
    probes: Sequence[Probe] = self._get_probes(args)
    env_config: EnvConfig = self._get_env_config(args)
    env_validation_mode: ValidationMode = self._get_env_validation_mode(args)
    timing: Timing = self._get_timing(args)
    self._runner = self._get_runner(args, benchmark, env_config,
                                    env_validation_mode, timing)

    # We prevent running multiple stories in repetition OR if multiple
    # browsers are open when 'power' probes are used since it might distort
    # the data.
    if len(args.browser) > 1 or args.repetitions > 1:
      probe_names = [probe.name for probe in probes if probe.BATTERY_ONLY]
      if probe_names:
        names_str = ",".join(probe_names)
        raise argparse.ArgumentTypeError(
            f"Cannot use [{names_str}] probe(s) "
            "with repeat > 1 and/or with multiple browsers. We need to "
            "always start at the same battery level, and by running "
            "stories on multiple browsers or multiples time will create "
            "erroneous data.")

    for probe in probes:
      self.runner.attach_probe(probe, matching_browser_only=True)

    self._run_benchmark(args, self.runner)

  def _helper(self, args: argparse.Namespace) -> None:
    """Handle common subcommand mistakes that are not easily implementable
    with argparse.
    run      => just run the benchmark
    help     => use --help
    describe => use describe benchmark NAME
    """
    if not args.other_browser_args:
      return
    maybe_command = args.other_browser_args[0]
    if maybe_command == "run":
      args.other_browser_args.pop()
      return
    if maybe_command == "help":
      self._parser.print_help()
      sys.exit(0)
    if maybe_command == "describe":
      logging.warning("See `describe benchmark %s` for more options",
                      self._benchmark_cls.NAME)
      # Patch args to simulate: describe benchmark BENCHMARK_NAME
      args.category = "benchmarks"
      args.filter = self._benchmark_cls.NAME
      args.json = False
      self.cli.describe_subcommand.run(args)
      sys.exit(0)

  def _process_args(self, args) -> None:
    if args.config:
      self._process_config_args(args)
    else:
      # We keep separate *_config args so we can throw in case they conflict
      # with --config. Since we don't use argparse's dest, we have to manually
      # copy the args.*_config back.
      self._process_network_args(args)

  def _process_network_args(self, args) -> None:
    # The order of preference of flags is as follows:
    # Explicitly specified network config > explicitly specified network >
    # benchmark-specific network config > default network.
    if network_config := args.network_config:
      args.network = network_config
    elif args.network:
      pass
    elif network_config := self._benchmark_cls.default_network_config_path():
      args.network = network_config
    else:
      args.network = NetworkConfig.default()

  def _process_env_args(self, args) -> None:
    if env_config := args.env_config:
      args.env = env_config
    elif args.env:
      pass
    else:
      args.env = EnvConfig.default()

  def _process_config_args(self, args) -> None:
    if args.env_config:
      raise argparse.ArgumentTypeError(
          "--config cannot be used together with --env-config")
    if args.network_config:
      raise argparse.ArgumentTypeError(
          "--config cannot be used together with --network-config")
    if args.browser_config:
      raise argparse.ArgumentTypeError(
          "--config cannot be used together with --browser-config")
    if args.probe_config:
      raise argparse.ArgumentTypeError(
          "--config cannot be used together with --probe-config")

    config_file = args.config
    config_data = ObjectParser.hjson_file(config_file)
    found_any_config = False

    if env_config_data := config_data.get("env"):
      args.env = EnvConfig.parse(env_config_data)
      found_any_config = True
    else:
      logging.warning("Skipping env config: no 'env' property in %s",
                      config_file)
    if not args.env:
      args.env = EnvConfig.default()

    if network_config_data := config_data.get("network"):
      # TODO: migrate all --config helper to this format
      args.network = NetworkConfig.parse(network_config_data)
      found_any_config = True
    else:
      logging.warning("Skipping network config: no 'network' property in %s",
                      config_file)
    if not args.network:
      args.network = NetworkConfig.default()

    if config_data.get("browsers"):
      args.browser_config = config_file
      found_any_config = True
    else:
      logging.warning("Skipping browsers config: No 'browsers' property in %s",
                      config_file)

    if config_data.get("probes"):
      args.probe_config = config_file
      found_any_config = True
    else:
      logging.warning("Skipping probes config: no 'probes' property in %s",
                      config_file)

    if not found_any_config:
      raise argparse.ArgumentTypeError(
          f"--config: config file has no config properties {config_file}")

  def _log_benchmark_subcommand_failure(self, benchmark: Optional[Benchmark],
                                        runner: Optional[Runner],
                                        e: Exception) -> None:
    logging.debug(e)
    logging.error("")
    logging.error("#" * 80)
    message: str = "SUBCOMMAND"
    if benchmark:
      message = f"{benchmark.NAME.upper()} BENCHMARK"
    logging.error("%s FAILED WITH %s:", message, e.__class__.__name__)
    logging.error("-" * 80)
    self._log_benchmark_subcommand_exception(e)
    logging.error("-" * 80)
    if runner and runner.runs:
      self._log_runner_debug_hints(runner)
    else:
      logging.error("- Check %s.json detailed backtraces", ErrorsProbe.NAME)
    logging.error(
        "- Use --debug for very verbose output (equivalent to --throw -vvv)")
    logging.error("#" * 80)
    sys.exit(3)

  def _log_benchmark_subcommand_exception(self, e: Exception) -> None:
    message = str(e)
    if message:
      logging.error(message)
      return
    if isinstance(e, AssertionError):
      self.cli.log_assertion_error_statement(e)

  def _run_benchmark(self, args: argparse.Namespace, runner: Runner) -> None:
    try:
      runner.run(is_dry_run=args.dry_run)
      logging.info("")
      self._log_results(args, runner, is_success=runner.is_success)
    except:  # pylint: disable=broad-except
      self._log_results(args, runner, is_success=False)
      raise
    finally:
      self._update_symlinks(args, runner)

  def _log_results(self, args: argparse.Namespace, runner: Runner,
                   is_success: bool) -> None:
    logging.info("=" * 80)
    if is_success:
      logging.critical("RESULTS: %s", runner.out_dir)
    else:
      logging.critical("RESULTS (maybe incomplete/broken): %s", runner.out_dir)
    logging.info("=" * 80)
    self._log_run_annotations(runner)
    if not runner.has_browser_group:
      logging.debug("No browser group in %s", runner)
      return
    browser_group = runner.browser_group
    for probe in runner.probes:
      try:
        probe.log_browsers_result(browser_group)
      except Exception as e:  # pylint: disable=broad-except
        if args.throw:
          raise
        logging.warning("log_result_summary failed: %s", e)

  def _log_run_annotations(self, runner: Runner) -> None:
    all_annotations = set(
        itertools.chain.from_iterable(run.annotations for run in runner.runs))
    RunAnnotation.log_all(all_annotations)

  def _update_symlinks(self, args: argparse.Namespace, runner: Runner) -> None:
    if not args.create_symlinks:
      return
    runner.update_symlinks()
    if not args.out_dir:
      self._update_default_results_symlinks(args, runner)

  def _update_default_results_symlinks(self, args: argparse.Namespace,
                                       runner: Runner) -> None:
    assert args.create_symlinks
    results_root = runner.out_dir.parent
    latest_link = results_root / "latest"
    if latest_link.is_symlink():
      latest_link.unlink()
    if not latest_link.exists():
      latest_link.symlink_to(
          runner.out_dir.relative_to(results_root), target_is_directory=True)
    else:
      logging.error("Could not create %s", latest_link)

  def _get_browsers(self, args: argparse.Namespace) -> Sequence[Browser]:
    # TODO: move browser instance create to separate method.
    # TODO: move --browser-config parsing to BrowserVariantsConfig
    args.browser_config = BrowserVariantsConfig.parse_args(args)
    browsers = args.browser_config.browsers
    return browsers

  def _get_probes(self, args: argparse.Namespace) -> Sequence[Probe]:
    # TODO: move probe creation to separate method
    # TODO: move --probe-config parsing to ProbeListConfig
    args.probe_config = ProbeListConfig.from_cli_args(args)
    return args.probe_config.probes

  def _get_benchmark(self, args: argparse.Namespace) -> Benchmark:
    benchmark_cls: Type[Benchmark] = self._get_benchmark_cls(args)
    assert (issubclass(benchmark_cls, Benchmark)), (
        f"benchmark_cls={benchmark_cls} is not subclass of Runner")
    with exception.annotate_argparsing(
        f"Parsing {benchmark_cls.NAME} arguments"):
      return benchmark_cls.from_cli_args(args)
    raise exception.UnreachableError()

  def _get_benchmark_cls(self, args: argparse.Namespace) -> Type[Benchmark]:
    del args
    return self._benchmark_cls

  def _get_env_validation_mode(self,
                               args: argparse.Namespace) -> ValidationMode:
    return args.env_validation

  def _get_env_config(self, args: argparse.Namespace) -> EnvConfig:
    return args.env

  def _get_timing(self, args: argparse.Namespace) -> Timing:
    timeout_unit: dt.timedelta = args.timeout_unit or args.time_unit
    return Timing(args.cool_down_time, args.time_unit, timeout_unit,
                  args.run_timeout, args.start_delay, args.stop_delay)

  def _get_runner(self, args: argparse.Namespace, benchmark: Benchmark,
                  env_config: EnvConfig, env_validation_mode: ValidationMode,
                  timing: Timing) -> Runner:
    runner_kwargs = self._runner_cls.kwargs_from_cli(args)
    return self._runner_cls(
        benchmark=benchmark,
        env_config=env_config,
        env_validation_mode=env_validation_mode,
        timing=timing,
        **runner_kwargs)

  def _log_runner_debug_hints(self, runner: Runner) -> None:
    failed_runs = [run for run in runner.runs if not run.is_success]
    if not failed_runs:
      return
    candidates: list[pth.LocalPath] = [
        *runner.out_dir.glob(f"{ErrorsProbe.NAME}*"),
    ]
    for failed_run in failed_runs:
      candidates.extend(failed_run.out_dir.glob(f"{ErrorsProbe.NAME}*"))
      candidates.extend(failed_run.out_dir.glob("*.log"))

    failed_run = failed_runs[0]
    logging.error("- Check log outputs (example 1 of %d failed runs):",
                  len(failed_runs))
    limit = 3
    for log_file in candidates[:limit]:
      try:
        log_file = log_file.relative_to(pth.LocalPath.cwd())
      except Exception as e:  # pylint: disable=broad-except
        logging.debug("Could not create relative log_file: %s", e)
      logging.error("  - %s", log_file)
    if (pending := len(candidates) - limit) > 0:
      logging.error("  - ... and %d more interesting %s.json or *.log files",
                    pending, ErrorsProbe.NAME)
