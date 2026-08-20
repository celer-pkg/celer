/**
 * @license
 * Copyright 2024 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {SelectOption} from '@material/web/select/select-option.js';
import {css, PropertyValues} from 'lit';

import {MenuItem} from '../menu/menu_item';

/**
 * Event type for when an option is triggered.
 * @export
 */
export type OptionTriggeredEvent = CustomEvent<{menuItem: IconDropdownOption}>;

/** A chromeOS compliant option to be used in cros-icon-dropdown. */
export class IconDropdownOption extends MenuItem implements SelectOption {
  /** @nocollapse */
  static override shadowRootOptions = {
    ...MenuItem.shadowRootOptions,
  };

  /** @nocollapse */
  static override styles = [
    MenuItem.styles, css`
      md-menu-item {
        --md-menu-item-label-text-font: var(--cros-icon-dropdown-option-text-font, var(--cros-button-2-font-family));
      }
    `
  ];

  /** @nocollapse */
  static override properties = {
    ...MenuItem.properties,
    value: {type: String, reflect: true},
  };

  /** @nocollapse */
  static override events = {
    ...MenuItem.events,
    OPTION_TRIGGERED: 'cros-icon-dropdown-option-triggered',
  } as const;

  /**
   * Internal value associated with the option.
   */
  private internalValue: string;

  get value() {
    if (this.internalValue === '') {
      return this.headline;
    }

    return this.internalValue;
  }

  set value(value: string) {
    this.internalValue = value;
  }

  // When extending a lit element, it seems that getters and setters get
  // clobbered. To avoid this we specifically reimplement needed getters/setters
  // below to ensure they function correctly on cros-icon-dropdown-option. These
  // should be identical to the functions they override in cros-menu-item.
  override get selected() {
    return this.renderRoot?.querySelector('md-menu-item')?.selected ?? false;
  }

  override set selected(selected: boolean) {
    const item = this.renderRoot?.querySelector('md-menu-item');
    if (!item) {
      this.missedPropertySets.selected = selected;
    } else {
      item.selected = selected;
    }
  }

  override get switchSelected() {
    const crosSwitch = this.renderRoot?.querySelector('cros-switch');
    if (crosSwitch) {
      return crosSwitch.selected;
    }
    return this.missedPropertySets.switchSelected ?? false;
  }

  override set switchSelected(value: boolean) {
    const crosSwitch = this.renderRoot?.querySelector('cros-switch');
    if (!crosSwitch) {
      this.missedPropertySets.switchSelected = value;
    } else {
      crosSwitch.selected = value;
    }
  }

  override get typeaheadText() {
    return this.renderRoot?.querySelector('md-menu-item')?.typeaheadText ?? '';
  }

  override set typeaheadText(text: string) {
    const item = this.renderRoot?.querySelector('md-menu-item');
    if (!item) {
      this.missedPropertySets.typeaheadText = text;
    } else {
      item.typeaheadText = text;
    }
  }

  // SelectOption implementation:
  get displayText() {
    return this.headline;
  }

  constructor() {
    super();

    // Default type to 'option' to match cros-icon-dropdown default menutype of
    // 'listbox'.
    this.type = 'option';
    this.internalValue = '';
    this.keepOpen = false;
  }

  override connectedCallback() {
    super.connectedCallback();
    this.addEventListener('click', this.onClickHandler);
    this.addEventListener('keydown', this.onKeyDownHandler);
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    this.removeEventListener('click', this.onClickHandler);
    this.removeEventListener('keydown', this.onKeyDownHandler);
  }

  override updated(changedProperties: PropertyValues<this>) {
    super.updated(changedProperties);
    if (changedProperties.has('checked')) {
      const ariaSelected = this.checked ? 'true' : 'false';
      this.renderRoot?.querySelector('md-menu-item')
          ?.setAttribute('aria-selected', ariaSelected);
    }
  }

  private onClickHandler = () => {
    this.onItemTriggered();
  };

  private onKeyDownHandler = (e: KeyboardEvent) => {
    if (e.key === 'Enter' || e.key === ' ') {
      this.onItemTriggered();
    }
  };

  // Notifies the parent icon-dropdown that this option was triggered.
  protected onItemTriggered() {
    // If the itemEnd is "switch" then do not notify the parent icon-dropdown
    // that this option was triggered.
    if (this.itemEnd === 'switch') {
      return;
    }
    this.checked = true;
    this.dispatchEvent(
        new CustomEvent(IconDropdownOption.events.OPTION_TRIGGERED, {
          bubbles: true,
          composed: true,
          detail: {menuItem: this},
        }));
  }
}

customElements.define('cros-icon-dropdown-option', IconDropdownOption);

declare global {
  interface HTMLElementEventMap {
    [IconDropdownOption.events.OPTION_TRIGGERED]: OptionTriggeredEvent;
  }

  interface HTMLElementTagNameMap {
    'cros-icon-dropdown-option': IconDropdownOption;
  }
}
