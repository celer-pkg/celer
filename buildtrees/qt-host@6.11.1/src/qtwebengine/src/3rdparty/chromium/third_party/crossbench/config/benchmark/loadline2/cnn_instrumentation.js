// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (window.location.href ===
    'https://edition.cnn.com/2024/04/21/china/china-spy-agency-public-profile' +
        '-intl-hnk/index.html') {
  const button_id = 'headerMenuIcon';
  const menu_selector = '.header--active';
  let complete = false;

  const button_observer = new MutationObserver(mutations => {
    const button = document.getElementById(button_id);

    if (!button) {
      return;
    }

    button_observer.disconnect();
    click = function() {
      if (complete)
        return;
      button.click();
      setTimeout(click, 10);
    };
    click();
  });

  const menu_observer = new MutationObserver(mutations => {
    const menu = document.querySelector(menu_selector);

    if (!menu) {
      return;
    }
    performance.mark('LoadLine2/cnn_article/menu_shown');
    menu_observer.disconnect();
    complete = true;
  });

  // Make sure the cookie banner doesn't pop up
  document.cookie = 'OptanonAlertBoxClosed=' + new Date().toISOString()
  button_observer.observe(document, {childList: true, subtree: true});
  menu_observer.observe(document, {childList: true, subtree: true});
}
