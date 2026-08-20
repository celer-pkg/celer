/**
 * @license
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/textfield/outlined-text-field.js';

import {MdOutlinedTextField, TextFieldType} from '@material/web/textfield/outlined-text-field.js';
import {css, CSSResultGroup, html, LitElement, nothing, PropertyValues} from 'lit';
import {classMap} from 'lit/directives/class-map';
import {ifDefined} from 'lit/directives/if-defined';

/**
 * Textfields have two variants that differ only by the container background,
 * designed to improve contrast depending on the color of the surface behind it:
 * 1) textfield-on-app-base (lighter background, app base)
 * 2) textfield-on-shaded (darker background, shaded base)
 *
 * Consumers of the component can use the `shaded` property to ensure that it
 * has the correct container color for their use case.
 */
const TEXTFIELD_CONTAINER_ON_BASE = css`var(--cros-sys-input_field_on_base)`;
const TEXTFIELD_CONTAINER_ON_SHADED =
    css`var(--cros-sys-input_field_on_shaded)`;
const ICON_COLOR =
    css`var(--cros-textfield-icon-color, var(--cros-sys-secondary))`;

/**
 * We need the outline to render on the outside of the textfield filled
 * container, which is an element we control. This adds an extra
 * 2px padding to the text to accomodate the extra space, as the
 * md-textfield does not support changing it's height, only line height.
 * These values are computed from the default values at
 * https://carbon.googleplex.com/google-material-3/pages/text-fields/specs
 * and adding on the outline width. Computed to
 */
const MD_TEXTFIELD_OUTLINE_WIDTH = 2;
const DEFAULT_TOP_BOTTOM_SPACE = 8;
const DEFAULT_LEFT_RIGHT_SPACE = 16;
const MD_FIELD_TOP_BOTTOM_SPACE_PX =
    css`${DEFAULT_TOP_BOTTOM_SPACE + MD_TEXTFIELD_OUTLINE_WIDTH}px`;
const MD_FIELD_LEFT_RIGHT_SPACE_PX =
    css`${DEFAULT_LEFT_RIGHT_SPACE + MD_TEXTFIELD_OUTLINE_WIDTH}px`;

/**
 * To account for the extra spacing around the textfield background and the
 * focus outline, we have to increase the corner radius by the outline width.
 */
const MD_TEXTFIELD_CONTAINER_CORNER_RADIUS =
    css`${8 + MD_TEXTFIELD_OUTLINE_WIDTH}px`;

/**
 * Supported types for cros-textfield:
 * 'email'
 * 'number'
 * 'password'
 * 'search'
 * 'tel'
 * 'text'
 * 'url'
 * 'textarea'
 * 'integer'
 */
export type SupportedTextfieldType = TextFieldType|'integer';

/**
 * Supported autofix types for cros-textfield (only for type=integer):
 * 'clear'
 * 'preserve'
 * 'strip'
 */
export type TextfieldAutofixType = 'clear'|'preserve'|'strip';

/** Regex used for automatically stripping non numeric characters from input. */
const NON_INTEGER_REGEX = /\D/g;

// DO NOT MODIFY THIS STYLESHEET
// Added by MWC team to be removed in b/278960272.
const MWC_SELECTION_STYLES = css`
  ::selection {
    background-color: var(--cros-sys-highlight_text);
  }
`;

const TEXT_ELLISION_STYLES = css`
  :host(.use-ellipsis) .input-wrapper > input {
    text-overflow: ellipsis;
  }
`;

/**
 * ChromeOS compliant Textfield component.
 */
export class Textfield extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: block;
    }

    :host(.focused) {
      outline: none;
    }

    :host([disabled]) .text-labels {
      opacity: var(--cros-disabled-opacity);
    }

    md-outlined-text-field {
      /** Base styles */
      --md-outlined-field-supporting-text-color: var(--cros-sys-on_surface);
      --md-outlined-field-supporting-text-leading-space: ${
      MD_TEXTFIELD_OUTLINE_WIDTH}px;
      --md-outlined-field-supporting-text-trailing-space: ${
      MD_TEXTFIELD_OUTLINE_WIDTH}px;
      --md-outlined-field-supporting-text-top-space: 6px;
      --md-outlined-text-field-focus-caret-color: var(--cros-sys-primary);
      --md-outlined-text-field-container-shape-end-end: ${
      MD_TEXTFIELD_CONTAINER_CORNER_RADIUS};
      --md-outlined-text-field-container-shape-end-start: ${
      MD_TEXTFIELD_CONTAINER_CORNER_RADIUS};
      --md-outlined-text-field-container-shape-start-end: ${
      MD_TEXTFIELD_CONTAINER_CORNER_RADIUS};
      --md-outlined-text-field-container-shape-start-start: ${
      MD_TEXTFIELD_CONTAINER_CORNER_RADIUS};
      --md-outlined-text-field-bottom-space: 8px;
      --md-outlined-text-field-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-input-text-placeholder-color: var(--cros-sys-secondary);
      --md-outlined-text-field-input-text-prefix-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-input-text-suffix-color: var(--cros-sys-secondary);
      --md-outlined-text-field-input-text-suffix-leading-space: 8px;
      --md-outlined-text-field-input-text-font: var(--cros-textfield-font-family, var(--cros-body-2-font-family));
      --md-outlined-text-field-input-text-size: var(--cros-textfield-font-size, var(--cros-body-2-font-size));
      --md-outlined-text-field-input-text-line-height: var(--cros-body-2-line-height);
      --md-outlined-text-field-input-text-weight: var(--cros-textfield-font-weight, var(--cros-body-2-font-weight));
      --md-outlined-text-field-outline-width: 0px;
      --md-outlined-text-field-supporting-text-font: var(--cros-label-2-font-family);
      --md-outlined-text-field-supporting-text-size: var(--cros-label-2-font-size);
      --md-outlined-text-field-supporting-text-line-height: var(--cros-label-2-line-height);
      --md-outlined-text-field-supporting-text-weight: var(--cros-label-2-font-weight);
      --md-outlined-text-field-top-space: 8px;
      --md-outlined-field-bottom-space: ${MD_FIELD_TOP_BOTTOM_SPACE_PX};
      --md-outlined-field-leading-space: ${MD_FIELD_LEFT_RIGHT_SPACE_PX};
      --md-outlined-field-top-space: ${MD_FIELD_TOP_BOTTOM_SPACE_PX};
      --md-outlined-field-trailing-space: ${MD_FIELD_LEFT_RIGHT_SPACE_PX};
      /** Leading/trailing spacing */
      --md-outlined-field-content-space: 8px;
      --md-outlined-field-with-leading-content-leading-space: 12px;
      --md-outlined-field-with-trailing-content-trailing-space: 14px;
      /** Disabled */
      --md-outlined-field-disabled-supporting-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-disabled-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-disabled-input-text-prefix-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-disabled-input-text-suffix-color: var(--cros-sys-secondary);
      --md-outlined-text-field-disabled-outline-width: 0px;
      /** Error */
      --md-outlined-text-field-error-focus-caret-color: var(--cros-sys-primary);
      --md-outlined-text-field-error-focus-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-error-focus-outline-color: var(--cros-sys-error);
      --md-outlined-text-field-error-hover-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-error-hover-state-layer-opacity: 0;
      --md-outlined-text-field-error-hover-supporting-text-color: var(--cros-sys-error);
      --md-outlined-text-field-error-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-error-supporting-text-color: var(--cros-sys-error);
      /** Focus */
      --md-outlined-field-focus-outline-color: var(--cros-sys-primary);
      --md-outlined-field-focus-outline-width: ${MD_TEXTFIELD_OUTLINE_WIDTH}px;
      --md-outlined-text-field-focus-outline-color: var(--cros-sys-primary);
      --md-outlined-text-field-focus-input-text-color: var(--cros-sys-on_surface);
      /** Hover */
      --md-outlined-text-field-hover-outline-width: 0px;
      --md-outlined-text-field-hover-supporting-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-hover-input-text-color: var(--cros-sys-on_surface);
      --md-outlined-text-field-hover-state-layer-opacity: 0;
      text-align: inherit;
      width: 100%;
    }

    .text-labels {
      color: var(--cros-sys-on_surface);
    }

    :host(.focused) .text-labels {
      color: var(--cros-sys-primary);
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
      padding-bottom: 8px;
      padding-inline-start: ${MD_TEXTFIELD_OUTLINE_WIDTH}px;
    }

    #main-container {
      position: relative;
    }

    #textfield-background {
      background-color: ${TEXTFIELD_CONTAINER_ON_BASE};
      border-radius: 8px;
      inset: 2px;
      height: calc(100% - (2 * ${MD_TEXTFIELD_OUTLINE_WIDTH}px));
      position: absolute;
    }

    :host([shaded]) #textfield-background {
      background-color: ${TEXTFIELD_CONTAINER_ON_SHADED};
    }

    /**
     * This adjustment updates the trailing icon to match the spec. This cannot
     * be achieved with the md-outlined-field spacing tokens because the inner
     * spacing is symmetric.
     */
    ::slotted([slot="trailing"]) {
      position: relative;
      inset-inline-start: 4px;
    }

    /**
     * Some md styling can override the color provided to the md-textfield, so
     * so we explicitly set it here as well.
     */
    ::slotted(cros-icon-button) {
      --cros-icon-button-color-override: ${ICON_COLOR};
    }

    ::slotted(svg) {
      fill: ${ICON_COLOR};
    }

    ::slotted(*) {
      color: ${ICON_COLOR};
    }
  `;

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };

  /** @nocollapse */
  static override properties = {
    type: {type: String, attribute: true},
    shaded: {type: Boolean, reflect: true},
    value: {type: String, attribute: true},
    label: {type: String, attribute: true},
    ariaLabel: {type: String, reflect: true, attribute: 'aria-label'},
    suffix: {type: String, attribute: true},
    disabled: {type: Boolean, reflect: true},
    hint: {type: String, attribute: true},
    maxLength: {type: Number, attribute: true},
    placeholder: {type: String, attribute: true},
    min: {type: Number, attribute: true},
    max: {type: Number, attribute: true},
    autoValidate: {type: Boolean, attribute: true},
    errorMessage: {type: String, attribute: true},
    error: {type: Boolean, attribute: true},
    pattern: {type: String, attribute: true},
    autofix: {type: String, attribute: true},
    required: {type: Boolean, attribute: true},
    noSpinner: {type: Boolean, attribute: true},
    rows: {type: Number, attribute: true},
    useEllipsis: {type: Boolean, attribute: true},
  };

  /** @nocollapse */
  static events = {
    /**
     * The textfield value changed via user input. Dispatched on blur/when a
     * changed value is committed.
     */
    CHANGE: 'change',
    /**
     * The textfield value changed via user input, but dispatched immediately on
     * keystroke/every value change.
     */
    INPUT: 'input',
    /** The entered textfield value is invalid.  */
    INVALID: 'invalid',
  } as const;

  /** @export */
  declare type: SupportedTextfieldType;
  /**
   * When false, will use the darker container designed to sit on app-base. When
   * true, will use the lighter container colored designed for use with
   * app-base-shaded. Purely a cosmetic difference to improve contrast.
   * @export
   */
  declare shaded: boolean;

  /** @export */
  get value() {
    return this.mdTextfield?.value || '';
  }

  set value(value: string) {
    // On first render the initial value is lost because mwcTextfield is not yet
    // rendered. Store it in `valueInitial` and set it in firstUpdated().
    this.valueInitial = value;
    if (this.mdTextfield) {
      this.mdTextfield.value = this.valueInitial;
      this.checkAndUpdateValidity();
    }
  }

  /**
   * The visible label above the textfield. This is also used as the aria-label
   * for the internal md-textfield.
   * @export
   */
  declare label: string;
  // Properties supported by the MD textfield that are surfaced to the
  // cros-textfield API. These are manually plumbed through as cros-textfield is
  // not a subclass. This is not an exhaustive list, and support should be
  // added by clients as needed.

  /** @export */
  declare suffix: string;

  /** @export */
  declare disabled: boolean;

  /** @export */
  declare hint: string;

  /**
   * Max length of the textfield value. If set to -1 or less, md-textfield will
   * ignore this value when restricting the length of the textfield, and also
   * not render the charCounter.
   * @export
   */
  declare maxLength: number;

  /**
   * Defines the most negative value in the range of permitted values. Only used
   * with `number` type.
   * @export
   */
  declare min: number;

  /**
   * Defines the greatest value in the range of permitted values. Only used with
   * `number` type.
   * @export
   */
  declare max: number;

  /**
   * When true, the validity state of the textfield is reported on every
   * input event (eg each keystroke) rather than on each change event (only
   * occur on blur).
   * @export
   */
  declare autoValidate: boolean;

  /**
   * The error text shown below the textfield on `error`. This will override
   * native error messages returned on validation, and is only shown when
   * `error` is set.
   * @export
   */
  declare errorMessage: string;

  /**
   * Gets or sets whether or not the text field is in a visually invalid state.
   * This overrides the error state controlled by the internal textfield's
   * `reportValidity()`, so should only be set if the error state of the
   * textfield is determined by application logic instead of the HTMLInput
   * constraint validation.
   * @export
   */
  declare error: boolean;

  /**
   * A regular expression that the text field's value must match to pass
   * constraint validation.
   *
   * https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input#pattern
   * @export
   */
  declare pattern: string;

  /** @export */
  declare placeholder: string;

  /**
   * When using `type=integer`, clients can also set this field to either
   * `clear`, `preserve` or `strip` to force the value to valid value, removing
   * any non integer characters. For every other type except for `integer`,
   * `autofix` is ignored.
   *
   * preserve:    Restore to the last valid value. (Default)
   * clear:       Set value to the empty string.
   * strip:       Remove any non-integer characters.
   */
  declare autofix: TextfieldAutofixType;

  /**
   * Whether the input should be invalid when left blank.
   * @export
   */
  declare required: boolean;

  /**
   * When true, hide the spinner for `type="number"` text fields.
   * @export
   */
  declare noSpinner: boolean;


  /**
   * The number of rows to display for a `type="textarea"` text field.
   * @export
   */
  declare rows: number;

  /**
   * When true, adds an ellipsis to the end of the input text if it is
   * overflowing the available width. Only applies when the cursor is not
   * currently in the textfield input.
   * @export
   */
  declare useEllipsis: boolean;

  get mdTextfield(): MdOutlinedTextField|undefined {
    return this.renderRoot?.querySelector('md-outlined-text-field') ||
        undefined;
  }

  /**
   * A copy of the value on initialization to ensure mdTextfield has the
   * correct value on first render.
   */
  private valueInitial = '';

  /**
   * A copy of the last valid value we had, updated on change events. Used to
   * restore the textfield value when `autofix=preserve` is used.
   */
  private lastValidValue = '';

  constructor() {
    super();
    this.type = 'text';
    this.shaded = false;
    this.label = '';
    this.suffix = '';
    this.disabled = false;
    this.hint = '';
    this.maxLength = -1;
    this.autoValidate = false;
    this.min = -1;
    this.max = -1;
    this.errorMessage = '';
    this.error = false;
    this.pattern = '';
    this.placeholder = '';
    this.autofix = 'preserve';
    this.required = false;
    this.noSpinner = false;
    this.rows = 2;
    this.useEllipsis = false;
  }

  override async firstUpdated() {
    this.mdTextfield!.value = this.valueInitial;
    // Run the logic to forward any slotted icons to the md-text-field internal
    // icon slots.
    this.handleIconChange();

    // Ensure md-text-field has finished rendering before adding our custom
    // styles.
    await this.mdTextfield!.updateComplete;
    this.mdTextfield!.shadowRoot!.adoptedStyleSheets = [
      ...this.mdTextfield!.shadowRoot!.adoptedStyleSheets,
      // Added by MWC team to be removed in b/278960272.
      MWC_SELECTION_STYLES.styleSheet!, TEXT_ELLISION_STYLES.styleSheet!
    ];
  }

  override update(changedProperties: PropertyValues<Textfield>) {
    if (changedProperties.has('disabled')) {
      // Work around for b/315384008.
      this.renderRoot.querySelector('md-outlined-text-field')?.requestUpdate();
    }

    super.update(changedProperties);
  }

  override render() {
    const ariaLabel = this.ariaLabel || this.label;
    const errorTextOrUndef =
        this.error && this.errorMessage ? this.errorMessage : undefined;
    const mdType = this.type === 'integer' ? 'number' : this.type;
    const classList = {'use-ellipsis': this.useEllipsis};
    return html`
      ${this.maybeRenderLabel()}
      <div id="main-container">
        <md-outlined-text-field
            class=${classMap(classList)}
            ?disabled=${this.disabled}
            type=${mdType}
            aria-label=${ariaLabel}
            value=${this.value}
            suffix-text=${this.suffix}
            maxLength=${this.maxLength}
            min=${this.min > -1 ? this.min : nothing}
            max=${this.max > -1 ? this.max : nothing}
            ?required=${this.required}
            ?no-spinner=${this.noSpinner}
            rows=${this.rows}
            supporting-text=${this.hint}
            ?error=${this.error ?? nothing}
            error-text=${ifDefined(errorTextOrUndef)}
            pattern=${this.pattern}
            step=1
            placeholder=${this.placeholder}
            @focus=${this.toggleFocusStyle}
            @blur=${this.toggleFocusStyle}
            @change=${this.onChange}
            @input=${this.onInput}
            @invalid=${this.onInvalid}>
          <slot name="leading" @slotchange=${this.handleIconChange}></slot>
          <slot
              name="trailing"
              @slotchange=${this.handleIconChange}>
          </slot>
          <div slot="container" id="textfield-background"></div>
        </md-outlined-text-field>
      </div>
    `;
  }

  focusTextfield() {
    this.mdTextfield!.focus();
  }

  blurTextfield() {
    this.mdTextfield!.blur();
  }

  reportValidity() {
    const valid = this.mdTextfield!.reportValidity();
    this.toggleErrorStyles(!valid);
    return valid;
  }

  private maybeRenderLabel() {
    return this.label ?
        html`<div id="visible-label" class="text-labels">${this.label}</div>` :
        nothing;
  }

  private handleIconChange() {
    const {leadingIcon, trailingIcon} = this.queryIconSlots();
    // When no icon in slot is given, we need to remove the slot attribute from
    // cros-icon-button, otherwise md-textfield will still render an empty
    // space.
    const leadingSlot = this.shadowRoot!.querySelector('slot[name="leading"]');
    const trailingSlot =
        this.shadowRoot!.querySelector('slot[name="trailing"]');

    if (leadingIcon) {
      leadingSlot!.setAttribute('slot', 'leading-icon');
    } else {
      leadingSlot!.removeAttribute('slot');
    }
    if (trailingIcon) {
      trailingSlot!.setAttribute('slot', 'trailing-icon');
    } else {
      trailingSlot!.removeAttribute('slot');
    }
  }

  private checkAndUpdateValidity() {
    const valid = this.mdTextfield!.reportValidity();
    this.toggleErrorStyles(!valid);
  }

  private toggleErrorStyles(error: boolean) {
    this.classList.toggle('error', error);
  }

  private toggleFocusStyle() {
    this.classList.toggle('focused');
  }

  // Sync the validity state here to ensure our visible label has the correct
  // styling, as there is no corresponding `valid` event dispatched.
  private onChange() {
    // If the last committed value contained non numeric characters, we can
    // strip them out here.
    if (this.type === 'integer' && this.value.match(NON_INTEGER_REGEX)) {
      switch (this.autofix) {
        case 'clear':
          this.value = '';
          break;
        case 'strip':
          this.value = this.value.replace(NON_INTEGER_REGEX, '');
          break;
        case 'preserve':
        default:
          this.value = this.lastValidValue;
      }
    }
    this.checkAndUpdateValidity();
    this.dispatchEvent(new Event('change', {bubbles: true}));
    this.lastValidValue = this.value;
  }

  private onInput() {
    if (this.autoValidate) {
      this.checkAndUpdateValidity();
    }
  }

  private onInvalid() {
    this.toggleErrorStyles(true);
    this.dispatchEvent(new Event('invalid', {bubbles: true}));
  }

  private getSlottedIcon(slotName: 'leading'|'trailing') {
    return this.querySelector(`*[slot="${slotName}"]`) !== null;
  }

  private queryIconSlots() {
    return {
      leadingIcon: this.getSlottedIcon('leading'),
      trailingIcon: this.getSlottedIcon('trailing'),
    };
  }
}

customElements.define('cros-textfield', Textfield);

declare global {
  interface HTMLElementEventMap {
    [Textfield.events.CHANGE]: Event;
    [Textfield.events.INPUT]: Event;
    [Textfield.events.INVALID]: Event;
  }

  interface HTMLElementTagNameMap {
    'cros-textfield': Textfield;
  }
}
