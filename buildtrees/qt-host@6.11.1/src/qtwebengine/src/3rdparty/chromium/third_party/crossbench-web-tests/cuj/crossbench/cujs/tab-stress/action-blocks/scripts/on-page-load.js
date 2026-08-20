// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const navigationEntries = performance.getEntriesByType('navigation');
if (navigationEntries.length !== 1) {
  throw new Error(`expected 1 'navgitation' performance entry, got ${navigationEntries.length}`);
}
const navigation = navigationEntries[0];

function pageLoaded() {
  performance.mark('page-loaded', {
    // Put the page-loaded event at the correct time, even if we're logging
    // later
    startTime: navigation.domComplete,
    detail: {
      url: 'PAGE_URL',
      // put the complete and interactive times into the event to make it easier
      // to query the values. Perfetto will map the startTime above to the
      // global trace timeline.
      domComplete: navigation.domComplete,
      domInteractive: navigation.domInteractive,
    },
  });
}

// Wait until 500 ms after navigation start to make sure that performance events
// aren't dropped.
const markDelay = Math.max(0, 500 - performance.now());
setTimeout(() => {
  // Back date the page load event to have happened when the page load started.
  performance.mark('page-load', { startTime: 0, detail: { url: 'PAGE_URL' }});

  if (document.readyState === 'complete') {
    pageLoaded();
  } else {
    // Asynchronously log the page-loaded event.
    let complete = false;
    document.addEventListener('readystatechange', () => {
      if (!complete && document.readyState === 'complete') {
        complete = true;
        pageLoaded();
      }
    });
  }
}, markDelay);
