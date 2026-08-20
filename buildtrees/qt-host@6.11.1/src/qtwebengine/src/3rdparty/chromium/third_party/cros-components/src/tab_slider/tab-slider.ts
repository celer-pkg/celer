/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {css, CSSResultGroup, html, LitElement, nothing} from 'lit';

import {isRTL, shadowPiercingActiveElement} from '../helpers/helpers';

import {isTabSliderItem, TabSliderItem, TabSliderItemSelectedChangedEvent, TabSliderItemTriggeredEvent} from './tab-slider-item';

/**
 * The padding between `cros-tab-slider` and `cros-tab-slider-item`s.
 */
export const TAB_SLIDER_PADDING_PX = 2;

const OBSERVE_ITEMS = {
  attributes: true,
  attributeFilter: ['disabled'],
  subtree: true,
  childList: true,
};

function clamp(value: number, min: number, max: number): number {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}

/**
 * A ChromeOS compliant tab-slider.
 */
export class TabSlider extends LitElement {
  private get itemSlot(): HTMLSlotElement {
    return this.shadowRoot!.getElementById('items') as HTMLSlotElement;
  }

  get items(): TabSliderItem[] {
    return this.itemSlot.assignedElements().filter(isTabSliderItem);
  }

  get selectableItems(): TabSliderItem[] {
    return this.items.filter(item => !item.disabled);
  }

  get selectedItem(): TabSliderItem|null {
    return this.selectableItems.find(item => item.selected) || null;
  }

  get selectionIndicator(): HTMLDivElement {
    return this.shadowRoot!.getElementById('selectionIndicator') as
        HTMLDivElement;
  }

  /**
   * The index of the item the indicator is highlighting.
   * Public for testing. Do not use externally.
   */
  indicatorIndex = -1;

  private readonly attributeObserver = new MutationObserver(() => {
    this.updateIndicator(false);
    this.ensureFocusable();
  });

  private readonly resizeObserver = new ResizeObserver(() => {
    this.updateIndicator(false);
  });

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      background: rgba(var(--cros-sys-surface_variant-rgb), 0.8);
      border-radius: 100vh;
      display: inline-block;
      font: var(--cros-body-0-font);
      padding: ${TAB_SLIDER_PADDING_PX}px;
    }

    :host > div {
      position: relative;
    }

    #items {
      display: grid;
      grid-auto-columns: 1fr;
      grid-auto-flow: column;
      width: 100%;
    }

    #items::slotted(cros-tab-slider-item) {
      width: 100%;
      z-index: 2;
    }

    #selectionIndicator {
      background: var(--cros-sys-primary);
      border-radius: 100vh;
      height: 40px;
      width: 100px;
      inset: 0;
      z-index: 1;
    }

    #indicatorTrack {
      position: absolute;
      top: 0;
    }

    :host-context([dir="rtl"]) #indicatorTrack {
      transform: scaleX(-1);
    }

    #items::slotted(cros-tab-slider-item[disabled]) {
      display: none;
    }
  `;

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };

  /** @nocollapse */
  static override properties = {
    ariaLabel: {type: String, attribute: 'aria-label'},
  };

  override render() {
    return html`
      <div
          aria-label=${this.ariaLabel || nothing}
          id="container"
          role="tablist">
        <slot
            id="items"
            @cros-tab-slider-item-selected-changed=${this.onItemSelectedChanged}
            @cros-tab-slider-item-triggered=${this.onItemTriggered}
            @slotchange=${this.onSlotChanged}>
        </slot>
        <div id="indicatorTrack">
          <div id="selectionIndicator"></div>
        </div>
      </div>
    `;
  }

  override connectedCallback() {
    super.connectedCallback();
    this.addEventListener('keydown', this.onKeyDown);
    this.attributeObserver.observe(this, OBSERVE_ITEMS);
    this.resizeObserver.observe(this);
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    this.attributeObserver.disconnect();
    this.resizeObserver.disconnect();
  }

  override firstUpdated() {
    this.ensureFocusable();
  }

  override focus() {
    this.selectableItems.filter(i => i.tabIndex >= 0)[0]?.focus();
  }
  itemToFocus: TabSliderItem|null|undefined = null;

  private onItemTriggered(event: TabSliderItemTriggeredEvent) {
    this.setSelected(event.detail.item);
  }

  private onItemSelectedChanged(event: TabSliderItemSelectedChangedEvent) {
    if (event.detail.item.selected) {
      this.setSelected(event.detail.item);
    } else {
      this.updateIndicator();
    }
  }

  private onSlotChanged() {
    this.ensureFocusable();
    this.updateIndicator(false);
  }

  /** Ensure there is at least one item to delegate focus to. */
  private ensureFocusable() {
    // Focus should jump to the selected item.
    for (const item of this.items) {
      item.tabIndex = item.selected ? 0 : -1;
    }

    const selectedItem = this.selectedItem;
    if (!selectedItem) {
      const first = this.selectableItems[0];
      if (first) first.tabIndex = 0;
    }
  }

  /**
   * Sets `item` to be the selected item, unselected other items and plays the
   * transition animation.
   */
  setSelected(itemToSelect: TabSliderItem) {
    // Undo invalid selection.
    if (itemToSelect.disabled) {
      itemToSelect.selected = false;
      itemToSelect.tabIndex = -1;
      return;
    }

    // Unselect old items.
    for (const item of this.items) {
      if (item !== itemToSelect) {
        item.selected = false;
        item.tabIndex = -1;
      }
    }

    itemToSelect.selected = true;
    itemToSelect.tabIndex = 0;
    this.updateIndicator();
  }

  /**
   * Resizes and positions the indicator behind the active item. The sliding
   * animation should be skipped if the slider isn't moving between items.
   */
  private updateIndicator(animate?: boolean) {
    const selectedItem = this.selectedItem;
    const selectedIndex =
        selectedItem ? this.selectableItems.indexOf(selectedItem) : -1;

    if (animate === undefined) {
      // Don't animate the inidicator if it is coming or going off-screen.
      animate = this.indicatorIndex !== -1 && selectedIndex !== -1;
    }

    this.indicatorIndex = selectedIndex;
    const indicatorWidth = selectedItem?.getBoundingClientRect()?.width || 0;
    const indicatorLeft = selectedItem ?
        selectedItem.getBoundingClientRect().left -
            this.selectableItems[0]!.getBoundingClientRect().left :
        0;

    this.selectionIndicator.style.transition = animate ? '200ms' : 'none';
    this.selectionIndicator.style.transform = `translateX(${indicatorLeft}px)`;
    this.selectionIndicator.style.width = `${indicatorWidth}px`;
  }

  /**
   * Handle the keydown. This mainly handles the navigation and the selection
   * with the keyboard.
   */
  private onKeyDown(e: KeyboardEvent) {
    const selectableItems = this.selectableItems;
    const focusedItem = shadowPiercingActiveElement('cros-tab-slider-item');
    const currentIndex = selectableItems.findIndex(i => i === focusedItem);

    if (e.ctrlKey) {
      return;
    }

    if (currentIndex === -1) {
      return;
    }

    let nextIndex = currentIndex;
    switch (e.key) {
      case 'ArrowLeft':
      case 'ArrowRight':
        // Don't let back/forward keyboard shortcuts be used.
        if (e.altKey) {
          break;
        }

        nextIndex += e.key === 'ArrowRight' === isRTL(this) ? -1 : 1;
        nextIndex = clamp(nextIndex, 0, selectableItems.length - 1);
        break;
      case 'Home':
        nextIndex = 0;
        break;
      case 'End':
        nextIndex = selectableItems.length - 1;
        break;
      default:
        break;
    }

    if (nextIndex !== currentIndex) {
      selectableItems[nextIndex]!.focus();
      e.preventDefault();
    }
  }

  override async getUpdateComplete() {
    const result = await super.getUpdateComplete();
    await Promise.all(
        this.items.map((item: TabSliderItem) => item.updateComplete));
    return result;
  }
}

customElements.define('cros-tab-slider', TabSlider);

declare global {
  interface HTMLElementTagNameMap {
    'cros-tab-slider': TabSlider;
  }
}
