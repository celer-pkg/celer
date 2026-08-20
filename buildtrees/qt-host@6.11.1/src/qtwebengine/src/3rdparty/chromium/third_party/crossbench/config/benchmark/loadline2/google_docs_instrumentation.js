// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (window.location.href ===
    'https://docs.google.com/document/d/13AWeOGqtSkfpPK7meqE_X-GQQggwx4JJ1vc0' +
        'YGvKg34/edit#heading=h.gjdgxs') {
  const headlineObserver = new MutationObserver(unused => {
    // Corresponds to docs.png in crbug.com/372457479#comment9. The historical
    // context is that this story used LCP on loadline v1, and so for v2 we
    // picked the corresponding LCP element on pixel 9.
    if (document.querySelector('div.navigation-widget-empty-content')) {
      performance.mark('LoadLine2/google_docs/navigation_widget_added_to_dom')
      headlineObserver.disconnect();
    }
  });
  headlineObserver.observe(document, {childList: true, subtree: true});
}
