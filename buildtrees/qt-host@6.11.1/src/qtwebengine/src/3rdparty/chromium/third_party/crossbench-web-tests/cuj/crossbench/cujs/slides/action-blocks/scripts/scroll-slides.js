// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const slideIndex = $[SLIDE_INSERT_POSITION];
let currentIndex = 1;
const iterateAndScrollIntoView = async () => {
  // This element gets all the superscript numbers of the slide thumbnails.
  const element = document.evaluate("//*[local-name()='g' and @class='punch-filmstrip-thumbnail']//*[local-name()='text' and @x and @y]", document);
  let lastTextNode = null;
  for (let node = element.iterateNext(); node !== null; node = element.iterateNext()) {
    lastTextNode = node;
    if (slideIndex === Number(node.textContent)) {
      node.scrollIntoView();
      return;
    }
  }
  lastTextNode.scrollIntoView();
  if (currentIndex === Number(lastTextNode.textContent)) {
    return;
  }
  currentIndex = Number(lastTextNode.textContent);
  await new Promise(resolve => setTimeout(resolve, 1000));
  await iterateAndScrollIntoView();
}

performance.mark("filmstrip-scroll-start");
await iterateAndScrollIntoView();
performance.mark("filmstrip-scroll-end");
performance.measure("filmstrip-scroll-duration", "filmstrip-scroll-start", "filmstrip-scroll-end");