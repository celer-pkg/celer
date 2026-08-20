/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '../tooltip/tooltip';
import '@material/web/chips/filter-chip.js';
import '@material/web/chips/input-chip.js';
import '@material/web/chips/suggestion-chip.js';

import {css, CSSResultGroup, html, LitElement, TemplateResult} from 'lit';

/** Size of an icon rendered inside a chip. */
export const ICON_SIZE = css`20px`;
/** Width of a focus ring. */
export const FOCUS_RING_WIDTH = css`2px`;
/** Horizontal chip padding. */
export const CHIP_PADDING = css`12px`;

/**
 * The debounce timeout for the ResizeObserver callbacks. After this amount of
 * time, the ResizeObserver callbacks will be called and the chip will be
 * updated. This is used to improve performance.
 */
const RESIZE_OBSERVER_DEBOUNCE_TIMEOUT_MS = 500;

const closeIcon = html`
  <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 -960 960 960" id="close-icon" slot='remove-trailing-icon'>
    <path d="M291-232.348 232.348-291l189-189-189-189L291-727.652l189 189 189-189L727.652-669l-189 189 189 189L669-232.348l-189-189-189 189Z"/>
  </svg>`;

const checkedIcon = html`
  <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 -960 960 960" id="checked-icon" slot="selected-icon">
    <path d="M395-277.348 218.348-455l57.891-57.891L395-395.131l288.761-287.76L741.652-624 395-277.348Z"/>
  </svg>`;

/**
 * A ChromeOS compliant chip component.
 */
export class Chip extends LitElement {
  /** @export */
  declare disabled: boolean;
  /** @export */
  declare label: string;
  /** @export */
  declare type: 'filter'|'input';
  /**
   * Input chips can have a trailing icon (to close chip) or not.
   * @export
   */
  declare trailingIcon: boolean;
  /**
   * Input chips can have an avatar as leading icon which makes the icon
   * circular and changes border radius of component.
   * @export
   */
  declare avatar: boolean;
  /**
   * If true, the chip will show a tooltip when the label is truncated.
   * @export
   */
  declare showTooltipWhenTruncated: boolean;

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
    }

    :host([disabled]) {
      pointer-events: none;
    }

    md-filter-chip {
      --md-filter-chip-disabled-label-text-color: var(--cros-sys-on_surface);
      --md-filter-chip-disabled-label-text-opacity: var(--cros-sys-opacity-disabled);
      --md-filter-chip-disabled-leading-icon-color: var(--cros-sys-disabled);
      --md-filter-chip-disabled-leading-icon-opacity: var(--cros-sys-opacity-disabled);
      --md-filter-chip-disabled-outline-color: var(--cros-sys-separator);
      --md-filter-chip-disabled-outline-opacity: var(--cros-sys-opacity-disabled);
      --md-filter-chip-disabled-selected-container-color: var(--cros-sys-disabled_container);
      --md-filter-chip-disabled-selected-container-opacity: 1;
      --md-filter-chip-disabled-trailing-icon-color: var(--cros-sys-disabled);
      --md-filter-chip-disabled-trailing-icon-opacity: var(--cros-sys-opacity-disabled);
      --md-filter-chip-focus-label-text-color: var(--cros-sys-on_surface);
      --md-filter-chip-focus-leading-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-focus-trailing-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-hover-label-text-color: var(--cros-sys-on_surface);
      --md-filter-chip-hover-leading-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-hover-trailing-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-filter-chip-hover-state-layer-opacity: 1;
      --md-filter-chip-icon-size: ${ICON_SIZE};
      --md-filter-chip-label-text-font: var(--cros-button-1-font-family);
      --md-filter-chip-label-text-size: var(--cros-button-1-font-size);
      --md-filter-chip-label-text-line-height: var(--cros-button-1-line-height);
      --md-filter-chip-label-text-weight: var(--cros-button-1-font-weight);
      --md-filter-chip-label-text-color: var(--cros-sys-on_surface);
      --md-filter-chip-leading-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-leading-space:  ${CHIP_PADDING};
      --md-filter-chip-outline-color: var(--cros-sys-separator);
      --md-filter-chip-pressed-label-text-color: var(--cros-sys-on_surface);
      --md-filter-chip-pressed-leading-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-pressed-trailing-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-filter-chip-pressed-state-layer-opacity: 1;
      --md-filter-chip-selected-container-color: var(--cros-sys-secondary_container);
      --md-filter-chip-selected-hover-label-text-color: var(--cros-sys-on_secondary_container);
      --md-filter-chip-selected-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-filter-chip-selected-hover-state-layer-opacity: 1;
      --md-filter-chip-selected-label-text-color: var(--cros-sys-on_secondary_container);
      --md-filter-chip-selected-leading-icon-color: var(--cros-sys-on_secondary_container);
      --md-filter-chip-selected-outline-width: 1px;
      --md-filter-chip-selected-pressed-label-text-color: var(--cros-sys-on_secondary_container);
      --md-filter-chip-selected-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-filter-chip-selected-pressed-state-layer-opacity: 1;
      --md-filter-chip-selected-trailing-icon-color: var(--cros-sys-on_secondary_container);
      --md-filter-chip-trailing-icon-color: var(--cros-sys-on_surface);
      --md-filter-chip-trailing-space: ${CHIP_PADDING};
      --md-focus-ring-duration: 0s;
    }

    md-suggestion-chip {
      --md-focus-ring-duration: 0s;
      --md-suggestion-chip-disabled-label-text-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-disabled-label-text-opacity: var(--cros-sys-opacity-disabled);
      --md-suggestion-chip-disabled-leading-icon-color: var(--cros-sys-disabled);
      --md-suggestion-chip-disabled-leading-icon-opacity: var(--cros-sys-opacity-disabled);
      --md-suggestion-chip-disabled-outline-color: var(--cros-sys-separator);
      --md-suggestion-chip-disabled-outline-opacity: var(--cros-sys-opacity-disabled);
      --md-suggestion-chip-focus-label-text-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-focus-leading-icon-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-hover-label-text-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-hover-leading-icon-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-suggestion-chip-hover-state-layer-opacity: 1;
      --md-suggestion-chip-icon-size: ${ICON_SIZE};
      --md-suggestion-chip-label-text-font: var(--cros-button-1-font-family);
      --md-suggestion-chip-label-text-size: var(--cros-button-1-font-size);
      --md-suggestion-chip-label-text-line-height: var(--cros-button-1-line-height);
      --md-suggestion-chip-label-text-weight: var(--cros-button-1-font-weight);
      --md-suggestion-chip-label-text-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-leading-icon-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-leading-space: ${CHIP_PADDING};
      --md-suggestion-chip-outline-color: var(--cros-sys-separator);
      --md-suggestion-chip-pressed-label-text-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-pressed-leading-icon-color: var(--cros-sys-on_surface);
      --md-suggestion-chip-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-suggestion-chip-pressed-state-layer-opacity: 1;
      --md-suggestion-chip-trailing-space: ${CHIP_PADDING};
    }

    md-input-chip {
      --md-focus-ring-duration: 0s;
      --md-input-chip-disabled-label-text-color: var(--cros-sys-on_surface);
      --md-input-chip-disabled-label-text-opacity: var(--cros-sys-opacity-disabled);
      --md-input-chip-disabled-leading-icon-color: var(--cros-sys-disabled);
      --md-input-chip-disabled-leading-icon-opacity: var(--cros-sys-opacity-disabled);
      --md-input-chip-disabled-outline-color: var(--cros-sys-separator);
      --md-input-chip-disabled-outline-opacity: var(--cros-sys-opacity-disabled);
      --md-input-chip-focus-label-text-color: var(--cros-sys-on_surface);
      --md-input-chip-focus-leading-icon-color: var(--cros-sys-on_surface);
      --md-input-chip-focus-trailing-icon-color: var(--cros-sys-on_surface);
      --md-input-chip-hover-label-text-color: var(--cros-sys-on_surface);
      --md-input-chip-hover-leading-icon-color: var(--cros-sys-on_surface);
      --md-input-chip-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-input-chip-hover-state-layer-opacity: 1;
      --md-input-chip-hover-trailing-icon-color: var(--cros-sys-on_surface);
      --md-input-chip-icon-size: ${ICON_SIZE};
      --md-input-chip-label-text-font: var(--cros-button-1-font-family);
      --md-input-chip-label-text-size: var(--cros-button-1-font-size);
      --md-input-chip-label-text-line-height: var(--cros-button-1-line-height);
      --md-input-chip-label-text-weight: var(--cros-button-1-font-weight);
      --md-input-chip-label-text-color: var(--cros-sys-on_surface);
      --md-input-chip-leading-icon-color: var(--cros-sys-on_surface);
      --md-input-chip-leading-space: ${CHIP_PADDING};
      --md-input-chip-outline-color: var(--cros-sys-separator);
      --md-input-chip-pressed-label-text-color: var(--cros-sys-on_surface);
      --md-input-chip-pressed-leading-icon-color: var(--cros-sys-on_surface);
      --md-input-chip-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-input-chip-pressed-state-layer-opacity: 1;
      --md-input-chip-pressed-trailing-icon-color: var(--cros-sys-on_surface);
      --md-input-chip-trailing-space: ${CHIP_PADDING};
    }

    md-filter-chip,
    md-suggestion-chip,
    md-input-chip {
      max-width: 100%;
    }

    :host([avatar]) md-input-chip {
      --md-input-chip-container-shape: 9999px;
    }

    md-filter-chip::part(focus-ring),
    md-filter-chip::part(trailing-focus-ring),
    md-suggestion-chip::part(focus-ring),
    md-input-chip::part(focus-ring),
    md-input-chip::part(trailing-focus-ring) {
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-width: ${FOCUS_RING_WIDTH};
    }
    #close-icon {
      fill: var(--cros-sys-on_surface);
      height: ${ICON_SIZE};
      width: ${ICON_SIZE};
    }
    #checked-icon {
      height: ${ICON_SIZE};
      width: ${ICON_SIZE};
    }
    :host([type="filter"]) #checked-icon {
      fill: var(--cros-sys-on_surface);
    }

    .label-wrapper {
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }
  `;

  /** @nocollapse */
  static override shadowRootOptions:
      ShadowRootInit = {mode: 'open', delegatesFocus: true};

  /** @nocollapse */
  static override properties = {
    disabled: {type: Boolean, reflect: true},
    selected: {type: Boolean},
    label: {type: String},
    type: {type: String},
    trailingIcon: {type: Boolean},
    avatar: {type: Boolean},
    showTooltipWhenTruncated:
        {type: Boolean, attribute: 'show-tooltip-when-truncated'},
  };

  /** @export */
  set selected(value) {
    // Selected only makes sense for filter chips.
    if (this.type !== 'filter') {
      return;
    }
    // Set may be called before shadowroot exists - await updateComplete to
    // ensure shadowroot exists.
    this.updateComplete.then(
        () => this.shadowRoot!.querySelector('md-filter-chip')!.selected =
            value);
    // No need to requestupdate since this is a property of the child.
  }

  /** @export */
  get selected() {
    // Update needs to use this to calculate the old value even when shadowroot
    // doesn't exist.
    return this.shadowRoot?.querySelector('md-filter-chip')?.selected ?? false;
  }

  /**
   * This ResizeObserver is used to detect when the chip is resized and update
   * the tooltip. The ResizeObserver is only active when
   * `showTooltipWhenTruncated` is true.
   */
  private resizeObserver: ResizeObserver|null = null;

  /**
   * Used to keep track of the timeout that debounces the ResizeObserver
   * callbacks (to improve performance).
   */
  private resizeObserverDebounceTimeout: number = -1;

  /**
   * Stops event propagation when the chip is disabled to ensure subsequent
   * handlers are not called.
   */
  private onClick = (e: MouseEvent) => {
    if (this.disabled) {
      e.stopImmediatePropagation();
      e.preventDefault();
      return;
    }
  };

  constructor() {
    super();
    this.disabled = false;
    this.selected = false;
    this.type = 'filter';
    this.trailingIcon = false;
    this.label = '';
    this.avatar = false;
    this.showTooltipWhenTruncated = false;

    // Ensure onClick is first click listener called.
    this.addEventListener('click', this.onClick);
  }

  override firstUpdated() {
    if (this.showTooltipWhenTruncated) {
      this.resizeObserver = new ResizeObserver(this.onChipResized.bind(this));
      this.resizeObserver.observe(this);
    }
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    if (this.resizeObserver) {
      this.resizeObserver.disconnect();
    }
    clearTimeout(this.resizeObserverDebounceTimeout);
  }

  override render() {
    if (this.type === 'filter') {
      return this.renderWrapperWithMaybeTooltip(html`
        <md-filter-chip
            ?disabled=${this.disabled}
            ?selected=${this.selected}>
          <slot slot='icon' name='icon'></slot>
          ${this.renderLabel()}
          ${checkedIcon}
        </md-filter-chip>
        `);
    }
    if (this.trailingIcon || this.avatar) {
      return this.renderWrapperWithMaybeTooltip(html`
        <md-input-chip
          ?disabled=${this.disabled}
          ?avatar=${this.avatar}>
            <slot slot='icon' name='icon'></slot>
            ${this.renderLabel()}
            ${closeIcon}
          </md-input-chip>
        `);
    }
    return this.renderWrapperWithMaybeTooltip(html`
      <md-suggestion-chip ?disabled=${this.disabled}>
        <slot slot='icon' name='icon'></slot>
        ${this.renderLabel()}
      </md-suggestion-chip>
      `);
  }

  private renderWrapperWithMaybeTooltip(content: TemplateResult) {
    if (!this.showTooltipWhenTruncated) {
      return content;
    }

    return html`
      <div id="chip-wrapper">
        ${content}
        <cros-tooltip
            anchor="chip-wrapper"
            label=${this.label}
            ?hidden=${!this.isLabelTruncated()}>
        </cros-tooltip>
      </div>
      `;
  }

  private renderLabel() {
    return html`<div class="label-wrapper">${this.label}</div>`;
  }

  onChipResized() {
    // If there is already a timeout, clear it to reset the debounce timer.
    clearTimeout(this.resizeObserverDebounceTimeout);

    this.resizeObserverDebounceTimeout = setTimeout(() => {
      // Request an update so that the component will check if the label is
      // truncated.
      this.requestUpdate();
    }, RESIZE_OBSERVER_DEBOUNCE_TIMEOUT_MS);
  }

  isLabelTruncated(): boolean {
    if (!this.renderRoot) {
      return false;
    }
    const labelWrapper =
        this.renderRoot.querySelector<HTMLElement>('.label-wrapper');
    if (!labelWrapper) {
      return false;
    }
    return labelWrapper.scrollWidth > labelWrapper.clientWidth;
  }
}

customElements.define('cros-chip', Chip);

declare global {
  interface HTMLElementTagNameMap {
    'cros-chip': Chip;
  }
}
