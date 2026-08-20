/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/menu/menu-item.js';
import '../switch/switch';

import {CloseMenuEvent, MenuItem as MenuItemType} from '@material/web/menu/menu-item.js';
import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';
import {ifDefined} from 'lit/directives/if-defined';

const CHECKED_ICON = html`
  <svg width="20" height="20" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg">
    <path d="M13.7071 7.29289C13.3166 6.90237 12.6834 6.90237 12.2929 7.29289L9 10.5858L7.70711 9.29289C7.31658 8.90237 6.68342 8.90237 6.29289 9.29289C5.90237 9.68342 5.90237 10.3166 6.29289 10.7071L8.29289 12.7071C8.68342 13.0976 9.31658 13.0976 9.70711 12.7071L13.7071 8.70711C14.0976 8.31658 14.0976 7.68342 13.7071 7.29289Z"/>
    <path fill-rule="evenodd" clip-rule="evenodd" d="M10 18C14.4183 18 18 14.4183 18 10C18 5.58172 14.4183 2 10 2C5.58172 2 2 5.58172 2 10C2 14.4183 5.58172 18 10 18ZM10 16C13.3137 16 16 13.3137 16 10C16 6.68629 13.3137 4 10 4C6.68629 4 4 6.68629 4 10C4 13.3137 6.68629 16 10 16Z"/>
  </svg>
`;

/**
 * Fired when the user clicks on or otherwise activates the menu
 * item i.e. via a enter or space key.
 */
export type MenuItemTriggeredEvent = CustomEvent<{
  menuItem: MenuItem,
}>;

// Lit analyzer has stricter types then the actual typesceipt lib types, as such
// we need to cast our strings to this type before setting aria-has-popup to
// avoid lit analyzer raising a warning.
type AriaHasPopupValue = 'false'|'true'|'menu'|'listbox'|'tree'|'grid'|'dialog';

/**
 * A cros compliant menu-item component for use in cros-menu.
 */
export class MenuItem extends LitElement implements MenuItemType {
  // TODO: b/198759625 - Remove negative margin-inline-end when mwc token
  // available (padding 16px)
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    md-menu-item {
      --md-menu-item-disabled-label-text-opacity: var(--cros-sys-opacity-disabled);
      --md-menu-item-disabled-leading-icon-opacity: var(--cros-sys-opacity-disabled);
      --md-menu-item-disabled-trailing-icon-opacity: var(--cros-sys-opacity-disabled);
      --md-menu-item-disabled-label-text-color: var(--cros-sys-on_surface);
      --md-menu-item-focus-label-text-color: var(--cros-sys-on_surface);
      --md-menu-item-hover-label-text-color: var(--cros-sys-on_surface);
      --md-menu-item-hover-state-layer-color: var(--cros-sys-hover_on_subtle);
      --md-menu-item-hover-state-layer-opacity: 1;
      --md-menu-item-label-text-color: var(--cros-sys-on_surface);
      --md-menu-item-label-text-font: var(--cros-button-2-font-family);
      --md-menu-item-label-text-size: var(--cros-button-2-font-size);
      --md-menu-item-label-text-line-height: var(--cros-button-2-line-height);
      --md-menu-item-label-text-weight: var(--cros-button-2-font-weight);
      --md-menu-item-leading-icon-color: var(--cros-sys-on_surface);
      --md-menu-item-pressed-label-text-color: var(--cros-sys-on_surface);
      --md-menu-item-pressed-state-layer-color: var(--cros-sys-ripple_neutral_on_subtle);
      --md-menu-item-pressed-state-layer-opacity: 1;
      --md-menu-item-trailing-element-headline-trailing-element-space: 48px;
      --md-menu-item-trailing-space: 16px;
      --md-menu-item-selected-container-color: var(--cros-sys-hover_on_subtle);
      --md-menu-item-selected-label-text-color: var(--cros-sys-on_surface);
    }
    md-menu-item([itemEnd=null]) {
      --md-menu-item-trailing-space: 48px;
    }
    :host([itemEnd="icon"]) md-menu-item {
      --md-menu-item-trailing-space: 12px;
    }
    :host([checked]) md-menu-item {
      --md-menu-item-trailing-space: 12px;
    }
    md-menu-item::part(focus-ring) {
      --md-focus-ring-active-width: 2px;
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-duration: 0s;
      --md-focus-ring-width: 2px;
    }

    :host([checked]) #checked-icon {
      display: inline-block;
      fill: var(--cros-sys-primary);
      height: 20px;
      width: 20px;
    }

    md-menu-item[selected]:hover::part(ripple) {
      --md-ripple-hover-opacity: 0;
    }

    ::slotted(:is(
      [slot="start"],
      [slot="end"],
    )) {
      color: var(--cros-sys-on_surface);
      height: 20px;
      width: 20px;
    }

    ::slotted(ea-icon) {
      --ea-icon-size: 20px;
    }

    cros-switch {
      height: 18px;
      width: 32px;
    }

    .slot-end:not(cros-switch) {
      font: var(--cros-button-2-font);
      width: unset;
      height: unset;
    }

    #shortcut {
      color: var(--cros-sys-secondary);
    }

    #headline {
      white-space: nowrap;
    }

    :host-context([dir=rtl]) ::slotted(:is(
      [slot="start"],
      [slot="end"]
    )) {
      transform: scaleX(-1);
    }

    :host-context([dir=rtl]) #checked-icon {
      transform: scaleX(1);
    }
  `;

  static override shadowRootOptions = {
    mode: 'open' as const,
    delegatesFocus: true,
  };

  /** @nocollapse */
  static override properties = {
    ariaHasPopup: {type: String, reflect: true, attribute: 'aria-haspopup'},
    ariaLabel: {type: String, reflect: true, attribute: 'aria-label'},
    headline: {type: String},
    itemStart: {type: String},
    itemEnd: {type: String},
    tabIndex: {type: Number, reflect: true},
    disabled: {type: Boolean},
    keepOpen: {type: Boolean, attribute: 'keep-open'},
    isMenuItem: {type: Boolean, reflect: true, attribute: 'md-menu-item'},
    checked: {type: Boolean, reflect: true},
    shortcutText: {type: String},
    typeaheadText: {type: String, attribute: 'typeahead-text'},
    selected: {type: Boolean},
    switchSelected: {type: Boolean},
    type: {type: String},
  };

  /** @nocollapse */
  static events = {
    /** Triggers when an item is clicked or equivalent. */
    MENU_ITEM_TRIGGERED: 'cros-menu-item-triggered',
  } as const;

  /**
   * Headline is the primary text of the list item, name follows from
   * md-menu-item.
   * @export
   */
  declare headline: string;
  /**
   * Item to be placed in the start slot if any. 'icon' exposes the start slot.
   * @export
   */
  declare itemStart: 'icon'|'';
  /**
   * Item to be placed in the end slot if any. Shortcut allows a text label (set
   * via `shortcutText`), switch uses cros-switch and 'icon' exposes the end
   * slot.
   * @export
   */
  declare itemEnd: 'shortcut'|'icon'|'switch'|'';

  /**
   * If true, keeps the menu open when clicked. (used by submenu)
   *
   * @export
   */
  declare keepOpen: boolean;
  // The following properties are necessary for wrapping and keyboard
  // navigation
  /**
   * Whether or not the menu-item is disabled.
   * @export
   */
  declare disabled: boolean;
  /** @export */
  declare readonly isMenuItem: boolean;
  /**
   * If true, this will result in a checkmark icon used in the end slot. This
   * overrides any other components in end slot if otherwise specified.
   * @export
   */
  declare checked: boolean;
  /**
   * The text that will appear in the end slot if menu-item is of `shortcut`
   * type. Defaults to 'Shortcut' if text is not given.
   * @export
   */
  declare shortcutText: string;
  /**
   * Sets the behavior and role of the menu item, defaults to "menuitem".
   */
  declare type: 'menuitem'|'option'|'button'|'link';

  /**
   * For properties menu-item proxies to a child element via a setter, any
   * attempt to set the value before firstUpdated() will cause the value to be
   * dropped since there is no md-menu-item to send the value to. To prevent
   * this we cache these values and set them on first render.
   */
  protected missedPropertySets: Partial<{
    selected: boolean,
    typeaheadText: string,
    switchSelected: boolean,
  }> = {};

  constructor() {
    super();

    this.ariaHasPopup = 'false';
    this.ariaLabel = '';
    this.headline = '';
    this.itemStart = '';
    this.itemEnd = '';

    this.keepOpen = false;
    this.disabled = false;
    this.isMenuItem = true;
    this.tabIndex = 0;
    this.checked = false;
    this.shortcutText = 'Shortcut';
    this.type = 'menuitem';
  }

  // Start slot should exist when `itemStart` is  `icon`.
  private get startSlot(): HTMLSlotElement|null {
    return this.renderRoot.querySelector('slot[name="start"]');
  }

  // End slot should exist when `itemEnd` is `icon`.
  private get endSlot(): HTMLSlotElement|null {
    return this.renderRoot.querySelector('slot[name="end"]');
  }

  get mdMenuItem() {
    return this.renderRoot?.querySelector('md-menu-item');
  }

  // List item is the interactive element of the menu item and thus the element
  // in focus by Chromevox. To ensure 'switch' announces correctly we need to
  // set attributes directly on the list item.
  get listItem() {
    return this.mdMenuItem?.renderRoot.querySelector('.list-item');
  }

  override updated(changedProperties: PropertyValues<this>) {
    super.updated(changedProperties);
    if (changedProperties.has('keepOpen') && this.keepOpen) {
      this.addEventListener('keydown', this.keyDownListener);
      this.addEventListener('click', this.clickListener);
    }
    if (changedProperties.has('itemEnd') && this.itemEnd === 'switch') {
      this.addEventListener('keydown', this.keyDownListener);
      this.addEventListener('click', this.clickListener);
    }
    if (changedProperties.has('itemStart') && this.itemStart === 'icon' &&
        this.startSlot) {
      const startItems = this.startSlot.assignedElements();
      startItems.forEach((startItem) => {
        startItem.setAttribute('slot', 'start');
      });
    }
    if (changedProperties.has('itemEnd') && this.itemEnd === 'icon' &&
        this.endSlot) {
      const endItems = this.endSlot.assignedElements();
      endItems.forEach((endItem) => {
        endItem.setAttribute('slot', 'end');
      });
    }

    // MdMenuItem sets the list item role via its controller on update, we need
    // to do the same here to correct it when itemEnd is 'switch'.
    if (this.itemEnd === 'switch') {
      this.listItem?.setAttribute('role', 'menuitemcheckbox');
      this.listItem?.setAttribute(
          'aria-checked', this.switchSelected ? 'true' : 'false');
    }
  }

  /**
   * The text that is selectable via typeahead. If not set, defaults to the
   * `headline` property.
   * @export
   */
  get typeaheadText() {
    const item = this.renderRoot?.querySelector('md-menu-item');
    if (item) {
      return item.typeaheadText;
    }
    return this.missedPropertySets.typeaheadText ?? '';
  }

  /** @export */
  set typeaheadText(text: string) {
    const item = this.renderRoot?.querySelector('md-menu-item');
    if (!item) {
      this.missedPropertySets.typeaheadText = text;
    } else {
      item.typeaheadText = text;
    }
  }

  /**
   * Whether or not to display the menu item in the selected visual state.
   * @export
   */
  get selected() {
    const item = this.renderRoot?.querySelector('md-menu-item');
    if (item) {
      return item.selected;
    }
    return this.missedPropertySets.selected ?? false;
  }

  /** @export */
  set selected(selected: boolean) {
    const item = this.renderRoot?.querySelector('md-menu-item');
    if (!item) {
      this.missedPropertySets.selected = selected;
    } else {
      item.selected = selected;
    }
  }

  /** @export */
  get switchSelected() {
    const crosSwitch = this.renderRoot?.querySelector('cros-switch');
    if (crosSwitch) {
      return crosSwitch.selected;
    }
    return this.missedPropertySets.switchSelected ?? false;
  }

  /** @export */
  set switchSelected(value: boolean) {
    const crosSwitch = this.renderRoot?.querySelector('cros-switch');
    if (!crosSwitch) {
      this.missedPropertySets.switchSelected = value;
    } else {
      crosSwitch.selected = value;
    }
  }

  override connectedCallback() {
    super.connectedCallback();
    // All aria properties on button just get proxied down to the real <button>
    // element, as such we set role to presentation so screenreaders ignore
    // this component and instead only read aria attributes off the inner
    // interactive element.
    this.setAttribute('role', 'presentation');
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    if (this.itemEnd === 'switch' || this.keepOpen) {
      this.removeEventListener('keydown', this.keyDownListener);
      this.removeEventListener('click', this.clickListener);
    }
  }

  private keyDownListener(e: KeyboardEvent) {
    if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault();
      this.onTrigger();
    }
  }

  private clickListener() {
    this.onTrigger();
  }

  private onTrigger() {
    if (this.itemEnd === 'switch') {
      const crosSwitch = this.renderRoot.querySelector('cros-switch')!;
      if (this.disabled) return;
      crosSwitch.selected = !crosSwitch.selected;
      if (crosSwitch.selected) {
        this.listItem?.setAttribute('aria-checked', 'true');
      } else {
        this.listItem?.setAttribute('aria-checked', 'false');
      }
    }
    this.fireTriggerEvent();
  }

  // Set item to be placed in `end` slot of menu item (if any).
  // Checked property (check mark) will always take precedence over any icon
  // placed in end slot (if both are specified the checked icon will be used).
  // `switch` enforces cros-switch to be placed into the md-menu-item end slot,
  // `shortcut` uses a span that uses `shortcutText` as a label in the
  // md-menu-item end slot. `icon` exposes a slot so the user can pass in a
  // slotted element via cros-menu-item.
  private getEndSlot() {
    if (this.checked) {
      return html`<div id="checked-icon" slot="end">${CHECKED_ICON}</div>`;
    }
    let endSlot;
    switch (this.itemEnd) {
      case 'shortcut':
        endSlot = html`
          <span
              slot="end"
              class="slot-end"
              id="shortcut">
            ${this.shortcutText}
          </span>`;
        break;
      case 'icon':
        endSlot = html`
          <slot
              name="end"
              slot="end"
              class="slot-end"
              aria-hidden="true">
          </slot>`;
        break;
      case 'switch':
        endSlot = html`
          <cros-switch
              slot="end"
              class="slot-end"
              ?selected=${!!this.missedPropertySets.switchSelected}>
          </cros-switch>`;
        break;
      default:
        endSlot = html``;
        break;
    }

    // md-list-item has a ::slotted(*) selector which will never select
    // the children of a <slot> or a <slot> itself which is why default
    // content is not set as children of slot.
    // i.e. <slot slot="end">${slotEnd}</slot>
    return [html`<slot name="end" slot="end"></slot>`, endSlot];
  }

  private fireTriggerEvent(e?: CloseMenuEvent) {
    const reason = e?.detail.reason;
    if (reason && reason.kind === 'keydown' && reason.key === 'Escape') {
      return;
    }

    this.dispatchEvent(
        new CustomEvent(MenuItem.events.MENU_ITEM_TRIGGERED, {bubbles: true, composed: true}));
  }

  override render() {
    const endSlot = this.getEndSlot();
    // `keep-open` property needs to be true in switch case so cros-switch can
    // be selected/unselected without menu closing.
    const keepOpen = this.itemEnd === 'switch' || this.keepOpen;
    let startSlot = null;
    if (this.itemStart === 'icon') {
      startSlot = html`
        <slot name="start" slot="start" aria-hidden="true"></slot>
      `;
    }
    return html`
      <md-menu-item
          aria-haspopup=${(this.ariaHasPopup || 'false') as AriaHasPopupValue}
          aria-label=${this.ariaLabel ?? ''}
          @close-menu=${(e: CloseMenuEvent) => void this.fireTriggerEvent(e)}
          .keepOpen=${keepOpen}
          .disabled=${this.disabled}
          .tabIndex=${this.tabIndex}
           ?selected=${!!this.missedPropertySets.selected}
           typeahead-text=${ifDefined(this.missedPropertySets.typeaheadText)}
          .type=${this.type}>
        <div slot="headline" id="headline">${this.headline}</div>
        ${startSlot}
        ${endSlot}
      </md-menu-item>
      `;
  }

  // This is just in case `delegatesFocus` doesn't do what is intended (which
  // is unfortunately often across browser updates).
  override focus() {
    this.renderRoot.querySelector('md-menu-item')?.focus();
  }
}

customElements.define('cros-menu-item', MenuItem);

declare global {
  interface HTMLElementEventMap {
    [MenuItem.events.MENU_ITEM_TRIGGERED]: MenuItemTriggeredEvent;
  }
  interface HTMLElementTagNameMap {
    'cros-menu-item': MenuItem;
  }
}
