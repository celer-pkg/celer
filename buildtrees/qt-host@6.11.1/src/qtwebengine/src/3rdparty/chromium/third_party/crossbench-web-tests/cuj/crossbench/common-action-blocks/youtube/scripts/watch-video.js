// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const video = document.querySelector("VIDEO_SELECTOR")
const duration = Math.floor(video.duration)
let endTime = VIDEO_TIME
if (duration < endTime) {
  endTime = duration
}
const videoListener = () => {
  if (video.currentTime >= endTime) {
    performance.mark("video-stop")
    const quality = video.getVideoPlaybackQuality();
    performance.mark("video-stop", {
      detail: {
        totalVideoFrames: quality.totalVideoFrames,
        droppedVideoFrames: quality.droppedVideoFrames,
      }
    })
    video.removeEventListener("timeupdate", videoListener)
    video.pause()
  }
}
video.addEventListener("timeupdate", videoListener)

video.play().then(() => {
  video.currentTime = 0
  performance.mark("video-start", {
    detail: {
      videoHeight: video.videoHeight,
      videoWidth: video.videoWidth,
    }
  });
});