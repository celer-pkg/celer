/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/button/text-button.js';

import {css, CSSResultGroup, html, LitElement} from 'lit';

/**
 * A ChromeOS compliant breadcrumb component.
 */
export class Breadcrumb extends LitElement {
  /** @nocollapse */
  static override properties = {
    size: {type: String},
  };
  /**
   * @export
   */
  declare size: 'default'|'small';
  constructor() {
    super();
    this.size = 'default';
  }
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: flex;
      align-items: center;
    }
    :host([size="small"]) ::slotted(*) {
      --breadcrumb-size: 32px;
      --icon-size: 32px;
      --chevron-size: 16px;
      --font-family: var(--cros-button-1-font-family);
      --font-size: var(--cros-button-1-font-size);
      --line-height: var(--cros-button-1-line-height);
      --font-weight: var(--cros-button-1-font-weight);
      --padding-between: 0px;
    }
  `;

  override render() {
    return html`
      <slot></slot>
    `;
  }
}

customElements.define('cros-breadcrumb', Breadcrumb);

declare global {
  interface HTMLElementTagNameMap {
    'cros-breadcrumb': Breadcrumb;
  }
}
