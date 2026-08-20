/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {SelectOption} from '@material/web/select/select-option.js';
import {css} from 'lit';

import {MenuItem} from '../menu/menu_item';

/**
 * A chromeOS compliant dropdown-option to use within dropdown.
 */
export class DropdownOption extends MenuItem implements SelectOption {
  static override styles = [
    MenuItem.styles, css`
      md-menu-item {
        --md-menu-item-label-text-font: var(--cros-dropdown-option-text-font, var(--cros-button-2-font-family));
      }
    `
  ];

  /** @nocollapse */
  static override properties = {
    ...MenuItem.properties,
    value: {type: String, reflect: true},
  };

  private internalValue: string|null = null;

  constructor() {
    super();

    this.type = 'option';
  }

  /**
   * The value of the dropdown option. If unsupplied defaults to `headline`.
   * @export
   */
  get value() {
    if (this.internalValue !== null) {
      return this.internalValue;
    }
    return this.headline;
  }
  set value(value: string) {
    this.internalValue = value;
  }

  // When extending a lit element, it seems that getters and setters get
  // clobbered. To avoid this we specifically reimplement typeaheadText and
  // selected getters/setters below to ensure they function correctly on
  // cros-dropdown-option. These should be identical to the functions they
  // override in cros-menu-item.
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

  // SelectOption implementation:
  get displayText() {
    return this.headline;
  }
}

customElements.define('cros-dropdown-option', DropdownOption);

declare global {
  interface HTMLElementTagNameMap {
    'cros-dropdown-option': DropdownOption;
  }
}
