/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/tabs/tabs.js';

import {css, CSSResultGroup, html, LitElement} from 'lit';

/** The default gap value between each tab in a tab group. */
export const TABS_GAP = '8px';

/**
 * A ChromeOS compliant tabs component.
 */
export class Tabs extends LitElement {
  get mdTabs(): HTMLElement|null {
    return this.shadowRoot!.querySelector('md-tabs');
  }

  get tabSlot(): HTMLSlotElement {
    return this.shadowRoot!.querySelector('slot') as HTMLSlotElement;
  }

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
    }

    :host-context([fill-container]) md-tabs {
      min-inline-size: var(--container-size, auto);
    }

    :host-context([fill-container]) slot {
      gap: var(--container-tabs-gap, 0);
    }

    md-tabs {
      --md-divider-thickness: 0;
    }

    slot {
      display: flex;
      gap: var(--cros-tabs-gap, 8px);
      width: 100%;
    }
  `;

  /**
   * The index of the active tab.
   * @export
   */
  declare activeTabIndex: number;

  /** @nocollapse */
  static override properties = {
    activeTabIndex: {type: Number, reflect: true},
  };

  constructor() {
    super();
    this.activeTabIndex = 0;
  }

  override render() {
    return html`
      <md-tabs
        .activeTabIndex=${this.activeTabIndex}>
        <slot></slot>
      </md-tabs>
    `;
  }
}

customElements.define('cros-tabs', Tabs);

declare global {
  interface HTMLElementTagNameMap {
    'cros-tabs': Tabs;
  }
}
