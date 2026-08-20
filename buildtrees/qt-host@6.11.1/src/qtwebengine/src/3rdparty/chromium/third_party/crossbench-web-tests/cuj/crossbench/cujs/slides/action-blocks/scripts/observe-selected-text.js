// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Before we click the text , set up an observer to log exactly
// observe the nearest  <g> tag that contains a <text> and has transform attribute. if the text is selected, a <rect> will be added within this <g> tag.
const oneLine = document.evaluate("//div[@id='pages']//*[local-name()='text' and text()= 'SELECTED_TEXT']/parent::*[local-name()='g' and @transform]/parent::*/parent::*[local-name()='g' and @transform]", document, null, XPathResult.FIRST_ORDERED_NODE_TYPE, null).singleNodeValue;
(new MutationObserver((mutationList, observer) => {
  //If a <rect> tag is added to the nearest <g> tag that contains a <text> tag and has a transform attribute.
  if (document.evaluate("//*[local-name()='rect']", oneLine, null, XPathResult.FIRST_ORDERED_NODE_TYPE, null)) {
    performance.mark("text-being-selected");
    observer.disconnect();
  } else {
    performance.mark("text-not-being-selected");
  }
})).observe(oneLine, { subtree: true, childList: true });
performance.mark('text-click');