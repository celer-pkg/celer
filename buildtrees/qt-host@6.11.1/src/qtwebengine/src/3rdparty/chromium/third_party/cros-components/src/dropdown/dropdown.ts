/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/select/outlined-select.js';

import {MdOutlinedSelect} from '@material/web/select/outlined-select.js';
import {css, CSSResultGroup, html, LitElement, nothing, PropertyValues} from 'lit';
import {ifDefined} from 'lit/directives/if-defined';

/**
 * Dropdowns have two variants that differ only by the container background,
 * designed to improve contrast depending on the color of the surface behind it:
 * 1) dropdown-on-app-base (lighter background, app base)
 * 2) dropdown-on-shaded (darker background, shaded base)
 *
 * Consumers of the component can use the `shaded` property to ensure that it
 * has the correct container color for their use case.
 */
const DROPDOWN_CONTAINER_ON_BASE = css`var(--cros-sys-input_field_on_base)`;
const DROPDOWN_CONTAINER_ON_SHADED = css`var(--cros-sys-input_field_on_shaded)`;

/**
 * We need the outline to render on the outside of the dropdown container, which
 * is an element we control. This adds an extra 2px padding to accomodate the
 * extra space, as the md-select does not support changing its height, only line
 * height.
 */
const CROS_DROPDOWN_OUTLINE_WIDTH = 2;
const DEFAULT_TOP_BOTTOM_SPACE = 8;
const CROS_DROPDOWN_TOP_BOTTOM_SPACE_PX =
    css`${DEFAULT_TOP_BOTTOM_SPACE - CROS_DROPDOWN_OUTLINE_WIDTH}px`;
const MD_FIELD_TOP_BOTTOM_SPACE_PX =
    css`${DEFAULT_TOP_BOTTOM_SPACE + CROS_DROPDOWN_OUTLINE_WIDTH}px`;

/** 10px line height + 6px of top padding. */
const SUPPORTING_TEXT_HEIGHT_PX = 16;

/**
 * To account for the extra spacing around the textfield background and the
 * focus outline, we have to increase the corner radius by the outline width.
 */
const CROS_DROPDOWN_CONTAINER_CORNER_RADIUS =
    css`${8 + CROS_DROPDOWN_OUTLINE_WIDTH}px`;

/** The SVG to use in the trailing icon slot when the dropdown is open. */
const ARROW_DROP_UP_ICON = html`
  <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 -960 960 960">
    <path d="M274.848-378.5 480-583.652 685.152-378.5H274.848Z"/>
  </svg>`;

/** The SVG to use in the trailing icon slot when the dropdown is closed. */
const ARROW_DROP_DOWN_ICON = html`
  <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 -960 960 960">
    <path d="M480-376.348 274.848-581.5h410.304L480-376.348Z"/>
  </svg>`;

/**
 * A chromeOS compliant dropdown.
 */
export class Dropdown extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
      --supporting-text-height: 0px;
    }

    md-outlined-select {
      /* Base styles */
      min-width: var(--cros-dropdown-min-width, 210px);
      --md-outlined-field-top-space: ${MD_FIELD_TOP_BOTTOM_SPACE_PX};
      --md-outlined-field-bottom-space: ${MD_FIELD_TOP_BOTTOM_SPACE_PX};
      --md-outlined-field-content-space: 12px;
      --md-outlined-field-trailing-content-color: var(--cros-sys-secondary);
      --md-outlined-field-with-leading-content-leading-space: 16px;

      --md-outlined-select-text-field-container-color: ${
      DROPDOWN_CONTAINER_ON_BASE};
      --md-outlined-select-text-field-container-shape-end-end: ${
      CROS_DROPDOWN_CONTAINER_CORNER_RADIUS};
      --md-outlined-select-text-field-container-shape-end-start: ${
      CROS_DROPDOWN_CONTAINER_CORNER_RADIUS};
      --md-outlined-select-text-field-container-shape-start-end: ${
      CROS_DROPDOWN_CONTAINER_CORNER_RADIUS};
      --md-outlined-select-text-field-container-shape-start-start: ${
      CROS_DROPDOWN_CONTAINER_CORNER_RADIUS};
      --md-outlined-select-text-field-outline-width: 0px;
      --md-outlined-select-text-field-trailing-icon-size: 20px;

      --md-outlined-select-text-field-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-select-text-field-input-text-font: var(--cros-dropdown-input-text-font, var(--cros-body-2-font-family));
      --md-outlined-select-text-field-input-text-line-height: var(--cros-body-2-line-height);
      --md-outlined-select-text-field-input-text-size: var(--cros-body-2-font-size);
      --md-outlined-select-text-field-input-text-weight: var(--cros-body-2-font-weight);
      --md-outlined-select-text-field-leading-icon-size: 20px;

      --md-outlined-field-supporting-text-color: var(--cros-sys-on_surface);
      --md-outlined-field-supporting-text-leading-space: ${
      CROS_DROPDOWN_OUTLINE_WIDTH}px;
      --md-outlined-field-supporting-text-top-space: ${
      CROS_DROPDOWN_TOP_BOTTOM_SPACE_PX};
      --md-outlined-field-supporting-text-trailing-space: ${
      CROS_DROPDOWN_OUTLINE_WIDTH}px;
      --md-outlined-select-text-field-supporting-text-color: var(--cros-sys-on_surface);
      --md-outlined-select-text-field-supporting-text-font: var(--cros-label-2-font-family);
      --md-outlined-select-text-field-supporting-text-line-height: var(--cros-label-2-line-height);
      --md-outlined-select-text-field-supporting-text-size: var(--cros-label-2-font-size);
      --md-outlined-select-text-field-supporting-text-weight: var(--cros-label-2-font-weight);

      /* Error */
      --md-outlined-field-error-content-color: var(--cros-sys-on_surface);
      --md-outlined-field-error-focus-content-color: var(--cros-sys-on_surface);
      --md-outlined-field-error-hover-content-color: var(--cros-sys-on_surface);
      --md-outlined-field-error-focus-trailing-content-color: var(--cros-sys-secondary);
      --md-outlined-field-error-hover-trailing-content-color: var(--cros-sys-secondary);
      --md-outlined-field-error-trailing-content-color: var(--cros-sys-secondary);
      --md-outlined-select-text-field-error-focus-outline-color: var(--cros-sys-error);
      --md-outlined-select-text-field-error-hover-state-layer-opacity: 0;
      --md-outlined-select-text-field-error-hover-supporting-text-color: var(--cros-sys-error);
      --md-outlined-select-text-field-error-supporting-text-color: var(--cros-sys-error);

      /* Focus */
      --md-outlined-field-focus-trailing-content-color: var(--cros-sys-secondary);
      --md-outlined-select-text-field-focus-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-select-text-field-focus-leading-icon-color: var(--cros-sys-secondary);
      --md-outlined-select-text-field-focus-supporting-text-color: var(--cros-sys-on_surface);
      --md-outlined-select-text-field-focus-outline-color: var(--cros-sys-focus_ring);
      --md-outlined-select-text-field-focus-outline-width: ${
      CROS_DROPDOWN_OUTLINE_WIDTH}px;

      /* Hover */
      --md-outlined-field-hover-trailing-content-color: var(--cros-sys-secondary);
      --md-outlined-select-text-field-hover-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-select-text-field-hover-outline-width: 0px;
      --md-outlined-select-text-field-hover-state-layer-opacity: 0;
      --md-outlined-select-text-field-hover-supporting-text-color: var(--cros-sys-on_surface);

      /* Disabled */
      --md-outlined-field-disabled-supporting-text-color: var(--cros-sys-on_surface);
      --md-outlined-field-disabled-trailing-content-color: var(--cros-sys-secondary);
      --md-outlined-select-text-field-disabled-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-select-text-field-disabled-outline-width: 0px;
      --md-outlined-select-text-field-disabled-supporting-text-color: var(--cros-sys-on_surface);

      /* Md-menu */
      --md-menu-container-color: var(--cros-sys-base_elevated);
      --md-menu-container-shape: 8px;
      --md-menu-item-bottom-space: 0px;
      --md-menu-item-container-color: var(--cros-sys-base_elevated);
      --md-menu-item-one-line-container-height: 36px;
      --md-menu-item-top-space: 0px;
    }

    .text-labels {
      color: var(--cros-sys-on_surface);
    }

    :host(:focus-within) .text-labels {
      color: var(--cros-sys-primary);
    }

    :host([disabled]) .text-labels {
      opacity: var(--cros-disabled-opacity);
    }

    :host([disabled]) #dropdown-background {
      opacity: var(--cros-disabled-opacity);
    }

    :host([supporting-text]:not([supporting-text=""])), :host(:is(.error, [error])) {
      --supporting-text-height: ${SUPPORTING_TEXT_HEIGHT_PX}px;
    }
    /**
     * The .error class is applied on native constraint invalidation, and the
     * error attribute is set by clients, so we need to use both to style the
     * label correctly.
    */
    :host(:is(.error, [error])) .text-labels {
      color: var(--cros-sys-error);
    }

    #visible-label {
      font: var(--cros-label-1-font);
      padding-bottom: ${CROS_DROPDOWN_TOP_BOTTOM_SPACE_PX};
      padding-inline-start: ${CROS_DROPDOWN_OUTLINE_WIDTH}px;
    }

    #main-container {
      position: relative;
    }

    #dropdown-background {
      background-color: ${DROPDOWN_CONTAINER_ON_BASE};
      border-radius: 8px;
      left: ${CROS_DROPDOWN_OUTLINE_WIDTH}px;
      min-height: 36px;
      position: absolute;
      right: ${CROS_DROPDOWN_OUTLINE_WIDTH}px;
      top: ${CROS_DROPDOWN_OUTLINE_WIDTH}px;
      height: calc(100% - 2 * ${
      CROS_DROPDOWN_OUTLINE_WIDTH}px - var(--supporting-text-height));
    }

    :host([shaded]) #dropdown-background {
      background-color: ${DROPDOWN_CONTAINER_ON_SHADED};
    }

    /**
     * Some md styling can override the color provided to the md-select, so we
     * explicitly set it here as well.
     */
    ::slotted(svg) {
      fill: var(--cros-sys-secondary);
    }

    ::slotted(*) {
      color: var(--cros-sys-secondary);
      height: var(--md-outlined-select-text-field-leading-icon-size);
      width: var(--md-outlined-select-text-field-leading-icon-size);
    }

    slot[name="trailing"] {
      fill: var(--cros-sys-secondary)
    }
  `;

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true,
  };

  /** @nocollapse */
  static override properties = {
    ariaLabel: {type: String, reflect: true, attribute: 'aria-label'},
    disabled: {type: Boolean, reflect: true},
    error: {type: Boolean, attribute: true},
    errorText: {type: String, attribute: true},
    label: {type: String},
    open: {type: Boolean, reflect: true},
    shaded: {type: Boolean, reflect: true},
    supportingText: {type: String, reflect: true, attribute: 'supporting-text'},
    value: {type: String},
  };

  private preRenderValue: string|undefined = undefined;

  /** @export */
  declare disabled: boolean;

  /**
   * The error text shown below the dropdown on `error`. This will override
   * native error messages returned on validation, and is only shown when
   * `error` is set.
   *  @export
   */
  declare errorText?: string;

  /** @export */
  declare error: boolean;

  /**
   * The visible label above the dropdown.
   * @export
   */
  declare label: string;

  /**
   * Whether or not the dropdown is open.
   * @export
   */
  declare open: boolean;

  /**
   * When false, will use the darker container designed to sit on app-base. When
   * true, will use the lighter container colored designed for use with
   * app-base-shaded. Purely a cosmetic difference to improve contrast.
   * @export
   */
  declare shaded: boolean;

  /** @export */
  declare supportingText?: string;

  get mdSelect(): MdOutlinedSelect|null {
    return this.shadowRoot?.querySelector('md-outlined-select') || null;
  }

  get mdField(): HTMLElement|null {
    return this.mdSelect?.shadowRoot?.querySelector('md-outlined-field') ||
        null;
  }

  get trailingIcon(): HTMLElement|null {
    return this.shadowRoot!.querySelector('slot[name="trailing"]') || null;
  }

  /**
   * The value of the dropdown, should match the `value` property of a child
   * dropdown option. Defaults to empty string if there is no selected option.
   * @export
   */
  get value() {
    if (!this.mdSelect) {
      return this.preRenderValue ?? '';
    }
    return this.mdSelect.value;
  }

  set value(value: string) {
    if (!this.mdSelect) {
      this.preRenderValue = value;
    } else {
      this.mdSelect.value = value;
    }
  }

  constructor() {
    super();
    this.disabled = false;
    this.error = false;
    this.errorText = '';
    this.label = '';
    this.open = false;
    this.shaded = false;
    this.supportingText = '';
  }

  override async firstUpdated() {
    // Run the logic to forward any slotted icons to the md-select internal
    // icon slots.
    this.handleIconChange();
  }

  override update(changedProperties: PropertyValues<Dropdown>) {
    // There is no corresponding 'valid' event to match md-select's
    // 'invalid' event, so just check on every update to see if it's still
    // invalid and remove the class if not.
    if (this.mdSelect && this.classList.contains('error')) {
      this.toggleErrorStyles(this.mdSelect.error);
    }

    super.update(changedProperties);
  }

  override render() {
    const ariaLabel = this.ariaLabel || this.label;
    const errorTextOrUndef =
        this.error && this.errorText ? this.errorText : undefined;

    return html`
      ${this.maybeRenderLabel()}
      <div id="main-container">
        <div id="dropdown-background"></div>
        <md-outlined-select
            @change=${this.onChange}
            @opened=${this.onOpened}
            @closed=${this.onClosed}
            ?disabled=${this.disabled}
            ?error=${this.error ?? nothing}
            aria-expanded=${this.open ? 'true' : 'false'}
            aria-label=${ariaLabel}
            error-text=${ifDefined(errorTextOrUndef)}
            supporting-text=${this.supportingText || ''}
            quick
            value=${ifDefined(this.preRenderValue)}>
          <slot
              name="leading"
              @slotchange=${this.handleIconChange}>
          </slot>
          <slot></slot>
          <slot
            name="trailing"
            slot="trailing-icon">
            ${this.open ? ARROW_DROP_UP_ICON : ARROW_DROP_DOWN_ICON}
          </slot>
        </md-outlined-select>
      </div>
    `;
  }

  private onChange() {
    // Md-select's changeevent won't exit out shadow DOM, redispatch it to
    // ensure clients can listen to it.
    this.dispatchEvent(new Event('change', {bubbles: true}));
  }

  // Manually set the arrow colors by listening to opened and closed events
  // because we can't pierce through the shadow root of the md-select from our
  // wrapper to read aria-expanded.
  private onOpened() {
    this.open = true;
    this.trailingIcon?.style.setProperty('fill', 'var(--cros-sys-primary)');
  }

  private onClosed() {
    this.open = false;
    this.trailingIcon?.style.setProperty('fill', 'var(--cros-sys-secondary)');
    this.mdField!.ariaExpanded = 'false';
  }

  private toggleErrorStyles(error: boolean) {
    this.classList.toggle('error', error);
  }

  private handleIconChange() {
    // Add the icon to the slot. If there is no slotted icon, remove the empty
    // leadingSlot from the md slot so we return to the original state.
    const leadingSlot = this.shadowRoot!.querySelector('slot[name="leading"]');

    if (this.hasSlottedIcon()) {
      leadingSlot!.setAttribute('slot', 'leading-icon');
    } else {
      leadingSlot!.removeAttribute('slot');
    }
  }

  private hasSlottedIcon() {
    return this.querySelector(`*[slot="leading"]`) !== null;
  }

  private maybeRenderLabel() {
    return this.label ?
        html`<div id="visible-label" class="text-labels">${this.label}</div>` :
        nothing;
  }
}

customElements.define('cros-dropdown', Dropdown);

declare global {
  interface HTMLElementTagNameMap {
    'cros-dropdown': Dropdown;
  }
}
