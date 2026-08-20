/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/tabs/primary-tab.js';

import {css, CSSResultGroup, html, LitElement} from 'lit';

/** A chromeOS compliant tab component for use in cros-tabs. */
export class Tab extends LitElement {
  /** @nocollapse */
  static override shadowRootOptions: ShadowRootInit = {
    ...LitElement.shadowRootOptions,
    mode: 'open',
    delegatesFocus: true,
  };

  get mdTab(): HTMLElement|null {
    return this.shadowRoot!.querySelector('md-primary-tab');
  }

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
    }

    #slotted-content {
      display: inline-flex;
      align-items: center;
    }

    :host([badged]) #slotted-content::after {
      display: inline-block;
      content: '';
      width: 6px;
      height: 6px;
      border-radius: 50%;
      background-color: var(--cros-tab-badge-color, var(--cros-sys-error));
      margin-inline-start: 4px;
    }

    md-primary-tab {
      padding: 0 8px;

      /** Base styles */
      --md-primary-tab-active-indicator-color: var(--cros-sys-primary);
      --md-primary-tab-active-indicator-height: 4px;
      --md-primary-tab-active-label-text-color: var(--cros-sys-primary);
      --md-primary-tab-container-color: transparent;
      --md-primary-tab-container-shape: 8px;
      --md-primary-tab-label-text-color: var(--cros-sys-on_surface_variant);
      --md-primary-tab-label-text-font: var(--cros-button-1-font-family);

      /** Hovered */
      --md-primary-tab-active-hover-label-text-color: var(--cros-sys-primary);
      --md-primary-tab-active-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-primary-tab-active-hover-state-layer-opacity: 1;
      --md-primary-tab-hover-label-text-color: var(--cros-sys-on_surface_variant);
      --md-primary-tab-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-primary-tab-hover-state-layer-opacity: 1;

      /** Focused */
      --md-primary-tab-active-focus-label-text-color: var(--cros-sys-primary);
      --md-primary-tab-active-focus-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-primary-tab-active-focus-state-layer-opacity: 1;
      --md-primary-tab-focus-label-text-color: var(--cros-sys-on_surface_variant);
      --md-primary-tab-focus-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-primary-tab-focus-state-layer-opacity: 1;

      /** Pressed */
      --md-primary-tab-active-pressed-label-text-color: var(--cros-sys-primary);
      --md-primary-tab-active-pressed-state-layer-color: var(--cros-sys-ripple_primary);
      --md-primary-tab-active-pressed-state-layer-opacity: 1;
      --md-primary-tab-pressed-label-text-color: var(--cros-sys-primary);
      --md-primary-tab-pressed-state-layer-color: var(--cros-sys-ripple_primary);
      --md-primary-tab-pressed-state-layer-opacity: 1;
    }

    md-primary-tab::part(focus-ring) {
      --md-focus-ring-active-width: 2px;
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-duration: 0s;
      --md-focus-ring-shape: 8px;
      --md-focus-ring-width: 2px;
      margin-bottom: 0;
    }

    ::slotted(*) {
      margin: 0 4px;
    }
  `;

  /** @nocollapse */
  static override properties = {
    active: {type: Boolean, reflect: true},
    badged: {type: Boolean, reflect: true},
  };

  declare active: boolean;
  declare badged: boolean;

  constructor() {
    super();
    this.active = false;
    this.badged = false;
  }

  override firstUpdated() {
    this.setAttribute('md-tab', 'tab');

    // Add extra styles if the tabs should fill the width of a container.
    if (this.parentElement!.hasAttribute('fill-container')) {
      if (this.mdTab) {
        this.mdTab.style.width = '100%';
        this.mdTab.style.boxSizing = 'border-box';
      }
    }
  }

  override render() {
    return html`
      <md-primary-tab ?active=${this.active}>
        <span id="slotted-content"><slot></slot></span>
      </md-primary-tab>
    `;
  }
}

customElements.define('cros-tab', Tab);

declare global {
  interface HTMLElementTagNameMap {
    'cros-tab': Tab;
  }
}
