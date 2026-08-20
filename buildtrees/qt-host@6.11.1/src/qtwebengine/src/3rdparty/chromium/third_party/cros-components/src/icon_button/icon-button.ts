/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/iconbutton/filled-icon-button.js';
import '@material/web/iconbutton/filled-tonal-icon-button.js';
import '@material/web/iconbutton/icon-button.js';

import {css, CSSResultGroup, html, LitElement} from 'lit';
import {ifDefined} from 'lit/directives/if-defined';

/**
 * Icon buttons have different variants depending on the container
 * background, designed to improve contrast depending on the color of the
 * surface behind it:
 * 1) base (app-base)
 * 2) subtle
 * 3) prominent
 * 4) image (gradient)
 * Filled only has base & image. Floating has all surfaces and toggle has
 * base, subtle and prominent.
 */
const FILLED_ICON_ON_BASE =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_surface))`;
const FILLED_ICON_ON_IMAGE =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_surface))`;

const FILLED_CONTAINER_ON_BASE = css`var(--cros-sys-surface_variant)`;
const FILLED_CONTAINER_ON_IMAGE = css`var(--cros-sys-surface3)`;

const FLOATING_ICON_ON_BASE =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_surface))`;
const FLOATING_ICON_ON_SUBTLE =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_primary_container))`;
const FLOATING_ICON_ON_PROMINENT =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_primary))`;
const FLOATING_ICON_ON_IMAGE =
    css`var(--cros-icon-button-color-override, white)`;

const TOGGLE_ICON_ON_BASE =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_surface))`;
const TOGGLE_ICON_ON_SUBTLE =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_primary_container))`;
const TOGGLE_ICON_ON_PROMINENT =
    css`var(--cros-icon-button-color-override, var(--cros-sys-on_primary))`;

const TOGGLE_SELECTED_ICON_ON_BASE =
    css`var(--cros-icon-button-selected-color-override, var(--cros-sys-on_primary_container))`;
const TOGGLE_SELECTED_ICON_ON_SUBTLE =
    css`var(--cros-icon-button-selected-color-override, var(--cros-sys-on_primary_container))`;
const TOGGLE_SELECTED_ICON_ON_PROMINENT =
    css`var(--cros-icon-button-selected-color-override, var(--cros-sys-on_primary))`;

const TOGGLE_CONTAINER_ON_BASE = css`var(--cros-sys-primary_container)`;
const TOGGLE_CONTAINER_ON_SUBTLE = css`var(--cros-sys-highlight_shape)`;
const TOGGLE_CONTAINER_ON_PROMINENT = css`var(--cros-sys-highlight_shape)`;

const LARGE_CONTAINER_WIDTH = css`72px`;
const LARGE_CONTAINER_HEIGHT = css`56px`;

/**
 * A ChromeOS compliant icon-button component.
 */
export class IconButton extends LitElement {
  /** @export */
  declare buttonStyle: 'filled'|'floating'|'toggle';
  /** @export */
  declare size: 'small'|'default'|'large';
  /** @export */
  declare shape: 'circle'|'square';
  /**
   * @export
   * The background the icon button sits on. This will change the background and
   * icon color of the icon-button.
   */
  declare surface: 'base'|'prominent'|'subtle'|'image';
  /** @export */
  declare disabled: boolean;
  /** @export */
  declare selected: boolean;
  /** @export */
  declare href: string;

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
    }
    md-filled-tonal-icon-button {
      --md-filled-tonal-icon-button-container-color: ${
      FILLED_CONTAINER_ON_BASE};
      --md-filled-tonal-icon-button-container-shape: 12px;
      --md-filled-tonal-icon-button-container-width: 40px;
      --md-filled-tonal-icon-button-container-height: 40px;
      --md-filled-tonal-icon-button-disabled-container-color: var(--cros-sys-disabled_container);
      --md-filled-tonal-icon-button-disabled-container-opacity: 1;
      --md-filled-tonal-icon-button-disabled-icon-color: var(--cros-sys-disabled);
      --md-filled-tonal-icon-button-disabled-icon-opacity: 1;
      --md-filled-tonal-icon-button-focus-icon-color: ${FILLED_ICON_ON_BASE};
      --md-filled-tonal-icon-button-hover-icon-color: ${FILLED_ICON_ON_BASE};
      --md-filled-tonal-icon-button-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-filled-tonal-icon-button-hover-state-layer-opacity: 1;
      --md-filled-tonal-icon-button-icon-color: ${FILLED_ICON_ON_BASE};
      --md-filled-tonal-icon-button-icon-size: var(--cros-icon-button-icon-size, 20px);
      --md-filled-tonal-icon-button-pressed-icon-color: ${FILLED_ICON_ON_BASE};
      --md-filled-tonal-icon-button-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-filled-tonal-icon-button-pressed-state-layer-opacity: 1;
      --md-filled-tonal-icon-button-selected-focus-icon-color: ${
      FILLED_ICON_ON_BASE};
      --md-filled-tonal-icon-button-selected-pressed-icon-color: ${
      FILLED_ICON_ON_BASE};
      --md-focus-ring-duration: 0s;
    }
    :host([size="small"]) md-filled-tonal-icon-button {
      --md-filled-tonal-icon-button-container-shape: 10px;
      --md-filled-tonal-icon-button-container-width: 32px;
      --md-filled-tonal-icon-button-container-height: 32px;
      --md-filled-tonal-icon-button-icon-size: var(--cros-icon-button-icon-size, 20px);
    }
    :host([size="large"]) md-filled-tonal-icon-button {
      --md-filled-tonal-icon-button-container-shape: 16px;
      --md-filled-tonal-icon-button-container-height: ${LARGE_CONTAINER_HEIGHT};
      --md-filled-tonal-icon-button-container-width: ${LARGE_CONTAINER_WIDTH};
      --md-filled-tonal-icon-button-icon-size: var(--cros-icon-button-icon-size, 24px);
      margin: 0;
    }
    :host([surface="image"][buttonStyle="filled"]) md-filled-tonal-icon-button {
      --md-filled-tonal-icon-button-container-color: ${
      FILLED_CONTAINER_ON_IMAGE};
      --md-filled-tonal-icon-button-focus-icon-color: ${FILLED_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-hover-icon-color: ${FILLED_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-icon-color: ${FILLED_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-pressed-icon-color: ${FILLED_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-selected-focus-icon-color: ${
      FILLED_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-selected-pressed-icon-color: ${
      FILLED_ICON_ON_IMAGE};
    }
    :host([surface="image"][buttonStyle="floating"]) md-filled-tonal-icon-button {
      --md-filled-tonal-icon-button-container-color: rgba(var(--cros-sys-black-rgb), 0.6);
      --md-filled-tonal-icon-button-focus-icon-color: ${FLOATING_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-hover-icon-color: ${FLOATING_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-icon-color: ${FLOATING_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-pressed-icon-color: ${
      FLOATING_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-selected-focus-icon-color: ${
      FLOATING_ICON_ON_IMAGE};
      --md-filled-tonal-icon-button-selected-pressed-icon-color: ${
      FLOATING_ICON_ON_IMAGE};
      }
    :host([shape="circle"]) md-filled-tonal-icon-button {
      --md-filled-tonal-icon-button-container-shape: 100vmax;
    }
    md-icon-button {
      --md-focus-ring-duration: 0s;
      --md-icon-button-disabled-icon-color: ${FLOATING_ICON_ON_BASE};
      --md-icon-button-disabled-icon-opacity: var(--cros-sys-disabled-opacity);
      --md-icon-button-icon-size: var(--cros-icon-button-icon-size, 20px);
      --md-icon-button-state-layer-shape: 12px;
      --md-icon-button-state-layer-width: 40px;
      --md-icon-button-state-layer-height: 40px;
      --md-icon-button-focus-icon-color: ${FLOATING_ICON_ON_BASE};
      --md-icon-button-hover-icon-color: ${FLOATING_ICON_ON_BASE};
      --md-icon-button-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-icon-button-hover-state-layer-opacity: 1;
      --md-icon-button-icon-color: ${FLOATING_ICON_ON_BASE};
      --md-icon-button-pressed-icon-color: ${FLOATING_ICON_ON_BASE};
      --md-icon-button-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-icon-button-pressed-state-layer-opacity: 1;
    }
    :host([size="small"]) md-icon-button {
      --md-icon-button-icon-size: var(--cros-icon-button-icon-size, 20px);
      --md-icon-button-state-layer-shape: 10px;
      --md-icon-button-state-layer-width: 32px;
      --md-icon-button-state-layer-height: 32px;
    }
    :host([size="large"]) md-icon-button {
      --md-icon-button-icon-size: var(--cros-icon-button-icon-size, 24px);
      --md-icon-button-state-layer-shape: 16px;
      height: ${LARGE_CONTAINER_HEIGHT};
      margin: 0;
      width: ${LARGE_CONTAINER_WIDTH};
    }
    :host([surface="subtle"]) md-icon-button {
      --md-icon-button-disabled-icon-color: ${FLOATING_ICON_ON_SUBTLE};
      --md-icon-button-focus-icon-color: ${FLOATING_ICON_ON_SUBTLE};
      --md-icon-button-hover-icon-color: ${FLOATING_ICON_ON_SUBTLE};
      --md-icon-button-icon-color: ${FLOATING_ICON_ON_SUBTLE};
      --md-icon-button-pressed-icon-color: ${FLOATING_ICON_ON_SUBTLE};
    }
    :host([surface="prominent"]) md-icon-button {
      --md-icon-button-disabled-icon-color: ${FLOATING_ICON_ON_PROMINENT};
      --md-icon-button-focus-icon-color: ${FLOATING_ICON_ON_PROMINENT};
      --md-icon-button-hover-icon-color: ${FLOATING_ICON_ON_PROMINENT};
      --md-icon-button-hover-state-layer-color: var(--cros-sys-hover_on_prominent);
      --md-icon-button-icon-color: ${FLOATING_ICON_ON_PROMINENT};
      --md-icon-button-pressed-icon-color: ${FLOATING_ICON_ON_PROMINENT};
      --md-icon-button-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_prominent);
    }
    :host([shape="circle"]) md-icon-button {
      --md-icon-button-state-layer-shape: 100vmax;
    }
    md-filled-icon-button {
      --md-filled-icon-button-container-shape: 12px;
      --md-filled-icon-button-container-width: 40px;
      --md-filled-icon-button-container-height: 40px;
      --md-filled-icon-button-disabled-container-color: #00000000;
      --md-filled-icon-button-disabled-container-opacity: 1;
      --md-filled-icon-button-disabled-icon-color: var(--cros-sys-disabled);
      --md-filled-icon-button-disabled-icon-opacity: 1;
      --md-filled-icon-button-hover-state-layer-opacity: 1;
      --md-filled-icon-button-icon-size: var(--cros-icon-button-icon-size, 20px);
      --md-filled-icon-button-pressed-state-layer-opacity: 1;
      --md-filled-icon-button-selected-container-color: ${
      TOGGLE_CONTAINER_ON_BASE};
      --md-filled-icon-button-toggle-selected-focus-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_BASE};
      --md-filled-icon-button-toggle-selected-hover-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_BASE};
      --md-filled-icon-button-toggle-selected-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-filled-icon-button-toggle-selected-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_BASE};
      --md-filled-icon-button-toggle-selected-pressed-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_BASE};
      --md-filled-icon-button-toggle-selected-pressed-state-layer-color: var(--cros-sys-ripple_primary);
      --md-filled-icon-button-toggle-focus-icon-color: ${TOGGLE_ICON_ON_BASE};
      --md-filled-icon-button-toggle-hover-icon-color: ${TOGGLE_ICON_ON_BASE};
      --md-filled-icon-button-toggle-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-filled-icon-button-toggle-icon-color: ${TOGGLE_ICON_ON_BASE};
      --md-filled-icon-button-toggle-pressed-icon-color: ${TOGGLE_ICON_ON_BASE};
      --md-filled-icon-button-toggle-pressed-state-layer-color: var(--cros-sys-ripple_primary);
      --md-filled-icon-button-unselected-container-color: #00000000;
      --md-focus-ring-duration: 0s;
    }
    :host([size="large"]) md-filled-icon-button {
      --md-filled-icon-button-container-shape: 16px;
      --md-filled-icon-button-container-width: ${LARGE_CONTAINER_WIDTH};
      --md-filled-icon-button-container-height: ${LARGE_CONTAINER_HEIGHT};
      --md-filled-icon-button-icon-size: var(--cros-icon-button-icon-size, 24px);
      margin: 0;
    }
    :host([size="small"]) md-filled-icon-button {
      --md-filled-icon-button-container-shape: 10px;
      --md-filled-icon-button-container-width: 32px;
      --md-filled-icon-button-container-height: 32px;
      --md-filled-icon-button-icon-size: var(--cros-icon-button-icon-size, 20px);
    }
    :host([surface="subtle"]) md-filled-icon-button {
      --md-filled-icon-button-selected-container-color: ${
      TOGGLE_CONTAINER_ON_SUBTLE};
      --md-filled-icon-button-toggle-selected-focus-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_SUBTLE};
      --md-filled-icon-button-toggle-selected-hover-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_SUBTLE};
      --md-filled-icon-button-toggle-selected-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_SUBTLE};
      --md-filled-icon-button-toggle-selected-pressed-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_SUBTLE};
      --md-filled-icon-button-toggle-focus-icon-color: ${TOGGLE_ICON_ON_SUBTLE};
      --md-filled-icon-button-toggle-hover-icon-color: ${TOGGLE_ICON_ON_SUBTLE};
      --md-filled-icon-button-toggle-icon-color: ${TOGGLE_ICON_ON_SUBTLE};
      --md-filled-icon-button-toggle-pressed-icon-color: ${
      TOGGLE_ICON_ON_SUBTLE};
    }
    :host([surface="prominent"]) md-filled-icon-button {
      --md-filled-icon-button-selected-container-color: ${
      TOGGLE_CONTAINER_ON_PROMINENT};
      --md-filled-icon-button-toggle-selected-focus-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-selected-hover-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-selected-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-selected-pressed-icon-color: ${
      TOGGLE_SELECTED_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-focus-icon-color: ${
      TOGGLE_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-hover-icon-color: ${
      TOGGLE_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-hover-state-layer-color: var(--cros-sys-hover_on_prominent);
      --md-filled-icon-button-toggle-icon-color: ${TOGGLE_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-pressed-icon-color: ${
      TOGGLE_ICON_ON_PROMINENT};
      --md-filled-icon-button-toggle-selected-hover-state-layer-color: var(--cros-sys-hover_on_prominent);
    }
    :host([shape="circle"]) md-filled-icon-button {
      --md-filled-icon-button-container-shape: 100vmax;
    }
    md-filled-icon-button[selected] {
      --md-filled-icon-button-disabled-container-color: var(--cros-sys-disabled_container);
      --md-filled-icon-button-disabled-icon-color: var(--cros-sys-disabled);
    }
    md-filled-tonal-icon-button::part(focus-ring),
    md-filled-icon-button::part(focus-ring),
    md-icon-button::part(focus-ring) {
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-width: 2px;
    }
    :host([surface="prominent"]) md-filled-tonal-icon-button::part(focus-ring),
    :host([surface="prominent"]) md-filled-icon-button::part(focus-ring),
    :host([surface="prominent"]) md-icon-button::part(focus-ring) {
      --md-focus-ring-color: var(--cros-sys-inverse_focus_ring);
    }

    ::slotted(ea-icon) {
      --ea-icon-size: var(--cros-icon-button-icon-size, 20px);
    }
  `;

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };

  /** @nocollapse */
  static override properties = {
    ariaLabel: {type: String, reflect: true, attribute: 'aria-label'},
    ariaExpanded: {type: String, reflect: true, attribute: 'aria-expanded'},
    ariaHasPopup: {type: String, reflect: true, attribute: 'aria-haspopup'},
    prominent: {type: Boolean, reflect: true},
    buttonStyle: {type: String},
    size: {type: String, reflect: true},
    shape: {type: String, reflect: true},
    surface: {type: String, reflect: true},
    disabled: {type: Boolean},
    selected: {type: Boolean},
    href: {type: String},
  };

  constructor() {
    super();
    this.ariaLabel = '';
    this.buttonStyle = 'filled';
    this.size = 'default';
    this.shape = 'square';
    this.surface = 'base';
    this.disabled = false;
    this.selected = false;
    this.href = '';
  }

  override connectedCallback() {
    super.connectedCallback();
    // All aria properties on button just get proxied down to the real <button>
    // element, as such we set role to presentation so screenreaders ignore
    // this component and instead only read aria attributes off the inner
    // interactive element.
    this.setAttribute('role', 'presentation');
  }

  override render() {
    const ariaLabel = this.ariaLabel || '';
    const ariaExpanded = this.ariaExpanded as 'true' | 'false';
    const ariaHasPopup = this.ariaHasPopup as 'false' | 'true' | 'menu' |
        'listbox' | 'tree' | 'grid' | 'dialog';

    if (this.buttonStyle === 'toggle') {
      return html`
          <md-filled-icon-button
              aria-expanded=${ifDefined(ariaExpanded)}
              aria-haspopup=${ifDefined(ariaHasPopup)}
              aria-label=${ifDefined(ariaLabel)}
              toggle
              ?selected=${this.selected}
              ?disabled=${this.disabled}
              aria-disabled=${this.disabled}
              href=${this.href}>
            <slot name="icon"></slot>
            <slot name="selectedIcon" slot="selected"></slot>
          </md-filled-icon-button>`;
    }

    if (this.buttonStyle === 'floating' && this.surface !== 'image') {
      return html`
          <md-icon-button
              ?disabled=${this.disabled}
              aria-expanded=${ifDefined(ariaExpanded)}
              aria-haspopup=${ifDefined(ariaHasPopup)}
              aria-label=${ifDefined(ariaLabel)}
              aria-disabled=${this.disabled}
              href=${this.href}>
            <slot name="icon"></slot>
          </md-icon-button>`;
    }

    return html`
          <md-filled-tonal-icon-button
              ?disabled=${this.disabled}
              aria-expanded=${ifDefined(ariaExpanded)}
              aria-haspopup=${ifDefined(ariaHasPopup)}
              aria-label=${ifDefined(ariaLabel)}
              aria-disabled=${this.disabled}
              href=${this.href}>
            <slot name="icon"></slot>
          </md-filled-tonal-icon-button>`;
  }
}

customElements.define('cros-icon-button', IconButton);

declare global {
  interface HTMLElementTagNameMap {
    'cros-icon-button': IconButton;
  }
}
