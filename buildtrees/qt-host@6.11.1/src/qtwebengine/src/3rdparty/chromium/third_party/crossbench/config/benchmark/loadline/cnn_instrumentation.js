// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const button_selector = 'button[id=onetrust-accept-btn-handler]';
const headline_text_id = 'maincontent';

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
  button.click();
});

button_observer.observe(document, {childList: true, subtree: true});

const headline_observer = new MutationObserver(mutations => {
  const headline = document.getElementById(headline_text_id);
  if (!headline) {
    return;
  }
  headline_observer.disconnect();
  performance.mark('maincontent.created');
});


if (window.location ==
    'https://edition.cnn.com/2024/04/21/china/china-spy-agency-public-profile-intl-hnk/index.html') {
  headline_observer.observe(document, {childList: true, subtree: true});
}
