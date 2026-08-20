// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (window.location.href === 'https://www.google.com/search?q=cats') {
  const headlineObserver = new MutationObserver(unused => {
    // Corresponds to google_search_phone.png in crbug.com/372457479#comment9.
    // The historical context is that this story used LCP on loadline v1, and so
    // for v2 we picked the corresponding LCP element on pixel 9.
    if (document.querySelector('img#dimg_1.YQ4gaf')) {
      performance.mark(
          'LoadLine2/google_search/result_added_to_dom');
      headlineObserver.disconnect();
    }
  });
  headlineObserver.observe(document, {childList: true, subtree: true});
}
