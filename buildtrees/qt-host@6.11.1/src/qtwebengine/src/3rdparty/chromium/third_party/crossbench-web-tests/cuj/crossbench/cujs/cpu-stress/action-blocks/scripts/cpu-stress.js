// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
await (async () => {
  // We already loaded the first tab, this script is running inside it.
  for (let i = 1; i < $[NUM_TABS]; i++) {
    await new Promise(resolve => setTimeout(resolve, 1000));
    performance.mark(`load-tab-${i}`);
    window.open('./cpu-stress.html', '_blank', "noopener");
    performance.mark(`load-tab-${i}-done`);
  }
  performance.mark(`load-done`);
})();
