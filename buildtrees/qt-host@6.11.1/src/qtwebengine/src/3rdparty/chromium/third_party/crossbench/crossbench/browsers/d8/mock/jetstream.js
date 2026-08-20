// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

globalThis.document ??= {
    readyState: 'complete',
    querySelectorAll() {
      return [,];
    },
    getElementById(name) {
      return {
        classList: {
          contains(name) {
            return true;
          }
        }
      }
    },
};
globalThis.isInBrowser ??= false;
globalThis.readFile ??= read;
globalThis.isD8 ??= true;
globalThis.testList ??= [];
