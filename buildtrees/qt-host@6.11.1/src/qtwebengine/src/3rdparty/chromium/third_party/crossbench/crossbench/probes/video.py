# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import atexit
import logging
import os
import subprocess
import tempfile
from typing import TYPE_CHECKING, Self, TextIO, Type

from typing_extensions import Final, override

from crossbench.config import ConfigEnum
from crossbench.helper import collection_helper
from crossbench.probes.probe import Probe, ProbeConfigParser, ProbeContext
from crossbench.probes.probe_error import ProbeMissingDataError
from crossbench.probes.result_location import ResultLocation
from crossbench.probes.results import LocalProbeResult, ProbeResult

if TYPE_CHECKING:
  from crossbench.browsers.browser import Viewport
  from crossbench.env.runner_env import RunnerEnv
  from crossbench.path import LocalPath
  from crossbench.runner.groups.browsers import BrowsersRunGroup
  from crossbench.runner.groups.repetitions import RepetitionsRunGroup
  from crossbench.runner.run import Run
  from crossbench.stories.story import Story


class Orientation(ConfigEnum):
  HORIZONTAL = ("horizontal", "Align horizontally.")
  VERTICAL = ("vertical", "Align vertically.")


FFMPEG_STACK_DIRECTION: Final[dict[Orientation, str]] = {
    Orientation.HORIZONTAL: "hstack",
    Orientation.VERTICAL: "vstack",
}

class VideoProbe(Probe):
  """
  General-purpose Probe that collects screen-recordings.

  It can also produce a timestrip png and creates merged versions of these files
  for visually comparing various browsers / variants / cb.stories
  """
  NAME = "video"
  RESULT_LOCATION = ResultLocation.BROWSER
  VIDEO_QUALITY = ["-vcodec", "libx264", "-crf", "20"]
  IMAGE_FORMAT = "png"
  TIMESTRIP_FILE_SUFFIX = f".timestrip.{IMAGE_FORMAT}"
  FRAMERATE = 60

  @classmethod
  @override
  def config_parser(cls) -> ProbeConfigParser[Self]:
    parser = super().config_parser()
    parser.add_argument(
        "generate_timestrip",
        aliases=("timestrip",),
        type=bool,
        default=True,
        help="Produce a timestrip png")
    parser.add_argument(
        "merge_runs",
        type=bool,
        default=True,
        help="Merge videos from multiple runs")
    parser.add_argument(
        "orientation",
        aliases=(
            "dir",
            "direction",
        ),
        type=Orientation,
        default=Orientation.HORIZONTAL,
        help=("Primary merge orientation for repetitions. "
              "Each further merge level will happen in the opposite direction "
              "of the previous one."))
    return parser

  def __init__(self,
               generate_timestrip: bool = True,
               merge_runs: bool = True,
               orientation: Orientation = Orientation.HORIZONTAL) -> None:
    super().__init__()
    self._duration = None
    self._generate_timestrip = generate_timestrip
    self._merge_runs = merge_runs
    self._orientation = orientation

  @property
  @override
  def result_path_name(self) -> str:
    return f"{self.name}.mp4"

  @property
  def generate_timestrip(self) -> bool:
    return self._generate_timestrip

  @property
  def primary_orientation(self):
    return self._orientation

  @property
  def secondary_orientation(self):
    if self._orientation == Orientation.VERTICAL:
      return Orientation.HORIZONTAL
    return Orientation.VERTICAL

  @property
  def merge_runs(self) -> bool:
    return self._merge_runs

  @override
  def validate_env(self, env: RunnerEnv) -> None:
    super().validate_env(env)
    if env.repetitions > 10:
      env.handle_warning(
          f"Probe={self.NAME} might not be able to merge so many "
          f"repetitions={env.repetitions}.")
    env.check_installed(
        binaries=("ffmpeg",), message="Missing binaries for video probe: {}")
    # Check that ffmpeg can be executed
    env.check_sh_success("ffmpeg", "-version")
    env.check_installed(
        binaries=("montage",),
        message="Missing 'montage' binary, please install imagemagick.")
    # Check that montage can be executed
    env.check_sh_success("montage", "--version")
    self._pre_check_viewport_size(env)

  def _pre_check_viewport_size(self, env: RunnerEnv) -> None:
    first_viewport: Viewport = env.browsers[0].viewport
    for browser in env.browsers:
      viewport: Viewport = browser.viewport
      if viewport.is_headless:
        env.handle_warning(
            f"Cannot record video for headless browser: {browser}")
      # TODO: support fullscreen / maximised
      if not viewport.has_size:
        env.handle_warning(
            "Can only record video for browsers with explicit viewport sizes, "
            f"but got {viewport} for {browser}.")
      if viewport.x < 10 or viewport.y < 50:
        env.handle_warning(
            f"Viewport for '{browser}' might include toolbar: {viewport}")
      if viewport != first_viewport:
        env.handle_warning(
            "Video recording requires same viewport size for all browsers.\n"
            f"Viewport size for {browser} is {viewport}, "
            f"which differs from first viewport {first_viewport}. ")

  @override
  def get_context_cls(self) -> Type[VideoProbeContext]:
    return VideoProbeContext

  @override
  def merge_repetitions(self, group: RepetitionsRunGroup) -> ProbeResult:
    if not self.merge_runs:
      return LocalProbeResult()
    runs = tuple(group.runs)
    if len(runs) == 1:
      # In the simple case just copy the files
      run_files = runs[0].results[self].file_list
      group_files = [group.path / f.name for f in run_files]
      for src, dest in zip(run_files, group_files):
        self.host_platform.copy(src, dest)
      return LocalProbeResult(file=group_files)

    video_file = group.get_local_probe_result_path(self)
    group_files = [video_file]
    logging.info("VIDEO merge page repetitions")
    browser = group.browser
    video_file_inputs: list[str | LocalPath] = []
    for run in runs:
      video_file_inputs += ["-i", run.results[self].file_list[0]]
    draw_text = ("fontfile='/Library/Fonts/Arial.ttf':"
                 f"text='{browser.app_name} {browser.label}':"
                 "fontsize=h/15:"
                 "y=h-line_h-10:x=10:"
                 "box=1:boxborderw=20:boxcolor=white")
    stack_direction: str = FFMPEG_STACK_DIRECTION[self.primary_orientation]
    self.host_platform.sh(
        "ffmpeg", "-hide_banner", \
        *video_file_inputs, \
        "-filter_complex",
        f"{stack_direction}=inputs={len(runs)},"
        f"drawtext={draw_text},"
        "scale=3000:-2", *self.VIDEO_QUALITY, video_file)

    if self._generate_timestrip:
      timeline_strip_file = video_file.with_suffix(self.TIMESTRIP_FILE_SUFFIX)
      logging.info("TIMESTRIP merge page repetitions")
      timeline_strips = (run.results[self].file_list[1] for run in runs)
      self.host_platform.sh("montage", *timeline_strips, "-tile", "1x",
                            "-gravity", "NorthWest", "-geometry", "x100",
                            timeline_strip_file)
      group_files.append(timeline_strip_file)

    return LocalProbeResult(file=group_files)

  @override
  def merge_browsers(self, group: BrowsersRunGroup) -> ProbeResult:
    """Merge story videos from multiple browser/configurations"""
    groups = list(group.repetitions_groups)
    if not self.merge_runs or len(groups) <= 1:
      return super().merge_browsers(group)
    grouped: dict[Story,
                  list[RepetitionsRunGroup]] = collection_helper.group_by(
                      groups,
                      key=lambda repetitions_group: repetitions_group.story)

    result_dir = group.get_local_probe_result_path(self)
    result_dir = result_dir / result_dir.stem
    result_dir.mkdir(parents=True)
    return LocalProbeResult(
        file=(self._merge_stories_for_browser(result_dir, story,
                                              repetitions_groups)
              for story, repetitions_groups in grouped.items()))

  def _merge_stories_for_browser(
      self, result_dir: LocalPath, story: Story,
      repetitions_groups: list[RepetitionsRunGroup]) -> LocalPath:
    story = repetitions_groups[0].story
    result_path = result_dir / f"{story.name}_combined.mp4"

    if len(repetitions_groups) == 1:
      # In the simple case just copy files
      input_file = repetitions_groups[0].results[self].file_list[0]
      self.host_platform.copy(input_file, result_path)
      return result_path

    input_files: list[str] = []
    for repetitions_group in repetitions_groups:
      result_files = repetitions_group.results[self].file_list
      input_files += ["-i", os.fspath(result_files[0])]
    stack_direction: str = FFMPEG_STACK_DIRECTION[self.secondary_orientation]
    try:
      self.host_platform.sh(
          "ffmpeg", "-hide_banner", *input_files, "-filter_complex",
          f"{stack_direction}=inputs={len(repetitions_groups)}",
          *self.VIDEO_QUALITY, result_path)
    except Exception as e:
      logging.error("Merging multiple browser video failed. "
                    "Different screen orientations are not supported yet.")
      logging.debug("Browser video merging failed: %e", e)
      raise e
    return result_path


class VideoProbeContext(ProbeContext[VideoProbe]):
  IMAGE_FORMAT = "png"
  FFMPEG_TIMELINE_TEXT = (
      "drawtext="
      "fontfile=/Library/Fonts/Arial.ttf:"
      "text='%{eif\\:t\\:d}.%{eif\\:t*100-floor(t)*100\\:d}s':"
      "fontsize=h/16:"
      "y=h-line_h-5:x=5:"
      "box=1:boxborderw=15:boxcolor=white")

  def __init__(self, probe: VideoProbe, run: Run) -> None:
    super().__init__(probe, run)
    self._record_process: subprocess.Popen | None = None
    self._recorder_log_file: TextIO | None = None

  def start(self) -> None:
    browser = self.run.browser
    cmd = self._record_cmd(browser.viewport)
    logging.debug("Screen recorder cmd: %s", cmd)
    if self.browser_platform.is_remote:
      self._recorder_log_file = None
    else:
      self._recorder_log_file = self.local_result_path.with_suffix(
          ".recorder.log").open(
              "w", encoding="utf-8")
    self._record_process = self.browser_platform.popen(
        *cmd,
        stdin=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        stdout=self._recorder_log_file)
    if self._record_process.poll():
      raise ValueError("Could not start screen recorder")
    atexit.register(self.stop_process)
    # TODO: Add common start-story-delay on runner for these cases.
    self.host_platform.sleep(1)

  def _record_cmd(self, viewport: Viewport) -> tuple[str, ...]:
    if self.browser_platform.is_linux:
      env_display = os.environ.get("DISPLAY", ":0.0")
      return ("ffmpeg", "-hide_banner", "-video_size",
              f"{viewport.width}x{viewport.height}", "-f", "x11grab",
              "-framerate", str(self.probe.FRAMERATE), "-i",
              f"{env_display}+{viewport.x},{viewport.y}", str(self.result_path))
    if self.browser_platform.is_macos:
      return ("/usr/sbin/screencapture", "-v",
              f"-R{viewport.x},{viewport.y},{viewport.width},{viewport.height}",
              str(self.result_path))
    if self.browser_platform.is_android:
      return ("screenrecord", str(self.result_path))
    raise ValueError("Invalid platform")

  def stop(self) -> None:
    assert self._record_process, "screencapture stopped early."
    if self.browser_platform.is_macos:
      assert not self._record_process.poll(), (
          "screencapture stopped early. "
          "Please ensure that the parent application has screen recording "
          "permissions")
      # The mac screencapture stops on the first (arbitrary) input.
      self._record_process.communicate(input=b"stop")
    elif self.browser_platform.is_android:
      assert not self._record_process.poll(), ("screencapture stopped early. ")
      self.browser_platform.send_signal(
          self._record_process, signal=self.browser_platform.signals.SIGINT)
    else:
      self.browser_platform.terminate(self._record_process)

  def teardown(self) -> ProbeResult:
    assert self._record_process, "Screen recorder stopped early."
    if self._recorder_log_file:
      self._recorder_log_file.close()
    self.stop_process()
    if not self.browser_platform.is_file(self.result_path):
      raise ProbeMissingDataError(
          f"No screen recording video found at: {self.result_path}")
    # Copy files
    browser_result = self.browser_result(file=(self.result_path,))
    self._default_result_path = browser_result.file
    assert self.host_platform.exists(self.result_path)

    if not self.probe.generate_timestrip:
      return LocalProbeResult(file=(self.local_result_path,))

    with tempfile.TemporaryDirectory() as tmp_dir:
      self._convert_to_constant_framerate()
      timestrip_file = self._create_time_strip(
          self.host_platform.local_path(tmp_dir))
    return LocalProbeResult(file=(self.local_result_path, timestrip_file))

  def stop_process(self) -> None:
    if self._record_process:
      self.browser_platform.terminate_gracefully(self._record_process,
                                                 timeout=5)
      self._record_process = None

  def _convert_to_constant_framerate(self) -> None:
    # On some platforms (android for certain) we get VFR videos which confuse
    # the next video extraction / conversion steps.
    vrf_video_result = (
        self.local_result_path.parent / f"vfr_{self.result_path.name}")
    self.local_result_path.rename(vrf_video_result)
    self.host_platform.sh(
        "ffmpeg", "-hide_banner", \
        "-fflags", "+igndts", \
        "-i", vrf_video_result, \
        "-filter:v", "fps=60", \
        "-fps_mode:v", "cfr",
        # Use the decoder timebase.
        "-copytb", "0", \
        *self.probe.VIDEO_QUALITY,
        self.result_path
    )
    if not self.local_result_path.exists() or self.local_result_path.stat(
    ).st_size == 0:
      vrf_video_result.rename(self.result_path)
      logging.error("Could not generate constant FPS video: %s",
                    self.result_path)
    else:
      vrf_video_result.unlink()

  def _create_time_strip(self, tmpdir: LocalPath) -> LocalPath:
    logging.info("TIMESTRIP")
    progress_dir = tmpdir / "progress"
    progress_dir.mkdir(parents=True, exist_ok=True)
    timeline_dir = tmpdir / "timeline"
    timeline_dir.mkdir(exist_ok=True)
    # Try detect scene changes / steps
    self.host_platform.sh(
        "ffmpeg", "-hide_banner", "-i", self.result_path, \
        "-filter_complex", "scale=3000:-2,"
        "select='gt(scene\\,0.011)'," + self.FFMPEG_TIMELINE_TEXT, \
        "-fps_mode", "cfr", \
        "-framerate", str(self.probe.FRAMERATE), \
          f"{progress_dir}/%02d.{self.IMAGE_FORMAT}")
    # Extract at regular intervals of 100ms, assuming 60fps input
    every_nth_frame = self.probe.FRAMERATE / 20
    safe_duration = 10
    safe_duration = 2
    self.host_platform.sh(
        "ffmpeg", "-hide_banner", \
        "-i", self.result_path, \
        "-filter_complex",
        f"trim=duration={safe_duration},"
        "scale=3000:-2,"
        f"select=not(mod(n\\,{every_nth_frame}))," + self.FFMPEG_TIMELINE_TEXT,
        f"{timeline_dir}/%02d.{self.IMAGE_FORMAT}")

    timeline_strip_file = self.local_result_path.with_suffix(
        self.probe.TIMESTRIP_FILE_SUFFIX)
    self.runner.platform.sh("montage", f"{timeline_dir}/*.{self.IMAGE_FORMAT}",
                            "-tile", "x1", "-gravity", "NorthWest", "-geometry",
                            "x100", timeline_strip_file)
    return timeline_strip_file
