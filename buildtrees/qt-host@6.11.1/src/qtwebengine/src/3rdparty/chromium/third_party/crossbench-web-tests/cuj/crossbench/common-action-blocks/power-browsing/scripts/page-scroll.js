// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function waitBy(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}
async function scrollUntil(totalSeconds, waitSeconds, scrollDistance) {
  const totalDuration = totalSeconds * 1000;
  const waitDuration  = waitSeconds * 1000;
  const startTime = Date.now();
  for (
    let nextScrollTime = startTime + waitDuration;
    nextScrollTime < startTime + totalDuration;
    nextScrollTime += waitDuration
  ) {
    const remainingDuration = nextScrollTime - Date.now();
    await waitBy(remainingDuration);
    window.scrollBy(0, scrollDistance);
    scrollDistance = -scrollDistance;
  }
  const remainingDuration = startTime + totalDuration - Date.now();
  await waitBy(remainingDuration)
}
scrollUntil(PER_PAGE_DURATION, WAIT_DURATION, SCROLL_DISTANCE);