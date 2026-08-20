// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (window.location.href === 'https://www.globo.com/') {
  const button_selector = 'button[aria-label=Consent]';
  const banner_selector = 'div[class=fc-consent-root]';
  let banner_observer;

  const button_observer = new MutationObserver(mutations => {
    const button = document.querySelector(button_selector);
    if (!button) {
      return;
    }
    button_observer.disconnect();
    const banner_node = document.querySelector(banner_selector);
    banner_observer = new MutationObserver(mutations => {
      for (const mutation of mutations) {
        for (const node of mutation.removedNodes) {
          if (node === banner_node) {
            performance.mark('LoadLine2/globo_homepage/cookie_banner_gone');
            banner_observer.disconnect();
            return;
          }
        }
      }
    });
    banner_observer.observe(banner_node.parentNode, {childList: true});
    button.click();
  });

  button_observer.observe(document, {childList: true, subtree: true});
}
