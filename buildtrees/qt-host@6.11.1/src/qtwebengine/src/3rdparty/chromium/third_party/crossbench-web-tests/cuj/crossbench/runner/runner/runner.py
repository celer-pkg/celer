# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import logging
import shlex
import tempfile

from pathlib import Path
from typing import Any, Dict, List, Optional

from crossbench import hjson as cb_hjson
from crossbench.cli.cli import CrossBenchCLI
from crossbench.helper.cwd import ChangeCWD

from runner.run_config import TargetPlatform, WebTestsRunConfig


def execute_crossbench(
    cb_benchmark_name: str,
    probe_config_file: Path,
    browser_config: str,
    additional_crossbench_args: str,
    debug: bool,
    dry_run: bool,
    results_path: Path,
    playback: Optional[str] = None,
    page_config_file: Optional[Path] = None,
    secrets_file: Optional[Path] = None,
) -> None:
  with tempfile.NamedTemporaryFile() as browser_config_file:
    browser_config_file.write(browser_config.encode("utf-8"))
    browser_config_file.seek(0)

    crossbench_args: List[str] = []

    crossbench_args.append(cb_benchmark_name)

    crossbench_args.append("--out-dir")
    crossbench_args.append(str(results_path))

    if page_config_file:
      crossbench_args.append("--page-config")
      crossbench_args.append(str(page_config_file))

    crossbench_args.append("--probe-config")
    crossbench_args.append(str(probe_config_file))

    crossbench_args.append("--browser-config")
    crossbench_args.append(str(browser_config_file.name))

    if secrets_file:
      crossbench_args.append("--secrets")
      crossbench_args.append(str(secrets_file))

    if playback:
      crossbench_args.append("--playback")
      crossbench_args.append(playback)

    if debug:
      crossbench_args.append("--debug")

    if dry_run:
      crossbench_args.append("--dry-run")
      crossbench_args.append("--env-validation")
      crossbench_args.append("skip")

    crossbench_args.append("--throw")

    for arg in shlex.split(additional_crossbench_args):
      crossbench_args.append(arg)

    logging.info("Running crossbench with args: %s", crossbench_args)

    CrossBenchCLI().run(crossbench_args)


def is_page_config(filename: str) -> bool:
  return filename.endswith("page-config.hjson")


def get_test_variant(page_config_filename: str) -> str:
  name_sections: List[str] = page_config_filename.split(".")

  if len(name_sections) <= 2:
    return ""

  return name_sections[0]


def get_android_browser_config(run_config: WebTestsRunConfig,
                               browser_flags_file: Path,
                               extensions: Any) -> Dict[str, Any]:
  # Default to normal chrome for android
  browser_string = "chrome"

  if run_config.browser:
    browser_string = run_config.browser

  return {
      "flags": str(browser_flags_file),
      "browsers": {
          browser_string: {
              "browser": browser_string,
              "flags": ["flags"],
              "extensions": extensions,
              "driver": {
                  "type": "adb",
                  "device_id": run_config.device_id
              }
          }
      }
  }


def get_chromeos_browser_config(run_config: WebTestsRunConfig,
                                browser_flags_file: Path,
                                extensions: Any) -> Dict[str, Any]:
  # Default to normal chrome for ChromeOS
  browser_string = "/opt/google/chrome/chrome"

  if run_config.browser:
    browser_string = run_config.browser

  return {
      "flags": str(browser_flags_file),
      "browsers": {
          browser_string: {
              "browser": browser_string,
              "flags": ["flags"],
              "extensions": extensions,
              "driver": {
                  "type": "chromeos-ssh",
                  "settings": {
                      "host": run_config.device_id,
                      # TODO support different ports
                      "ssh_port": 22,
                      "ssh_user": "root",
                  }
              }
          }
      }
  }


def get_local_browser_config(run_config: WebTestsRunConfig,
                             browser_flags_file: Path,
                             extensions: Any) -> Dict[str, Any]:
  logging.warning(
      "The 'local' platform is not officially supported by this script. "
      "You may need to tweak the probe config manually for some tests to pass."
  )

  # Default to normal chrome for local
  browser_string = "chrome"

  if run_config.browser:
    browser_string = run_config.browser

  return {
      "flags": str(browser_flags_file),
      "browsers": {
          browser_string: {
              "browser": browser_string,
              "flags": ["flags"],
              "extensions": extensions,
          }
      }
  }


def get_browser_config(run_config: WebTestsRunConfig, browser_flags_file: Path,
                       extensions: Any) -> str:
  # TODO support different chrome versions (i.e. dev/beta)

  if run_config.platform == TargetPlatform.ANDROID:
    config_dict = get_android_browser_config(run_config, browser_flags_file,
                                             extensions)
  elif run_config.platform == TargetPlatform.CHROME_OS:
    config_dict = get_chromeos_browser_config(run_config, browser_flags_file,
                                              extensions)
  elif run_config.platform == TargetPlatform.LOCAL:
    config_dict = get_local_browser_config(run_config, browser_flags_file,
                                           extensions)
  else:
    raise ValueError(f"Unsupported platform type: {run_config.platform}")

  return json.dumps(config_dict)


def get_additional_crossbench_args(test_path: Path,
                                   web_tests_root: Path,
                                   test_variant: str = "") -> str:
  additional_crossbench_args_file: Path = test_path / f"{test_variant}.cb-args"

  if not additional_crossbench_args_file.is_file():
    additional_crossbench_args_file = test_path / "cb-args"

  additional_crossbench_args: str = ""
  if additional_crossbench_args_file.is_file():
    additional_crossbench_args = additional_crossbench_args_file.read_text()

  return additional_crossbench_args.replace("$[WEB_TESTS]", str(web_tests_root))


def load_extensions(extension_config_file: Path) -> Any:
  if not extension_config_file.is_file():
    return None

  with extension_config_file.open(encoding="utf-8") as f:
    extensions = cb_hjson.load_unique_keys(f)
  # Allow exentions config files to be a reference to another file.
  if isinstance(extensions, str):
    with ChangeCWD(extension_config_file.parent):
      extensions = load_extensions(Path(extensions))
  return extensions

def run_benchmark(
    benchmark_path: Path,
    run_config: WebTestsRunConfig,
) -> List[str]:
  with ChangeCWD(benchmark_path):
    benchmark_name: str = benchmark_path.name
    benchmark_results_path: Path = run_config.results_path / benchmark_name
    probe_config_file: Path = benchmark_path / "probe-config.hjson"
    browser_flags_file: Path = benchmark_path / "browser-flags.hjson"
    extensions = load_extensions(benchmark_path / "extensions.hjson")
    browser_config = get_browser_config(run_config, browser_flags_file,
                                        extensions)

    logging.info("Executing crossbench for CUJ: %s", benchmark_name)

    try:
      execute_crossbench(
          cb_benchmark_name=benchmark_name,
          probe_config_file=probe_config_file,
          browser_config=browser_config,
          additional_crossbench_args=get_additional_crossbench_args(
              benchmark_path, run_config.web_tests_root),
          debug=run_config.debug,
          dry_run=run_config.dry_run,
          results_path=benchmark_results_path,
      )
    # pylint: disable=broad-exception-caught
    except Exception as e:
      logging.error(e)
      logging.error("Crossbench invocation for %s failed.", benchmark_name)
      return [benchmark_name]

    return []


def run_cuj(
    cuj_path: Path,
    run_config: WebTestsRunConfig,
) -> List[str]:
  with ChangeCWD(cuj_path):
    cuj_name: str = cuj_path.name

    failed_cujs: List[str] = []

    for config_file in cuj_path.iterdir():
      filename: str = config_file.name

      if not is_page_config(filename):
        continue

      cuj_variant: str = get_test_variant(filename)

      if not run_config.variants_regex.fullmatch(cuj_variant):
        continue

      full_cuj_name = cuj_name

      if cuj_variant:
        full_cuj_name = full_cuj_name + f"_{cuj_variant}"

      variant_results_path: Path = run_config.results_path / full_cuj_name

      page_config_file: Path = config_file

      probe_config_file: Path = cuj_path / f"{cuj_variant}.probe-config.hjson"

      if not probe_config_file.is_file():
        probe_config_file = cuj_path / "probe-config.hjson"

      browser_flags_file: Path = cuj_path / f"{cuj_variant}.browser-flags.hjson"

      if not browser_flags_file.is_file():
        browser_flags_file = cuj_path / "browser-flags.hjson"

      extension_config_file: Path = cuj_path / f"{cuj_variant}.extensions.hjson"
      if not extension_config_file.is_file():
        extension_config_file = cuj_path / "extensions.hjson"
      extensions = load_extensions(extension_config_file)

      browser_config = get_browser_config(run_config, browser_flags_file,
                                          extensions)

      logging.info("Executing crossbench for CUJ: %s", full_cuj_name)

      try:
        execute_crossbench(
            cb_benchmark_name="loading",
            probe_config_file=probe_config_file,
            browser_config=browser_config,
            additional_crossbench_args=get_additional_crossbench_args(
                cuj_path, run_config.web_tests_root, cuj_variant),
            debug=run_config.debug,
            dry_run=run_config.dry_run,
            results_path=variant_results_path,
            playback=run_config.playback,
            page_config_file=page_config_file,
            secrets_file=run_config.secrets_file,
        )
      # pylint: disable=broad-exception-caught
      except Exception as e:
        logging.error(e)
        logging.error("Crossbench invocation for %s failed.", full_cuj_name)
        failed_cujs.append(full_cuj_name)

    return failed_cujs
