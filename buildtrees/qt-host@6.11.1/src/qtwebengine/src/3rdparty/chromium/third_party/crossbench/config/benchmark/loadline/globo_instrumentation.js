// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const button_selector = 'button[aria-label=Consent]';
const banner_selector = 'div[class=fc-consent-root]';
let banner_observer;

const button_observer = new MutationObserver(mutations => {
  const button = document.querySelector(button_selector);
  if (!button) {
    return;
  }
  // This script can run multiple times.
  if (localStorage.getItem('already_run') === 'already_run') {
    return;
  }
  localStorage.setItem('already_run', 'already_run');
  button_observer.disconnect();
  const banner_node = document.querySelector(banner_selector);
  banner_observer = new MutationObserver(function(e) {
    e.forEach(function(m) {
      m.removedNodes.forEach(function(n) {
        if (n === banner_node) {
          performance.mark('cookie_banner_gone');
          banner_observer.disconnect();
        }
      });
    });
  });
  banner_observer.observe(banner_node.parentNode, {childList: true});
  button.click();
})

button_observer.observe(document, {childList: true, subtree: true});
