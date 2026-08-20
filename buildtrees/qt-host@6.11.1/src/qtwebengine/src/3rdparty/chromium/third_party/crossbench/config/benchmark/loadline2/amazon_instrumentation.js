// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (window.location.href ===
    'https://www.amazon.co.uk/NIVEA-Suncream-Spray-Protect-Moisture/dp/B001B0OJXM') {
  const button_selector = 'input[id=sp-cc-accept]';

  const button_observer = new MutationObserver(mutations => {
    const button = document.querySelector(button_selector);
    if (!button) {
      return;
    }

    button_observer.disconnect();

    const attribute_observer = new MutationObserver(() => {
      if (button.hasAttribute('data-cel-widget')) {
        attribute_observer.disconnect();
        performance.mark('LoadLine2/amazon_product/button_click');
        button.click();
      }
    });
    attribute_observer.observe(button, {attributes: true});
  });

  button_observer.observe(document, {childList: true, subtree: true});
}
