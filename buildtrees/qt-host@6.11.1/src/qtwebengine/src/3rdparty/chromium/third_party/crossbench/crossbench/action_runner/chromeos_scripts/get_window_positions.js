// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// <INSERT SELECTOR LOGIC HERE>

if (arguments[0] && element && arguments[1]) {
  element.scrollIntoView();
}

if (arguments[0] && element) {
  element_rect = element.getBoundingClientRect();
} else {
  element_rect = new DOMRect();
}

return [
  arguments[0] && element, window.devicePixelRatio, window.outerWidth,
  window.innerWidth, window.innerHeight, screen.width, screen.height,
  screen.availWidth, screen.availHeight, screenX, screenY, element_rect.left,
  element_rect.top, element_rect.width, element_rect.height
];
