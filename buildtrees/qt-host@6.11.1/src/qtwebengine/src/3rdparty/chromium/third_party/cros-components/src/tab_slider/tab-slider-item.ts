/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/ripple/ripple.js';
import '@material/web/focus/md-focus-ring.js';

import {css, CSSResultGroup, html, LitElement, nothing, PropertyValues} from 'lit';

/** Type of the item triggered custom event. */
export type TabSliderItemTriggeredEvent = CustomEvent<{
  /** The item which has been triggered. */
  item: TabSliderItem,
}>;

/** Type of the item.selected changed custom event. */
export type TabSliderItemSelectedChangedEvent = CustomEvent<{
  /** The item which has been changed. */
  item: TabSliderItem,
}>;

/** Check if an `Element` is a sidenav item or not. */
export function isTabSliderItem(element: unknown): element is TabSliderItem {
  return element instanceof Element &&
      element.tagName === 'CROS-TAB-SLIDER-ITEM';
}

/** A chromeOS compliant tab-slider. */
export class TabSliderItem extends LitElement {
  /**
   * TabSliderItem doesn't set its own background, because the parent TabSlider
   * needs to control it to create the sliding animation.
   * @nocollapse
   */
  static override styles: CSSResultGroup = css`
    :host {
      border-radius: 100vh;
      display: inline-block;
      font: var(--cros-button-2-font);
      height: fit-content;
      position: relative;
      text-align: center;
      width: fit-content;
    }

    #container {
      align-items: center;
      background: transparent;
      border: none;
      border-radius: 100vh;
      color: var(--cros-sys-on_surface_variant);
      font: inherit;
      height: 40px;
      justify-content: center;
      padding-inline-start: 16px;
      padding-inline-end: 16px;
      position: relative;
      text-align: center;
      width: 100%;
    }

    :host([selected]) #container {
      color: var(--cros-sys-on_primary);
    }

    #container:focus-visible {
      outline: none;
    }

    :host(:not([selected])) #container:hover {
      color: var(--cros-sys-on_surface);
    }

    md-focus-ring {
      animation-duration: 0s;
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-width: 2px;
    }

    md-ripple {
      display: none;
    }

    :host([selected]:active) md-ripple {
      display: unset;
      --md-ripple-hover-color: var(--cros-sys-ripple_primary);
      --md-ripple-hover-opacity: 1;
      --md-ripple-pressed-color: var(--cros-sys-ripple_primary);
      --md-ripple-pressed-opacity: 1;
    }

    #container {
      display: flex;
    }

    #container > div {
      display: flex;
      flex-direction: row;
      gap: 8px;
    }

    slot[name="icon"]::slotted(*) {
      height: 20px;
      width: 20px;
    }
  `;

  /** @export */
  declare label: string;

  /** @export */
  declare selected: boolean;

  /** @export */
  declare disabled: boolean;

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };

  /** @nocollapse */
  static override properties = {
    label: {type: String, reflect: true},
    selected: {type: Boolean, reflect: true},
    disabled: {type: Boolean, reflect: true},
    tabIndex: {type: Number},
  };

  /** @nocollapse */
  static events = {
    /** Triggers when the `selected` property changes. */
    TAB_SLIDER_ITEM_SELECTED_CHANGED: 'cros-tab-slider-item-selected-changed',
    /** Triggers when an item is clicked or equivalent. */
    TAB_SLIDER_ITEM_TRIGGERED: 'cros-tab-slider-item-triggered',
  } as const;

  constructor() {
    super();
    this.label = 'Item';
    this.selected = false;
    this.disabled = false;
  }

  override render() {
    return html`
          <button
              aria-selected=${this.selected}
              id="container"
              role="tab"
              tabindex=${this.tabIndex ?? nothing}>
            <div>
              <slot name="icon" aria-hidden="true"></slot>
              <span>${this.label}</span>
            </div>
            <md-ripple
                for="container"
                ?disabled=${!this.selected || this.disabled}>
            </md-ripple>
            <md-focus-ring
                ?disabled=${this.disabled}
                for="container">
            </md-focus-ring>
          </button>
    `;
  }

  override connectedCallback() {
    super.connectedCallback();
    this.addEventListener('click', () => {
      this.dispatchEvent(new CustomEvent(
          TabSliderItem.events.TAB_SLIDER_ITEM_TRIGGERED,
          {bubbles: true, detail: {item: this}}));
    });
  }

  override updated(changedProperties: PropertyValues<this>) {
    super.updated(changedProperties);
    if (changedProperties.has('selected')) {
      this.onSelectedChanged();
    }
  }

  private onSelectedChanged() {
    this.dispatchEvent(new CustomEvent(
        TabSliderItem.events.TAB_SLIDER_ITEM_SELECTED_CHANGED,
        {bubbles: true, detail: {item: this}}));
  }
}

customElements.define('cros-tab-slider-item', TabSliderItem);

declare global {
  interface HTMLElementTagNameMap {
    'cros-tab-slider-item': TabSliderItem;
  }

  interface HTMLElementEventMap {
    [TabSliderItem.events.TAB_SLIDER_ITEM_TRIGGERED]:
        TabSliderItemTriggeredEvent;
    [TabSliderItem.events.TAB_SLIDER_ITEM_SELECTED_CHANGED]:
        TabSliderItemSelectedChangedEvent;
  }
}
