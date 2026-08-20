/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/slider/slider.js';

import {MdSlider} from '@material/web/slider/slider.js';
import {css, CSSResultGroup, html, LitElement, nothing, PropertyValues} from 'lit';



// TODO(b/285172083) remove unsupported disabled state rendering/styling
// tslint:disable:no-any needed for JS interface
const renderTrack = (MdSlider.prototype as any).renderTrack;
(MdSlider.prototype as any).renderTrack = function() {
  return html`<slot name="track">${renderTrack.call(this)}</slot>`;
};

/**
 * Styles needed to achieve transparent border around handle for disabled state.
 * Border radius for upper and lower ensures the track is rounded at the end but
 * not next to the handle.
 * Width is calculated given slider lower/upper fraction (the part of the
 * track that is currently active/inactive depending on value) minus the handle
 * gap (half of the handle width + width of transparent gap on one side).
 *
 * TODO: b/285172083 - do not use md-slider's `--_private` custom properties
 */
const DISABLED_STATE_OVERRIDES = css`
  md-slider[disabled] > [slot="track"] {
    display: flex;
    align-items: center;
    position: absolute;
    inset: 0 calc(var(--md-slider-state-layer-size) / 2);
    --handle-margin: 2px;
    --handle-gap: calc(var(--handle-margin) + (var(--md-slider-handle-width) / 2));
  }
  .lower, .upper {
    position: absolute;
    height: 4px;
    background: var(--disabled-color);
  }
  .lower {
      border-radius: 9999px 0 0 9999px;
      left: 0;
      width: calc((var(--_end-fraction)) * 100% - var(--handle-gap));
  }
  .upper {
      border-radius: 0 9999px 9999px 0;
      right: 0;
      width: calc((1 - var(--_end-fraction)) * 100% - var(--handle-gap));
  }
`;

/**
 * A ChromeOS compliant slider component.
 */
export class Slider extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = [
    css`
    :host {
      display: inline-block;
      min-inline-size: 200px;
    }
    md-slider {
      display: block;
      min-inline-size: inherit;
      --active-disabled: var(--cros-sys-on_surface);
      --inactive-disabled: var(--cros-sys-on_surface);
      --disabled-color: var(--inactive-disabled);
      --md-focus-ring-duration: 0s;
      --md-slider-active-track-color: var(--cros-sys-primary);
      --md-slider-disabled-active-track-color: var(--disabled-color);
      --md-slider-disabled-active-track-opacity: var(--cros-disabled-opacity);
      --md-slider-disabled-handle-color: var(--disabled-color);
      --md-slider-disabled-inactive-track-color: var(--disabled-color);
      --md-slider-disabled-inactive-track-opacity: var(--cros-disabled-opacity);
      --md-slider-focus-handle-color: var(--cros-sys-primary);
      --md-slider-handle-color: var(--cros-sys-primary);
      --md-slider-handle-height: 12px;
      --md-slider-handle-width: 12px;
      --md-slider-hover-handle-color: var(--cros-sys-primary);
      --md-slider-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-slider-hover-state-layer-opacity: 1;
      --md-slider-inactive-track-color: var(--cros-sys-highlight_shape);
      --md-slider-label-container-color: var(--cros-sys-primary);
      --md-slider-label-container-height: 18px;
      --md-slider-label-text-color: var(--cros-sys-on_primary);
      --md-slider-label-text-font: var(--cros-label-1-font-family);
      --md-slider-label-text-size: var(--cros-label-1-font-size);
      --md-slider-label-text-line-height: var(--cros-label-1-line-height);
      --md-slider-label-text-weight: var(--cros-label-1-font-weight);
      --md-slider-pressed-handle-color: var(--cros-sys-primary);
      --md-slider-pressed-state-layer-color: var(--cros-sys-ripple_primary);
      --md-slider-pressed-state-layer-opacity: 1;
      --md-slider-state-layer-size: 28px;
      --md-slider-with-tick-marks-active-container-color: var(--cros-sys-on_primary);
      --md-slider-with-tick-marks-disabled-container-color: var(--disabled-color);
      --md-slider-with-tick-marks-inactive-container-color: var(--cros-sys-primary);
    }

    md-slider::part(focus-ring) {
      background: var(--cros-sys-ripple_primary);
      height: 28px;
      inset: unset;
      width: 28px;
      --md-focus-ring-active-width: 2px;
      --md-focus-ring-color: var(--cros-sys-primary);
      --md-focus-ring-width: 2px;
    }
  `,
    DISABLED_STATE_OVERRIDES
  ];

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };

  /** @nocollapse */
  static override properties = {
    ariaLabel: {type: String, reflect: true, attribute: 'aria-label'},
    ariaValueText: {type: String, reflect: true, attribute: 'aria-valuetext'},
    value: {type: Number, reflect: true},
    disabled: {type: Boolean},
    withTickMarks: {type: Boolean},
    withLabel: {type: Boolean},
    min: {type: Number},
    max: {type: Number},
    step: {type: Number},
  };

  /** @export */
  declare value: number;
  /** @export */
  declare disabled: boolean;
  /** @export */
  declare withTickMarks: boolean;
  /** @export */
  declare withLabel: boolean;
  /** @export */
  declare min: number;
  /** @export */
  declare max: number;
  /** @export */
  declare step: number;


  constructor() {
    super();
    this.ariaLabel = '';
    this.ariaValueText = '';
    this.value = 0;
    this.disabled = false;
    this.withTickMarks = false;
    this.withLabel = false;
    this.min = 0;
    this.max = 10;
    this.step = 1;
  }

  override render() {
    // Using unicode non-breaking space U-00A0, charCode 160. This is to add
    // padding on either side of the label text.
    const space = `  `;
    const valueLabel = `${space}${this.ariaValueText || this.value}${space}`;
    const disabledTemplate = this.disabled ? html`
      <div slot="track">
        <div class="lower"></div>
        <div class="upper"></div>
      </div>
      ` :
                                             nothing;
    return html`
      <md-slider
        @change=${this.handleChange}
        @input=${this.handleInput}
        ?disabled=${this.disabled}
        aria-label=${this.ariaLabel || ''}
        .labeled=${this.withLabel}
        .min=${this.min}
        .max=${this.max}
        .step=${this.step}
        .ticks=${this.withTickMarks}
        .value=${this.value}
        .valueLabel=${valueLabel}>
        ${disabledTemplate}
      </md-slider>`;
  }

  override updated(changedProperties: PropertyValues<this>) {
    if (changedProperties.has('disabled')) {
      // Work around for b/315384008.
      this.renderRoot.querySelector('md-slider')?.requestUpdate();
    }
  }

  private handleChange() {
    // Md-slider's change event won't exit our shadow DOM, redispatch it to
    // ensure clients can listen to it.
    this.dispatchEvent(new Event('change', {bubbles: true}));
  }

  private handleInput(e: Event) {
    const sliderValue = (e.target as MdSlider).value;
    if (sliderValue !== undefined) {
      this.value = sliderValue;
    }
  }
}

customElements.define('cros-slider', Slider);

declare global {
  interface HTMLElementTagNameMap {
    'cros-slider': Slider;
  }
}
