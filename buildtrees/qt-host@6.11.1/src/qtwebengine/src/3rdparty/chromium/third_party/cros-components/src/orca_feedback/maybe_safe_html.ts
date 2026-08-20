/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {unsafeHTML} from 'lit/directives/unsafe-html';

/**
 * Util method to hide HTML sanitization logic so we can support both Open
 * Source Lit and Google3 Lit.
 */
export function maybeSafeHTML(html: string) {
  // TODO: b/338151548 - Add sanitizer here.
  return unsafeHTML(html);
}
