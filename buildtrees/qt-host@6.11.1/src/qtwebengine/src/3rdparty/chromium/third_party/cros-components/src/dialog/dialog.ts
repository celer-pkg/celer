/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/dialog/dialog.js';

import {css, CSSResultGroup, html, LitElement} from 'lit';

/**
 * A ChromeOS compliant dialog.
 */
export class Dialog extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
    }
    md-dialog {
      --md-dialog-container-color: var(--cros-sys-dialog_container);
      --md-dialog-container-shape: 20px;
      --md-dialog-headline-color: var(--cros-sys-on_surface);
      --md-dialog-headline-font: var(--cros-display-7-font);
      --md-dialog-supporting-text-color: var(--cros-sys-on_surface_variant);
      --md-dialog-supporting-text-font: var(--cros-body-1-font);
      }
      :host([type="list"]) slot[name="headline"] {
        background-color: var(--cros-sys-surface1);
      }
      ::slotted(:is([slot="headline"])) {
        padding: 32px 32px 16px 32px;
      }
      ::slotted(:is([slot="content"])) {
        padding: 0px 32px 32px 32px;
      }
      ::slotted(:is([slot="actions"])) {
        padding: 0px 32px 28px 32px;
      }
      :host([type="list"]) ::slotted(:is[slot="headline"]) {
        padding: 32px 32px 20px 32px;
      }
      :host([type="illustration"]) {
        --cros-dialog-headline-alignment: center;
        --cros-dialog-content-alignment: center;
        --cros-dialog-actions-alignment: center;
      }
      slot {
        display: flex;
      }
      slot[name="headline"] {
        justify-content: var(--cros-dialog-headline-alignment, left);
      }
      slot[name="content"] {
        justify-content: var(--cros-dialog-content-alignment, left);
      }
      slot[name="actions"] {
        justify-content: var(--cros-dialog-actions-alignment, right);
      }
  `;

  /** @nocollapse */
  static override properties = {
    type: {type: String, reflect: true},
  };

  /** @export */
  declare type: 'basic'|'illustration'|'list'|'pagination';

  constructor() {
    super();

    this.type = 'basic';
  }

  override render() {
    return html`
      <md-dialog>
        <slot slot="headline" name="headline"></slot>
        <slot slot="content" name="content"></slot>
        <slot slot="actions" name="actions" class="footer"></slot>
      </md-dialog>
    `;
  }

  show() {
    this.shadowRoot!.querySelector('md-dialog')!.show();
  }

  close() {
    this.shadowRoot!.querySelector('md-dialog')!.close();
  }
}

customElements.define('cros-dialog', Dialog);

declare global {
  interface HTMLElementTagNameMap {
    'cros-dialog': Dialog;
  }
}
