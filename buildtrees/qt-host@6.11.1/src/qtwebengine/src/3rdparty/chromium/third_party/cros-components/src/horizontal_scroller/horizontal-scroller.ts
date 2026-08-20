/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {css, CSSResultGroup, html, LitElement} from 'lit';

/** A chromeOS compliant horizontal-scroller. */
export class HorizontalScroller extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
      font: var(--cros-body-0-font);
    }
  `;

  /** @export */
  declare label: string;

  /** @nocollapse */
  static override properties = {
    label: {type: String, reflect: true},
  };

  constructor() {
    super();
    this.label = 'I am a horizontal scroller';
  }

  override render() {
    return html`
      <div>
        Hello, ${this.label}!
      </div>
    `;
  }
}

customElements.define('horizontal-scroller', HorizontalScroller);

declare global {
  interface HTMLElementTagNameMap {
    'horizontal-scroller': HorizontalScroller;
  }
}
