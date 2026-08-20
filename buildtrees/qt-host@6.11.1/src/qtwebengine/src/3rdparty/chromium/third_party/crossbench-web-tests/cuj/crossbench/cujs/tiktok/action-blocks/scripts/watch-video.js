// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const unitToSeconds = {
  s: 1,
  m: 60,
  h: 3600,
}
const parseTimeToSeconds = (timeString) => {
  const lowerCaseTimeString = timeString.toLowerCase();
  const match = lowerCaseTimeString.match(/^(\d+)([smh]?)$/);
  const [_, numStr, unit] = match;
  return parseInt(numStr, 10) * (unitToSeconds[unit] || 1);
}
// add video pause listener.
const addVideoPauseListener = () => {
  const video = document.querySelector('video');
  performance.mark('video-resolution', {
    detail: {
      videoHeight: video.videoHeight,
      videoWidth: video.videoWidth,
    }
  });
  const duration = Math.floor(video.duration);
  let endTime = parseTimeToSeconds("$[VIDEO_TIME]");
  if (duration < endTime) {
    endTime = duration;
  }
  const videoPauseListener = () => {
    if (video.currentTime >= endTime) {
      performance.mark('video-stop');
      performance.measure('video-duration', 'video-start', 'video-stop');
      video.removeEventListener('timeupdate', videoPauseListener);
      video.pause();
    }
  }
  video.addEventListener('timeupdate', videoPauseListener);
}
addVideoPauseListener();

// Tiktok might adjust video due to network connection,
// so we need to rebind the event listener when new video element appears.
const playerContainer = document.querySelector('div.tiktok-web-player');
const observer = new MutationObserver((mutations) => {
  mutations.forEach(mutation => {
    if (mutation.type === 'childList') {
      mutation.addedNodes.forEach(node => {
        if (node.tagName && node.tagName.toLowerCase() === 'video') {
          addVideoPauseListener();
        }
      });
    }
  });
});
observer.observe(playerContainer, { childList: true });