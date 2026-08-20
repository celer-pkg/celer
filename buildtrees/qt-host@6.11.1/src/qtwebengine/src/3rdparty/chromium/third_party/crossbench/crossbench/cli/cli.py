# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import argparse
import logging
import os
import sys
import textwrap
import traceback
from typing import TYPE_CHECKING, Any, Optional, Sequence, Type

import tabulate as tbl

import crossbench.benchmarks.all as benchmarks
from crossbench import __version__
from crossbench import path as pth
from crossbench.cli import exception_formatter, ui
from crossbench.cli.parser import CrossBenchArgumentParser
from crossbench.cli.subcommand.benchmark import BenchmarkSubcommand
from crossbench.cli.subcommand.describe import DescribeSubcommand
from crossbench.cli.subcommand.devtools_recorder_proxy.subcommand import \
    DevtoolsRecorderProxySubcommand
from crossbench.cli.subcommand.help import HelpSubcommand
from crossbench.cli.subcommand.version import VersionSubcommand
from crossbench.helper.collection_helper import close_matches_message
from crossbench.probes.all import GENERAL_PURPOSE_PROBES
from crossbench.runner.runner import Runner

if TYPE_CHECKING:
  from crossbench.benchmarks.base import Benchmark
  from crossbench.browsers.browser import Browser
  from crossbench.cli.subcommand.base import CrossbenchSubcommand
  from crossbench.parse import LateArgumentError
  BenchmarkClsT = Type[Benchmark]
  BrowserLookupTableT = dict[str, tuple[Type[Browser], pth.LocalPath]]


class CrossBenchArgumentError(argparse.ArgumentError):
  """Custom class that also prints the argument.help if available.
  """

  def __init__(self, argument: Any, message: str) -> None:
    self.help: str = ""
    super().__init__(argument, message)
    if self.argument_name:
      self.help = getattr(argument, "help", "")

  def __str__(self) -> str:
    formatted = super().__str__()
    if not self.help:
      return formatted
    return (f"argument error {self.argument_name}:\n\n"
            f"Help {self.argument_name}:\n{self.help}\n\n"
            f"{formatted}")


argparse.ArgumentError = CrossBenchArgumentError  # type: ignore

class EnableDebuggingAction(argparse.Action):
  """Custom action to set both --throw and -vvv."""

  def __call__(self,
               parser: argparse.ArgumentParser,
               namespace: argparse.Namespace,
               values: str | Sequence[Any] | None,
               option_string: Optional[str] = None) -> None:
    setattr(namespace, "throw", True)
    setattr(namespace, "verbosity", 3)
    setattr(namespace, "driver_logging", True)


class MainCrossBenchArgumentParser(CrossBenchArgumentParser):

  def print_help(self, file=None) -> None:
    super().print_help(file=file)
    self.print_probes(file=file)
    self.print_urls(file=file)
    self.print_example(file=file)

  def print_probes(self, file=None) -> None:
    lines = [
        "Probes can be added and configured for each benchmark.",
        f"Use `{sys.argv[0]} describe probe $PROBE` for the full help.",
        "",
        "Usage: --probe=v8.log --probe=video ...",
        "Usage: --probe=v8.log:{log_all:false} ...",
        "Usage: --probe-config=configs/probe/perfetto/default.config.hjson",
        "",
    ]
    table = []
    for probe_cls in GENERAL_PURPOSE_PROBES:
      table.append((probe_cls.NAME, probe_cls.summary_text()))
    lines.append(tbl.tabulate(table, tablefmt="plain"))
    contents = "\n".join(lines)
    file = file or sys.stdout
    file.write("\n")
    file.write("Available Probes for all Benchmarks:\n")
    file.write(textwrap.indent(contents, "    "))
    file.write("\n")

  def print_urls(self, file=None) -> None:
    file = file or sys.stdout
    file.write("\n")
    file.write("URLS:\n")
    file.write("  Source: https://chromium.googlesource.com/crossbench\n")
    file.write("  Bugs:   "
               "https://issues.chromium.org/u/1/issues/new?component=1456712\n")

  def print_example(self, file=None) -> None:
    file = file or sys.stdout
    file.write("\n")
    file.write("EXAMPLE:\n")
    file.write("  ./cb.py speedometer --browser=chrome-m131 "
               "--browser=out/release/chrome --probe=profiling\n\n")
    readme_file = pth.AnyPath(__file__).parents[2] / "README.md"
    file.write(f"  See {readme_file} for more details and instructions.\n")

class CrossBenchCLI:
  BENCHMARKS: tuple[BenchmarkClsT, ...] = (
      benchmarks.EmbedderBenchmark,
      # JetStream:
      benchmarks.JetStream11Benchmark,
      benchmarks.JetStream20Benchmark,
      benchmarks.JetStream21Benchmark,
      benchmarks.JetStream22Benchmark,
      benchmarks.JetStreamMainBenchmark,
      # Loading:
      benchmarks.LoadingBenchmark,
      # LoadLine:
      benchmarks.LoadLine1PhoneBenchmark,
      benchmarks.LoadLine1PhoneDebugBenchmark,
      benchmarks.LoadLine1PhoneFastBenchmark,
      benchmarks.LoadLine1TabletBenchmark,
      benchmarks.LoadLine1TabletDebugBenchmark,
      benchmarks.LoadLine1TabletFastBenchmark,
      # LoadLine 2:
      benchmarks.LoadLine2PhoneBenchmark,
      benchmarks.LoadLine2PhoneDebugBenchmark,
      benchmarks.LoadLine2TabletBenchmark,
      benchmarks.LoadLine2TabletDebugBenchmark,
      # Manual:
      benchmarks.ManualBenchmark,
      # Memory:
      benchmarks.MemoryBenchmark,
      # Motionmark
      benchmarks.MotionMark10Benchmark,
      benchmarks.MotionMark11Benchmark,
      benchmarks.MotionMark12Benchmark,
      benchmarks.MotionMark13Benchmark,
      benchmarks.MotionMark131Benchmark,
      benchmarks.MotionMarkMainBenchmark,
      # Powerline
      benchmarks.PowerlineBenchmark,
      # Speedometer:
      benchmarks.Speedometer10Benchmark,
      benchmarks.Speedometer20Benchmark,
      benchmarks.Speedometer21Benchmark,
      benchmarks.Speedometer30Benchmark,
      benchmarks.Speedometer31Benchmark,
      benchmarks.SpeedometerMainBenchmark,
  )

  RUNNER_CLS: Type[Runner] = Runner

  def __init__(self, enable_logging: bool = True) -> None:
    self._enable_logging: bool = enable_logging
    self._console_handler: logging.StreamHandler | None = None
    self._benchmark_subcommands: dict[BenchmarkClsT, BenchmarkSubcommand] = {}
    self.parser = MainCrossBenchArgumentParser(
        description=("A cross browser and cross benchmark runner "
                     "with configurable measurement probes.\n"))
    self._subparsers = self._setup_subparsers()
    self._setup_parser()
    self._describe_subcommand = DescribeSubcommand(self)
    self._help_subcommand = HelpSubcommand(self)
    self._version_subcommand = VersionSubcommand(self)
    self._recorder_proxy_subcommand = DevtoolsRecorderProxySubcommand(self)
    self._last_subcommand: CrossbenchSubcommand | None = None
    self.args = argparse.Namespace()
    self._setup_subcommands()

  @property
  def subparsers(self):
    return self._subparsers

  @property
  def describe_subcommand(self) -> DescribeSubcommand:
    return self._describe_subcommand

  @property
  def help_subcommand(self) -> HelpSubcommand:
    return self._help_subcommand

  @property
  def last_subcommand(self) -> CrossbenchSubcommand | None:
    return self._last_subcommand

  def _setup_parser(self) -> None:
    self.add_debugging_arguments(self.parser)
    self.add_base_arguments(self.parser)

  def _setup_subparsers(
      self) -> argparse._SubParsersAction[CrossBenchArgumentParser]:
    subparsers = self.parser.add_subparsers(
        title="Subcommands",
        dest="subcommand",
        required=True,
        parser_class=CrossBenchArgumentParser)
    return subparsers

  def add_base_arguments(self, parser) -> None:
    # Disable colors by default when piped to a file.
    has_color = hasattr(sys.stdout, "isatty") and sys.stdout.isatty()
    parser.add_argument(
        "--no-color",
        dest="color",
        action="store_false",
        default=has_color,
        help="Disable colored output")
    parser.add_argument(
        "--version", action="version", version=f"%(prog)s {__version__}")

  def add_debugging_arguments(self, parser: argparse.ArgumentParser):
    debug_group = parser.add_argument_group("Verbosity / Debugging Options")
    verbosity_group = debug_group.add_mutually_exclusive_group()
    verbosity_group.add_argument(
        "--quiet",
        "-q",
        dest="verbosity",
        default=0,
        action="store_const",
        const=-1,
        help="Disable most output printing.")
    verbosity_group.add_argument(
        "--verbose",
        "-v",
        dest="verbosity",
        action="count",
        default=0,
        help=("Increase output verbosity. "
              "Repeat for more verbose output (0..2)."))

    debug_group.add_argument(
        "--throw",
        action="store_true",
        default=False,
        help=("Directly throw exceptions instead. "
              "Note that this prevents merging of probe results if only "
              "a subset of the runs failed."))
    debug_group.add_argument(
        "--debug",
        action=EnableDebuggingAction,
        nargs=0,
        help="Enable debug output, equivalent to --throw -vvv")
    return debug_group

  def _setup_subcommands(self) -> None:
    for benchmark_cls in self.BENCHMARKS:
      subcommand = BenchmarkSubcommand(self, benchmark_cls)
      self._benchmark_subcommands[benchmark_cls] = subcommand

  def log_assertion_error_statement(self, e: AssertionError) -> None:
    _, exc_exception, tb = sys.exc_info()
    if exc_exception is not e:
      return
    tb_info = traceback.extract_tb(tb)
    filename, line, _, text = tb_info[-1]
    logging.info("%s:%s: %s", filename, line, text)

  def run(self, argv: Sequence[str]) -> None:
    self._init_logging(argv)
    unprocessed_argv: list[str] = []
    try:
      argv = self._rename_subcommand(argv)
      # Manually check for unprocessed_argv to print nicer error messages.
      self.args, unprocessed_argv = self.parser.parse_known_args(argv)
    except argparse.ArgumentError as e:
      # args is not set at this point, as parsing might have failed before
      # handling --throw or --debug.
      if "--throw" in argv or "--debug" in argv:
        raise e
      self.error(str(e))
    if unprocessed_argv:
      self.error(f"unrecognized arguments: {unprocessed_argv}\n"
                 f"Use `{self.parser.prog} {self.args.subcommand} --help` "
                 "for more details.")
    # Properly initialize logging after having parsed all args
    self._setup_logging()
    try:
      self._last_subcommand = self.args.crossbench_subcommand
      self.args.crossbench_subcommand.run(self.args)
    finally:
      self._teardown_logging()

  def _rename_subcommand(self, argv: Sequence[str]) -> Sequence[str]:
    if not argv:
      return argv
    subcommand = argv[0]
    if subcommand.startswith("-"):
      return argv

    choices = set(self._subparsers.choices.keys())
    if subcommand in choices:
      return argv

    alternative: str | None = None
    for benchmark_cls in self._benchmark_subcommands:
      aliases = benchmark_cls.aliases()
      if subcommand in aliases:
        alternative = benchmark_cls.NAME
        break
      choices.update(benchmark_cls.aliases())

    if not alternative:
      message, alternative = close_matches_message(subcommand, set(choices))
      message = f"Unknown subcommand {repr(subcommand)}: {message}"
    if not alternative:
      raise argparse.ArgumentError(None, message)
    return [alternative, *argv[1:]]

  def handle_late_argument_error(self, e: LateArgumentError) -> None:
    self.error(f"error argument {e.flag}: {e.message}")

  def error(self, message: str) -> None:
    parser: argparse.ArgumentParser = self.parser
    # Try to use the subparser to print nicer usage help on errors.
    # ArgumentParser tends to default to the toplevel parser instead of the
    # current subcommand, which in turn prints the wrong usage text.
    subcommand_name: str = getattr(self.args, "subcommand", "")
    if subcommand_name == "describe":
      parser = self._describe_subcommand.parser
    else:
      maybe_benchmark_cls = getattr(self.args, "benchmark_cls", None)
      if maybe_benchmark_cls:
        parser = self._benchmark_subcommands[maybe_benchmark_cls].parser
    if subcommand_name:
      message = f"{subcommand_name}: {message}"
    if isinstance(parser, CrossBenchArgumentParser):
      parser.fail(message)
    else:
      parser.error(message)

  def _init_logging(self, argv: Sequence[str]) -> None:
    sys.excepthook = exception_formatter.excepthook
    assert self._console_handler is None
    if not self._enable_logging:
      logging.getLogger().setLevel(logging.CRITICAL)
      return
    if hasattr(sys.stdout, "reconfigure"):
      sys.stdout.reconfigure(encoding="utf-8")  # type: ignore[union-attr]
    self._console_handler = logging.StreamHandler(sys.stderr)
    self._console_handler.addFilter(logging.Filter("root"))
    self._console_handler.setLevel(logging.INFO)
    logging.getLogger().setLevel(logging.INFO)
    # Clear existing handlers in case logging has been initialized prematurely.
    logging.getLogger().handlers = []
    logging.getLogger().addHandler(self._console_handler)

    # Manually extract values to allow logging for failing arguments.
    if self._has_debug_logging_argv(argv):
      self._console_handler.setLevel(logging.DEBUG)
      logging.getLogger().setLevel(logging.DEBUG)
    # TODO: move to ui helpers
    ui.COLOR_LOGGING = self._detect_terminal_color(argv)
    if ui.COLOR_LOGGING:
      self._console_handler.setFormatter(ui.ColoredLogFormatter())
    logging.debug(" ".join(sys.argv))

  def _has_debug_logging_argv(self, argv: Sequence[str]) -> bool:
    return any(value in ("-v", "-vv", "-vvv", "--debug") for value in argv)

  def _detect_terminal_color(self, argv: Sequence[str]) -> bool:
    if "--no-color" in argv:
      return False
    if os.environ.get("NO_COLOR", ""):
      return False
    return True

  def _setup_logging(self) -> None:
    if not self._enable_logging:
      return
    assert self._console_handler
    if self.args.verbosity == -1:
      self._console_handler.setLevel(logging.ERROR)
    elif self.args.verbosity == 0:
      self._console_handler.setLevel(logging.INFO)
    elif self.args.verbosity >= 1:
      self._console_handler.setLevel(logging.DEBUG)
      logging.getLogger().setLevel(logging.DEBUG)
    if not self.args.color:
      ui.COLOR_LOGGING = False
    if ui.COLOR_LOGGING:
      self._console_handler.setFormatter(ui.ColoredLogFormatter())
    else:
      self._console_handler.setFormatter(None)

  def _teardown_logging(self) -> None:
    if not self._enable_logging:
      assert self._console_handler is None
      return
    assert self._console_handler
    self._console_handler.flush()
    logging.getLogger().removeHandler(self._console_handler)
    self._console_handler = None
