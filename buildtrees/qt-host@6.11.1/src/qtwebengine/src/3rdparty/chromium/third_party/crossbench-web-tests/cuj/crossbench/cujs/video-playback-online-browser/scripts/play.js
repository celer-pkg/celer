// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

performance.mark('page-loaded')
const testVideo = document.querySelector('video#video');
var finished = false;

testVideo.addEventListener('progress', (e) => {
  if (e.timeStamp > 70000) {
    finished = true;
    performance.mark('video-stop');
    performance.measure('video-duration', 'video-start', 'video-stop');

    testVideo.pause();
  }
});

testVideo.play().then(() => {
  performance.mark('video-start');
  performance.measure('video-ready', 'page-loaded', 'video-start');
});