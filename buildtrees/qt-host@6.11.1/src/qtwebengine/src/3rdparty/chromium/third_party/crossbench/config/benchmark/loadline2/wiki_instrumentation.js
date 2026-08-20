// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (window.location.href === 'https://en.m.wikipedia.org/wiki/Taylor_Swift') {
  const language_button_selector = '.language-selector';
  const language_xpath = '//span[text()=\'Afrikaans\']';

  const button_observer = new MutationObserver(mutations => {
    const button = document.querySelector(language_button_selector);

    if (!button) {
      return;
    }
    button_observer.disconnect();
    button.click();
  });

  const language_observer = new MutationObserver(mutations => {
    const language = document
                         .evaluate(
                             language_xpath, document, null,
                             XPathResult.FIRST_ORDERED_NODE_TYPE, null)
                         .singleNodeValue;

    if (!language) {
      return;
    }
    performance.mark('LoadLine2/wikipedia_article/language_shown');
    language_observer.disconnect();
  });

  button_observer.observe(document, {childList: true, subtree: true});
  language_observer.observe(document, {childList: true, subtree: true});
}
