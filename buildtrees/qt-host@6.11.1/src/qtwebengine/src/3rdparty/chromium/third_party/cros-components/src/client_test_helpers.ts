/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @fileoverview Some test helpers for clients to use when building tests using
 * cros components. Need to work in both Chromium and Google3 so this file
 * has no external depencies aside from lit/mwc.
 */

import {LitElement} from 'lit';

/** Waits for a component and its children to finish rendering. */
export async function waitForComponentReady(component: LitElement) {
  await component.updateComplete;
  // Flush the microtask queue. This just ensures that if `component` has child
  // components, we give them all a chance to update too before continuing.
  await new Promise(r => setTimeout(r, 0));
}
