/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/menu/menu.js';
import '@material/web/menu/sub-menu.js';
import '@material/web/menu/menu-item.js';
import './menu_item';

import {css, CSSResultGroup, html, LitElement} from 'lit';

const chevronIcon =
    html`<svg viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg" slot="end" id="icon">
      <path fill-rule="evenodd" clip-rule="evenodd" d="M6.66669 13.825L10.7872 10L6.66669 6.175L7.93524 5L13.3334 10L7.93524 15L6.66669 13.825Z"/>
    </svg>`;
/**
 * ChromeOS menu item which represents an openable submenu.
 * Hovering or clicking element will open a second submenu.
 */
export class SubMenuItem extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    #icon {
      width: 20px;
      height: 20px;
      fill: var(--cros-sys-on_surface);
    }
    :host-context([dir=rtl]) #icon {
      transform: scaleX(-1);
    }
  `;

  /** @nocollapse */
  static override shadowRootOptions = {
    mode: 'open' as const,
    delegatesFocus: true,
  };

  /** @nocollapse */
  static override properties = {
    headline: {type: String},
    // Use a lit property to handle reflecting with preset attributes.
    tabIndex: {type: Number, reflect: true},
    disabled: {type: Boolean},
  };

  /**
   * Headline is the primary text of the list item, name follows
   * from md-menu-item.
   */
  declare headline: string;

  // The following properties are necessary for wrapping and keyboard navigation
  /**
   * Whether or not the menu-item is disabled.
   */
  declare disabled: boolean;

  /**
   * Sets the item in the selected visual state when a submenu is opened.
   */
  get selected() {
    // NOTE: md-sub-menu changes .selected inside its implementation, so
    // the easiest way to keep this.selected in sync with md-sub-menu.selected
    // is just to create accessors.
    return this.renderRoot.querySelector('cros-menu-item')?.selected ?? false;
  }

  set selected(value: boolean) {
    const item = this.renderRoot.querySelector('cros-menu-item');
    if (item) {
      item.selected = value;
    }
  }

  get item() {
    // This is required to expose internal or slotted items to menu keyboard nav
    return this.renderRoot?.querySelector?.('cros-menu-item');
  }

  constructor() {
    super();

    this.tabIndex = 0;
    this.headline = '';
    this.disabled = false;
  }

  override render() {
    return html`
      <md-sub-menu>
        <cros-menu-item
            slot='item'
            .disabled=${this.disabled}
            .headline=${this.headline}>
          ${chevronIcon}
        </cros-menu-item>
        <slot name='submenu' slot='menu'></slot>
      </md-sub-menu>`;
  }

  /**
   * Shows the submenu.
   */
  show() {
    if (this.renderRoot.querySelector('md-sub-menu') === null) {
      throw new Error(
          'Called show before menu has rendered, maybe try await updateComplete.');
    }
    this.renderRoot.querySelector('md-sub-menu')!.show();
  }

  /**
   * Closes the submenu.
   */
  close() {
    // This close method is required for properly wrapping md-sub-menu
    // because md-menu uses it to close nested submenus.
    if (this.renderRoot.querySelector('md-sub-menu') === null) {
      throw new Error(
          'Called show before menu has rendered, maybe try await updateComplete.');
    }
    this.renderRoot.querySelector('md-sub-menu')!.close();
  }

  // This is just in case `delegatesFocus` doesn't do what is intended (which is
  // unfortunately often across browser updates).
  override focus() {
    this.renderRoot.querySelector('cros-menu-item')?.focus();
  }
}

customElements.define('cros-sub-menu-item', SubMenuItem);

declare global {
  interface HTMLElementTagNameMap {
    'cros-sub-menu-item': SubMenuItem;
  }
}
