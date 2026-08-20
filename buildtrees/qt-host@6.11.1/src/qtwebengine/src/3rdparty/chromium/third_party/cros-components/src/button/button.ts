/**
 * @license
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/button/filled-button.js';
import '@material/web/button/text-button.js';

import {css, CSSResultGroup, html, LitElement} from 'lit';
import {ifDefined} from 'lit/directives/if-defined';

// The padding on the label start/end when there is no icons.
const LABEL_PADDING_START_END = css`16px`;
// The padding on the icon start/end when present.
const ICON_PADDING_START_END = css`12px`;
// The inline gap between the label and the optional icons.
const ICON_GAP = css`8px`;
const CONTAINER_HEIGHT = css`36px`;
const ICON_SIZE = css`20px`;
const MIN_WIDTH = css`64px`;

/**
 * A chromeOS compliant button.
 */
export class Button extends LitElement {
  /** @nocollapse */
  static override shadowRootOptions:
      ShadowRootInit = {mode: 'open', delegatesFocus: true};

  // Note that theme colours have opacity defined in the colour, but default
  // colours have opacities set separately. As a consequence, styles are broken
  // unless a cros theme is present.
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
      text-overflow: ellipsis;
      text-wrap: nowrap;
      width: fit-content;
    }

    .button {
      max-width: var(--cros-button-max-width,200px);
      min-width: ${MIN_WIDTH};
      text-overflow: inherit;
      text-wrap: inherit;
      width: 100%;
      height: 100%;
    }

    .label {
      overflow: hidden;
      text-overflow: inherit;
    }

    :host([overflow="stack"]) {
      text-wrap: wrap;
    }

    ::slotted(*) {
      display: inline-flex;
      block-size: ${ICON_SIZE};
      inline-size: ${ICON_SIZE};
    }

    .content-container {
      align-items: center;
      display: flex;
      gap: ${ICON_GAP};
    }

    md-filled-button:has(.content-container.has-leading-icon)  {
      --md-filled-button-leading-space: ${ICON_PADDING_START_END};
    }

    md-filled-button:has(.content-container.has-trailing-icon)  {
      --md-filled-button-trailing-space: ${ICON_PADDING_START_END};
    }

    md-text-button:has(.content-container.has-leading-icon)  {
      --md-text-button-leading-space: ${ICON_PADDING_START_END};
    }

    md-text-button:has(.content-container.has-trailing-icon)  {
      --md-text-button-trailing-space: ${ICON_PADDING_START_END};
    }

    md-filled-button {
      --md-filled-button-container-height: ${CONTAINER_HEIGHT};
      --md-filled-button-disabled-container-color: var(--cros-sys-disabled_container);
      --md-filled-button-disabled-container-opacity: 100%;
      --md-filled-button-disabled-label-text-color: var(--cros-sys-disabled);
      --md-filled-button-disabled-label-text-opacity: 100%;
      --md-filled-button-focus-state-layer-opacity: 100%;
      --md-filled-button-hover-container-elevation: 0;
      --md-filled-button-hover-state-layer-opacity: 100%;
      --md-filled-button-label-text-font: var(--cros-button-2-font-family);
      --md-filled-button-label-text-size: var(--cros-button-2-font-size);
      --md-filled-button-label-text-line-height: var(--cros-button-2-line-height);
      --md-filled-button-label-text-weight: var(--cros-button-2-font-weight);
      --md-filled-button-leading-space: ${LABEL_PADDING_START_END};
      --md-filled-button-pressed-state-layer-opacity: 100%;
      --md-filled-button-trailing-space: ${LABEL_PADDING_START_END};
      --md-focus-ring-duration: 0s;
      --md-focus-ring-width: 2px;
      --md-sys-color-secondary: var(--cros-sys-focus_ring);
    }

    :host(:not([button-style="secondary"]):is([inverted][disabled])) {
      opacity: var(--cros-disabled-opacity);
    }

    :host([inverted][button-style="primary"]) md-filled-button {
      /** Base styles */
      --md-sys-color-primary: var(--cros-sys-inverse_primary);
      --md-sys-color-secondary: var(--cros-sys-inverse_focus_ring);
      --md-sys-color-on-primary: var(--cros-sys-inverse_on_primary);
      --md-filled-button-label-text-color: var(--cros-sys-inverse_on_primary);
      /** Disabled */
      --md-filled-button-disabled-container-color: var(--cros-sys-inverse_primary);
      --md-filled-button-disabled-label-text-color: var(--cros-sys-inverse_on_primary);
      /** Hover */
      --md-filled-button-hover-state-layer-color: var(--cros-sys-inverse_hover_on_prominent);
      /** Pressed */
      --md-filled-button-pressed-state-layer-color: var(--cros-sys-inverse_ripple_primary);
    }

    :host([button-style="primary"]) md-filled-button {
      --md-sys-color-primary: var(--cros-sys-primary);
      --md-sys-color-on-primary: var(--cros-sys-on_primary);
      --md-filled-button-hover-state-layer-color: var(--cros-sys-hover_on_prominent);
      --md-filled-button-pressed-state-layer-color: var(--cros-sys-ripple_primary);
    }

    :host([button-style="secondary"]) md-filled-button {
      --md-filled-button-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-filled-button-pressed-state-layer-color: var(--cros-sys-ripple_primary);
      --md-sys-color-primary: var(--cros-sys-primary_container);
      --md-sys-color-on-primary: var(--cros-sys-on_primary_container);
    }

    md-text-button {
      --md-sys-color-primary: var(--cros-sys-primary);
      --md-sys-color-secondary: var(--cros-sys-focus_ring);
      --md-focus-ring-duration: 0s;
      --md-focus-ring-width: 2px;
      --md-text-button-container-height: ${CONTAINER_HEIGHT};
      --md-text-button-disabled-label-text-color: var(--cros-sys-disabled);
      --md-text-button-disabled-label-text-opacity: 100%;
      --md-text-button-focus-state-layer-opacity: 100%;
      --md-text-button-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-text-button-hover-state-layer-opacity: 100%;
      --md-text-button-label-text-color: var(--cros-sys-primary);
      --md-text-button-label-text-font: var(--cros-button-2-font-family);
      --md-text-button-label-text-size: var(--cros-button-2-font-size);
      --md-text-button-label-text-line-height: var(--cros-button-2-line-height);
      --md-text-button-label-text-weight: var(--cros-button-2-font-weight);
      --md-text-button-leading-space: ${LABEL_PADDING_START_END};
      --md-text-button-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-text-button-pressed-state-layer-opacity: 100%;
      --md-text-button-trailing-space: ${LABEL_PADDING_START_END};
    }

    :host([inverted]) md-text-button {
      /** Base styles */
      --md-sys-color-primary: var(--cros-sys-inverse_primary);
      --md-sys-color-secondary: var(--cros-sys-inverse_focus_ring);
      --md-text-button-label-text-color: var(--cros-sys-inverse_primary);
      /** Disabled */
      --md-text-button-disabled-label-text-color: var(--cros-sys-inverse_primary);
      --md-text-button-pressed-state-layer-color: var(--cros-sys-inverse_ripple_neutral_on_subtle);
      --md-text-button-hover-state-layer-color: var(--cros-sys-inverse_hover_on_subtle);
    }

    ::slotted(ea-icon) {
      --ea-icon-size: 20px;
    }
  `;

  /** @export */
  declare label: string;
  /** @export */
  declare disabled: boolean;
  /**
   * How the button should be styled. One of {primary, secondary, floating}.
   * @export
   */
  declare buttonStyle: 'primary'|'secondary'|'floating';

  /**
   * If the button should be in the inverted color scheme, eg for use in
   * cros-snackbar. Inverted color schemes are only supported for `primary` and
   * floating button styles.
   * @export
   */
  declare inverted: boolean;

  /**
   * If button should truncate with ellipsis or stack contents if label
   * overflows button container.
   * @export
   */
  declare overflow: 'truncate'|'stack';

  /**
   * The URL that the link button points to.
   * @export
   */
  declare href: string;

  /** @nocollapse */
  static override properties = {
    ariaLabel: {type: String, reflect: true, attribute: 'aria-label'},
    ariaExpanded: {type: String, reflect: true, attribute: 'aria-expanded'},
    label: {type: String, reflect: true},
    disabled: {type: Boolean, reflect: true},
    buttonStyle: {type: String, reflect: true, attribute: 'button-style'},
    inverted: {type: Boolean, reflect: true},
    ariaHasPopup: {type: String, reflect: true, attribute: 'aria-haspopup'},
    overflow: {type: String, reflect: true},
    href: {type: String},
  };

  constructor() {
    super();
    this.ariaLabel = '';
    this.label = '';
    this.disabled = false;
    this.buttonStyle = 'primary';
    this.inverted = false;
    this.overflow = 'truncate';
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

  override firstUpdated() {
    this.addEventListener('click', this.clickListener);
  }

  private clickListener (e: MouseEvent) {
    if (this.disabled) {
      e.stopImmediatePropagation();
      e.preventDefault();
      return;
    }
  }

  override render() {
    const ariaHasPopup = (this.ariaHasPopup ?? '') as 'false' | 'true' |
        'menu' | 'listbox' | 'tree' | 'grid' | 'dialog';
    const ariaExpanded = (this.ariaExpanded ?? '') as 'true' | 'false';

    if (this.buttonStyle === 'floating') {
      return html`
        <md-text-button
            class="button"
            aria-label=${this.ariaLabel || ''}
            aria-haspopup=${ifDefined(ariaHasPopup)}
            aria-expanded=${ifDefined(ariaExpanded)}
            ?disabled=${this.disabled}
            href=${this.href}>
          ${this.renderButtonContent()}
        </md-text-button>
        `;
    }

    return html`
        <md-filled-button
            class="button"
            aria-label=${this.ariaLabel || ''}
            aria-haspopup=${ifDefined(ariaHasPopup)}
            aria-expanded=${ifDefined(ariaExpanded)}
            ?disabled=${this.disabled}
            href=${this.href}>
          ${this.renderButtonContent()}
        </md-filled-button>
        `;
  }

  private renderButtonContent() {
    return html`
      <div class="content-container">
        <slot name="leading-icon" @slotchange=${this.onSlotChange}></slot>
        <span class="label">${this.label}</span>
        <slot name="trailing-icon" @slotchange=${this.onSlotChange}></slot>
      </div>
    `;
  }

  private hasSlottedIcon(icon: 'leading'|'trailing'): boolean {
    return !!this.querySelector(`*[slot='${icon}-icon']`);
  }

  // The padding before/after the label changes based on whether there is an
  // icon present. We use the slot change event to toggle the different padding
  // styles on the container, as it's easier to achieve here compared with pure
  // CSS selectors. The actual padding is applied to the material button which
  // is difficult to define a pure CSS relationship with due to the nature of
  // how the slots are arranged.
  private onSlotChange() {
    const container = this.shadowRoot!.querySelector('.content-container');
    container!.classList.toggle(
        'has-leading-icon', this.hasSlottedIcon('leading'));
    container!.classList.toggle(
        'has-trailing-icon', this.hasSlottedIcon('trailing'));
  }
}

customElements.define('cros-button', Button);

declare global {
  interface HTMLElementTagNameMap {
    'cros-button': Button;
  }
}
