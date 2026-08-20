// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const policies = POLICY_JSON;

const sendMap = new Map();

function webUIResponse(id, isSuccess, response) {
  const entry = sendMap.get(id);
  if (entry === undefined) {
    return;
  }
  sendMap.delete(id);
  if (isSuccess) {
    entry.resolve(response);
  } else {
    entry.reject(response);
  }
}

// Note: this will break the existing UI.
Object.assign(window, { cr: { webUIResponse } });

function sendWithPromise(methodName, ...args) {
  return new Promise((resolve, reject) => {
    const id = `${methodName}_web-tests-cuj_${Date.now()}`;
    sendMap.set(id, {resolve, reject});
    chrome.send(methodName, [id, ...args]);
  });
}

await sendWithPromise('setUserAffiliation', true);

await sendWithPromise('setLocalTestPolicies', JSON.stringify(policies), '');
