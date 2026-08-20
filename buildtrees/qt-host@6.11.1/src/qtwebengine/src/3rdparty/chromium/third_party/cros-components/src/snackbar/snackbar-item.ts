/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {css, CSSResultGroup, html, LitElement} from 'lit';

// TODO: b/301335548 - Remove these once we have elevation tokens.
const AMBIENT_SHADOW_COLOR = css`rgba(var(--cros-sys-shadow-rgb), 0.2)`;
const KEY_SHADOW_COLOR = css`rgba(var(--cros-sys-shadow-rgb), 0.1)`;

/** The visible rendered part of a cros-snackbar. */
export class SnackbarItem extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      background-color: var(--cros-sys-inverse_surface);
      border-radius: 12px;
      box-shadow:
        0px 4px 4px 0px ${AMBIENT_SHADOW_COLOR},
        0px 0px 4px 0px ${KEY_SHADOW_COLOR};
      display: inline-block;
      font: var(--cros-body-2-font);
    }

    #container {
      align-items: center;
      box-sizing: border-box;
      column-gap: 12px;
      display: flex;
      justify-content: space-between;
      padding-bottom: 16px;
      padding-inline-end: 40px;
      padding-inline-start: 20px;
      padding-top: 16px;
      width: 336px;
    }

    :host(.has-action-button) #container {
      padding-bottom: 8px;
      padding-inline-end: 20px;
      padding-top: 8px;
    }

    #message-action-container {
      align-items: center;
      column-gap: 8px;
      display: flex;
      flex-grow: 2;
      justify-content: space-between;
    }

    #message {
      color: var(--cros-sys-surface);
    }

    #action-container {
      display: flex;
    }

    /** TODO: b/308699858 - Change this to inverted dark/light mode variant. */
    ::slotted(cros-placeholder-icon) {
      --icon-color: var(--cros-sys-error);
      min-width: 20px;
    }

    ::slotted(svg) {
      fill: var(--cros-sys-error);
    }
  `;

  /**
   * The visible message on the toast surface.
   * TODO b/308729483 - Handle message length and text wrapping.
   * @export
   */
  declare message: string;

  /** @nocollapse */
  static override properties = {
    message: {type: String, reflect: true},
  };

  constructor() {
    super();
    this.message = '';
  }

  override render() {
    return html`
      <div id="container">
        <slot name="icon"></slot>
        <div id="message-action-container">
          <div id="message">${this.message}</div>
          <div id="action-container">
            <slot name="action"></slot>
            <slot name="dismiss"></slot>
          </div>
        </div>
      </div>
    `;
  }
}

customElements.define('cros-snackbar-item', SnackbarItem);

declare global {
  interface HTMLElementTagNameMap {
    'cros-snackbar-item': SnackbarItem;
  }
}
