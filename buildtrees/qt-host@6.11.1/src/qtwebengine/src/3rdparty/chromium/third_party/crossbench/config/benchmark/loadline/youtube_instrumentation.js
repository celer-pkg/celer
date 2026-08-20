// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const button_selector =
    'div.body.style-scope.ytd-consent-bump-v2-lightbox > div.eom-buttons.style-scope.ytd-consent-bump-v2-lightbox > div:nth-child(1) > ytd-button-renderer:nth-child(1) > yt-button-shape > button';
const banner_selector =
    'ytd-consent-bump-v2-lightbox > tp-yt-paper-dialog[id=dialog]';

const button_observer = new MutationObserver(mutations => {
  const button = document.querySelector(button_selector);
  if (!button) {
    return;
  }
  const banner_node = document.querySelector(banner_selector);
  if (!banner_node) {
    return;
  }
  if (localStorage.getItem('already_run') === 'already_run') {
    return;
  }
  localStorage.setItem('already_run', 'already_run');
  button_observer.disconnect();
  const banner_observer = new MutationObserver(function(e) {
    for (m of e) {
      if (m.type == 'attributes' && banner_node.style.display == 'none') {
        banner_observer.disconnect();
        performance.mark('cookie_banner_gone');
        break;
      }
    }
  });
  banner_observer.observe(
      banner_node, {attributes: true, attributeFilter: ['style']});
  button.click();
});

button_observer.observe(document, {childList: true, subtree: true});
