// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(() => {
  const videoPaperDialogParentSelector = "ytd-popup-container";
  const videoPaperDialogSelector = "tp-yt-paper-dialog";
  const hiddenDialog = () => {
    const targetNode = document.querySelector(videoPaperDialogSelector);
    if (targetNode && targetNode.style.display !== 'none') {
      performance.mark("hide-dialog")
      targetNode.style.display = 'none';
    }
  };

  const elementObserver = new MutationObserver((mutationsList) => {
    for (const mutation of mutationsList) {
      if (mutation.type === 'childList') {
        for (const node of mutation.addedNodes) {
          if (node.tagName === videoPaperDialogSelector) {
            hiddenDialog();
            return;
          }
        }
      }
    }
  })
  const parentElement = document.querySelector(videoPaperDialogParentSelector);
  elementObserver.observe(parentElement, { childList: true, subtree: true });
  // Observer will be executed only when the element changes.
  // There may already be a popup dialog after the page is completed.
  hiddenDialog();
})();