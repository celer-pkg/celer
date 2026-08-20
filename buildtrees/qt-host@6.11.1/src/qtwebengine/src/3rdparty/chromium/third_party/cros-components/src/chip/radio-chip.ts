/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/ripple/ripple.js';
import '@material/web/focus/md-focus-ring.js';

import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';

import {FOCUS_RING_WIDTH, ICON_SIZE} from './chip';

/**
 * Start/End paddings for the radio chip.
 * Should be:
 * <12px>label<12px>
 * <8px>icon<8px>label<12px>
 * We achieve this by layering container, label and icon paddings as such:
 * <4px><8px>label<8px><4px>
 * <4px><4px>icon<0px><8px>label<8px><4px>
 *
 */
const RADIO_CHIP_PADDING = {
  containerStart: css`4px`,
  containerEnd: css`4px`,
  labelStart: css`8px`,
  labelEnd: css`8px`,
  leadingIconStart: css`4px`,
};
const CHIP_HEIGHT = css`32px`;

/**
 * A cros-compliant radio chip component.
 * Note this is separate from the cros-chip component to due specific a11y
 * requirements of cros-radio-chip that require a different implementation.
 */
export class RadioChip extends LitElement {
  /** @export */
  declare disabled: boolean;
  /** @export */
  declare selected: boolean;
  /** @export */
  declare label: string;

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
      :host {
        --md-ripple-hover-color: var(--cros-sys-hover_on_subtle);
        --md-ripple-hover-opacity: 1;
        --md-ripple-pressed-color: var(--cros-sys-ripple_neutral_on_subtle);
        --md-ripple-pressed-opacity: 1;
        align-items: center;
        border-radius: 9999px;
        border: 1px solid var(--cros-sys-separator);
        color: var(--cros-sys-on_surface);
        cursor: pointer;
        display: flex;
        font: var(--cros-button-1-font);
        height: ${CHIP_HEIGHT};
        padding-inline-end: ${RADIO_CHIP_PADDING.containerEnd};
        padding-inline-start: ${RADIO_CHIP_PADDING.containerStart};
        position: relative;
        user-select: none;
        width: fit-content;
      }

      :host([disabled]) {
        opacity: .32;
        pointer-events: none;
      }

      :host([selected]) {
        --md-ripple-hover-color: var(--cros-sys-hover_on_prominent);
        --md-ripple-pressed-color: var(--cros-sys-ripple_primary);
        background: var(--cros-sys-primary);
        color: var(--cros-sys-on_primary);
      }

      :host([disabled][selected]) {
        background: var(--cros-sys-disabled_container);
        color: var(--cros-sys-disabled);
        opacity: 1;
      }

      :host(:focus) {
        outline: none;
      }

      md-focus-ring {
        --md-focus-ring-color: var(--cros-sys-focus_ring);
        --md-focus-ring-duration: 0s;
        --md-focus-ring-width: ${FOCUS_RING_WIDTH};
      }

      span {
        display: inline-box;
        padding-inline-end: ${RADIO_CHIP_PADDING.labelEnd};
        padding-inline-start: ${RADIO_CHIP_PADDING.labelStart};
      }

      ::slotted([slot="icon"]) {
        height: ${ICON_SIZE};
        padding-inline-start: ${RADIO_CHIP_PADDING.leadingIconStart};
        width: ${ICON_SIZE};
      }
  `;

  /** @nocollapse */
  static override properties = {
    disabled: {type: Boolean, reflect: true},
    selected: {type: Boolean, reflect: true},
    label: {type: String},
  };

  private readonly onKeydown = (e: KeyboardEvent) => void this.handleKeydown(e);

  constructor() {
    super();
    this.disabled = false;
    this.selected = false;
    this.label = '';
  }

  override connectedCallback() {
    super.connectedCallback();

    this.setAttribute('role', 'radio');
    this.setAttribute('tabindex', '0');
    this.setAttribute('aria-checked', String(this.selected));
    this.addEventListener('keydown', this.onKeydown);
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    this.removeEventListener('keydown', this.onKeydown);
  }

  override firstUpdated() {
    this.renderRoot.querySelector('md-focus-ring')!.attach(this);
    this.renderRoot.querySelector('md-ripple')!.attach(this);
  }

  override updated(changedProperties: PropertyValues<RadioChip>) {
    if (changedProperties.has('selected')) {
      this.setAttribute('aria-checked', String(this.selected));
    }
  }

  override render() {
    return html`
      <slot slot='icon' name='icon'></slot>
      <span>${this.label}</span>
      <md-focus-ring></md-focus-ring>
      <md-ripple></md-ripple>
      `;
  }

  private handleKeydown(e: KeyboardEvent) {
    if (e.key !== ' ' && e.key !== 'Enter') {
      return;
    }

    // This will cause handleClick to run and update the state if needed.
    this.dispatchEvent(
        new MouseEvent('click', {composed: true, bubbles: true}));
  }
}

customElements.define('cros-radio-chip', RadioChip);

declare global {
  interface HTMLElementTagNameMap {
    'cros-radio-chip': RadioChip;
  }
}
