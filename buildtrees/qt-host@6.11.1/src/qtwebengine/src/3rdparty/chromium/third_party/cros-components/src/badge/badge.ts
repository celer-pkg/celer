/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {css, CSSResultGroup, html, LitElement} from 'lit';

/** A chromeOS compliant badge. */
export class Badge extends LitElement {
  // Total height of the badge is 18px, since line height is 10px, and there is
  // 4px of padding on top and bottom.
  static MARGIN = 8;
  static PADDING_BLOCK = 4;
  static PADDING_INLINE = 8;

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      background: var(--cros-sys-primary);
      border-radius: 100px;
      color: var(--cros-sys-on_primary);
      display: inline-block;
      font: var(--cros-label-1-font);
      height: fit-content;
      margin: 0 ${Badge.MARGIN}px;
      padding: ${Badge.PADDING_BLOCK}px ${Badge.PADDING_INLINE}px;
      width: fit-content;
    }
  `;

  override render() {
    return html`
      <slot></slot>
    `;
  }
}

customElements.define('cros-badge', Badge);

declare global {
  interface HTMLElementTagNameMap {
    'cros-badge': Badge;
  }
}
