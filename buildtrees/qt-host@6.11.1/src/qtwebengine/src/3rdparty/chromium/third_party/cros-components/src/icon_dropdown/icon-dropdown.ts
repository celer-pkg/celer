/**
 * @license
 * Copyright 2024 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/iconbutton/filled-tonal-icon-button.js';
import '@material/web/iconbutton/icon-button.js';
import '../menu/menu';

import {Corner, FocusState} from '@material/web/menu/menu.js';
import {SelectOption} from '@material/web/select/select-option.js';
import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';
import {ifDefined} from 'lit/directives/if-defined';
import {createRef, ref} from 'lit/directives/ref';

import {type Menu, type MenuType} from '../menu/menu';

import {IconDropdownOption, type OptionTriggeredEvent} from './icon-dropdown-option';

// TODO: b/355342209 - Investigate moving SVGs into build command or asset
// compatible with Chromium build process.
/** The SVG to use in the trailing icon slot when the dropdown is closed. */
const ARROW_DROP_DOWN_SVG = html`
  <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 -960 960 960">
    <path d="M480-376.348 274.848-581.5h410.304L480-376.348Z"/>
  </svg>`;

/** The SVG to use in the trailing icon slot when the dropdown is closed. */
const ARROW_DROP_UP_SVG = html`
  <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 -960 960 960">
    <path d="M274.848-378.5 480-583.652 685.152-378.5H274.848Z"/>
  </svg>`;

/** Custom CSS for styling anchor button. */
const ANCHOR_PRIMARY_ICON_CONTAINER_PADDING_INLINE =
    css`var(--cros-icon-dropdown-icon-padding-inline, var(--_button-icon-padding))`;
const ANCHOR_PRIMARY_ICON_SIZE =
    css`var(--cros-icon-dropdown-icon-size, var(--_button-icon-size))`;
const ANCHOR_ARROW_ICON_SIZE =
    css`var(--cros-icon-dropdown-arrow-icon-size, var(--_button-arrow-icon-size))`;

/** A chromeOS compliant icon-dropdown. */
export class IconDropdown extends LitElement {
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true,
  };

  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      --_button-arrow-icon-size: 20px;
      --_button-container-color: var(--cros-sys-surface_variant);
      --_button_container-disabled-color: var(--cros-sys-disabled_container);
      --_button-container-disabled-opacity: 1;
      --_button-container-height: 40px;
      --_button-container-hover-color: var(--cros-sys-hover_on_subtle);
      --_button-container-hover-opacity: 1;
      --_button-container-pressed-opacity: 1;
      --_button-container-pressed-color: var(--cros-sys-ripple_neutral_on_subtle);
      --_button-container-shape: 12px;
      --_button-container-width: 40px;
      --_button-focus-ring-color: var(--cros-sys-focus_ring);
      --_button-focus-ring-width: 2px;
      --_button-icon-color: var(--cros-sys-on_surface);
      --_button-icon-disabled-color: var(--cros-sys-disabled);
      --_button-icon-disabled-opacity: var(--cros-sys-disabled-opacity);
      --_button-icon-focus-color: var(--cros-sys-on_surface);
      --_button-icon-hover-color: var(--cros-sys-on_surface);
      --_button-icon-container-padding-inline: 4px 1px;
      --_button-icon-size: 20px;
      display: inline-block;
      font: var(--cros-body-0-font);
    }

    :host([shape="circle"]) {
      --_button-container-shape: 100vmax;
      --_button-container-width: 48px;
      --_button-icon-padding-inline: 8px 6px;
    }

    :host([size="large"]) {
      --_button-container-height: 56px;
      --_button-container-width: 72px;
      --_button-icon-size: 24px;
      --_button-icon-padding-inline: 18px 15px;
    }

    :host([shape="square"][size="large"]) {
      --_button-container-shape: 16px;
    }

    :host([surface="base"]) md-icon-button {
      --_button-icon-color: var(--cros-sys-on_surface);
      --_button-icon-focus-color: var(--cros-sys-on_surface);
    }

    :host([surface="prominent"]) md-icon-button {
      --_button-icon-color: var(--cros-sys-on_primary);
      --_button-icon-focus-color: var(--cros-sys-on_primary);
      --_button-icon-hover-color: var(--cros-sys-on_primary);
      --_button-container-hover-color: var(--cros-sys-hover_on_prominent);
      --_button-container-pressed-color: var(--cros-sys-ripple_neutral_on_prominent);
    }

    :host([surface="subtle"]) md-icon-button {
      --_button-icon-color: var(--cros-sys-on_primary_container);
      --_button-icon-focus-color: var(--cros-sys-on_primary_container);
      --_button-icon-hover-color: var(--cros-sys-on_primary_container);
      --_button-container-pressed-color: var(--cros-sys-ripple_neutral_on_subtle);
    }

    .button-icon-container {
      align-items: center;
      box-sizing: border-box;
      block-size: var(--_button-container-height);
      display: flex;
      inline-size: var(--_button-container-width);
      place-content: center;
      padding-inline: ${ANCHOR_PRIMARY_ICON_CONTAINER_PADDING_INLINE};
    }

    .button-icon-container .dropdown-arrow {
      align-self: center;
      align-items: center;
      display: flex;
      /* 5px overlap with the button icon. */
      margin-inline-start: -5px;
    }

    .button-icon-container .dropdown-arrow svg {
      block-size: ${ANCHOR_ARROW_ICON_SIZE};
      color: var(--_button-icon-color);
      fill: var(--_button-icon-color);
      inline-size: ${ANCHOR_ARROW_ICON_SIZE};
    }

    .button-icon-container .dropdown-icon {
      align-self: center;
      block-size: ${ANCHOR_PRIMARY_ICON_SIZE};
      color: var(--_button-icon-color);
      fill: var(--_button-icon-color);
      inline-size: ${ANCHOR_PRIMARY_ICON_SIZE};
    }

    md-filled-tonal-icon-button {
      --md-filled-tonal-icon-button-container-color: var(--_button-container-color);
      --md-filled-tonal-icon-button-container-height: var(--_button-container-height);
      --md-filled-tonal-icon-button-container-shape: var(--_button-container-shape);
      --md-filled-tonal-icon-button-container-width: var(--_button-container-width);
      --md-filled-tonal-icon-button-disabled-container-color: var(--_button_container-disabled-color);
      --md-filled-tonal-icon-button-disabled-container-opacity: var(--_button-container-disabled-opacity);
      --md-filled-tonal-icon-button-disabled-icon-color: var(--_button-icon-disabled-color);
      --md-filled-tonal-icon-button-disabled-icon-opacity: var(--_button-icon-disabled-opacity);
      --md-filled-tonal-icon-button-hover-state-layer-color: var(--_button-container-hover-color);
      --md-filled-tonal-icon-button-hover-state-layer-opacity: var(--_button-container-hover-opacity);
      --md-filled-tonal-icon-button-icon-color: var(--_button-icon-color);
      --md-filled-tonal-icon-button-icon-focus-color: var(--_button-icon-focus-color);
      --md-filled-tonal-icon-button-icon-size: ${ANCHOR_PRIMARY_ICON_SIZE};
      --md-filled-tonal-icon-button-pressed-state-layer-opacity: var(--_button-container-pressed-opacity);
      --md-filled-tonal-icon-button-pressed-icon-color: var(--_button-icon-pressed-color);
      --md-filled-tonal-icon-button-pressed-state-layer-color: var(--_button-container-pressed-color);;
      --md-filled-tonal-icon-button-pressed-state-layer-opacity: 1;
    }

    md-icon-button {
      --md-icon-button-disabled-icon-color: var(--_button-icon-disabled-color);
      --md-icon-button-disabled-icon-opacity: var(--_button-icon-disabled-opacity);
      --md-icon-button-focus-icon-color: var(--_button-icon-focus-color);
      --md-icon-button-hover-icon-color: var(--_button-icon-hover-color);
      --md-icon-button-hover-state-layer-color: var(--_button-container-hover-color);
      --md-icon-button-hover-state-layer-opacity: var(--_button-container-hover-opacity);
      --md-icon-button-icon-color: var(--_button-icon-color);
      --md-icon-button-icon-size: ${ANCHOR_PRIMARY_ICON_SIZE};
      --md-icon-button-state-layer-height: var(--_button-container-height);
      --md-icon-button-state-layer-shape: var(--_button-container-shape);
      --md-icon-button-state-layer-width: var(--_button-container-width);
      --md-icon-button-pressed-icon-color: var(--_button-icon-pressed-color);
      --md-icon-button-pressed-state-layer-color: var(--_button-container-pressed-color);
      --md-icon-button-pressed-state-layer-opacity: var(--_button-container-pressed-opacity);
    }

    md-filled-tonal-icon-button::part(focus-ring),
    md-icon-button::part(focus-ring) {
      --md-focus-ring-color: var(--_button-focus-ring-color);
      --md-focus-ring-duration: 0s; /* disabled animation */
      --md-focus-ring-width: var(--_button-focus-ring-width);
    }
    :host([surface="prominent"]) md-filled-tonal-icon-button::part(focus-ring),
    :host([surface="prominent"]) md-icon-button::part(focus-ring) {
      --_button-focus-ring-color: var(--cros-sys-inverse_focus_ring);
    }

    slot[name='button-icon']::slotted(*) {
      block-size: ${ANCHOR_PRIMARY_ICON_SIZE};
      inline-size: ${ANCHOR_PRIMARY_ICON_SIZE};
    }
  `;

  /** @nocollapse */
  static override properties = {
    anchorCorner: {type: String, attribute: 'anchor-corner'},
    ariaLabel: {type: String, reflect: true, attribute: 'aria-label'},
    value: {type: String},
    shape: {type: String, reflect: true},
    size: {type: String, reflect: true},
    buttonStyle: {type: String},
    surface: {type: String, reflect: true},
    disabled: {type: Boolean},
    menuCorner: {type: String, attribute: 'menu-corner'},
    menuType: {type: String, attribute: 'menu-type'},
    // Not intended to be set by markup. Reactive only for updating arrow icon.
    menuOpen: {type: Boolean, attribute: false},
  };

  /** @nocollapse */
  static events = {
    CHANGE: 'change',
    MENU_OPENED: 'opened',
    MENU_CLOSED: 'closed'
  } as const;

  /**
   * The corner of the anchor element that the menu will be anchored
   * to.
   * @export
   */
  declare anchorCorner: Corner;

  /**
   * The value of the first checked option in the dropdown or an empty string
   * if no options are checked.
   * @export
   */
  declare value: string;

  /**
   * The shape of the dropdown icon button. Used to apply CSS.
   * @export
   */
  declare shape: 'circle'|'square';

  /**
   * The size of the dropdown icon button. Used to apply CSS.
   * @export
   */
  declare size: 'default'|'large';

  /**
   * Whether the dropdown icon button style is filled or floating button.
   * @export
   */
  declare buttonStyle: 'filled'|'floating';

  /**
   * @export
   * The background the icon button sits on. Used to apply CSS.
   */
  declare surface: 'base'|'prominent'|'subtle';

  /**
   * Whether the dropdown icon button is disabled.
   * @export
   */
  declare disabled: boolean;

  /**
   * The corner of the menu that will be anchored to the anchor element.
   * @export
   */
  declare menuCorner: Corner;

  /**
   * The type of menu to use. Used to ensure correct aria-role is set.
   * @export
   */
  declare menuType: MenuType;

  /**
   * Whether the dropdown menu is open. Used to determine which arrow icon to
   * show and update the dropdown state when menu is closed via the menu's
   * internal event.
   * @export
   */
  private declare menuOpen: boolean;

  protected menuRef = createRef<Menu>();

  constructor() {
    super();

    this.anchorCorner = Corner.END_START;
    this.value = '';
    this.shape = 'square';
    this.size = 'default';
    this.buttonStyle = 'filled';
    this.surface = 'base';
    this.disabled = false;
    this.menuCorner = Corner.START_START;
    this.menuOpen = false;
    this.menuType = 'listbox';
  }

  get anchor() {
    if (this.buttonStyle === 'floating') {
      return this.renderRoot?.querySelector('md-icon-button');
    }

    return this.renderRoot?.querySelector('md-filled-tonal-icon-button');
  }

  get crosMenu() {
    return this.renderRoot?.querySelector('cros-menu');
  }

  get options(): SelectOption[] {
    return (this.crosMenu?.items ?? []) as SelectOption[];
  }

  /** Returns index of first option with matching value. */
  get selectedIndex(): number {
    return this.options.findIndex(option => option.value === this.value);
  }

  /**
   * Sets value equal to the value of the first option with matching index. If
   * no matching index is found then value is set to an empty string.
   */
  set selectedIndex(index: number) {
    const options = this.options;
    if (index < 0 || index >= options.length) {
      this.value = '';
    } else {
      this.value = options[index]!.value;
    }
    this.updateOptionsActivated();
  }

  override focus() {
    // TODO: b/339905985 - Let the menu handle focus when the menu is open.
    this.anchor?.focus();
  }

  override async firstUpdated() {
    await this.crosMenu?.updateComplete;
    if (this.value === '') {
      const items = (this.crosMenu?.items) as IconDropdownOption[] ?? [];
      this.value = items.find(option => option.checked)?.value ?? '';
    } else {
      this.updateOptionsActivated();
    }
    this.updateActiveDescendant();
  }

  override updated(changedProperties: PropertyValues<IconDropdown>) {
    if (changedProperties.has('value')) {
      this.updateOptionsActivated();
      this.updateActiveDescendant();
    }
  }

  /**
   * IconDropdown acts as a `select` element thus it requires the option list
   * and "field" to be updated to display and focus correctly.
   */
  override async getUpdateComplete() {
    await this.anchor?.updateComplete;
    await this.crosMenu?.updateComplete;
    return super.getUpdateComplete();
  }

  override render() {
    return html`
      <div @cros-icon-dropdown-option-triggered=${this.onOptionTriggered}>
        ${this.renderButtonContent()}
        ${this.renderMenuContent()}
      </div>
    `;
  }

  // TODO: Once Chromium lit dependency can support use of static html, replace
  // if block with single literal tag depending the button style similiar to
  // how MWC does it.
  private renderButtonContent() {
    const anchorId = 'button';
    const menuId = 'icon-dropdown-menu';
    const ariaLabel = ifDefined(this.ariaLabel ?? '');
    const disabled = ifDefined(this.disabled ?? '');
    const anchorContent = this.getAnchorContent();

    if (this.buttonStyle === 'floating') {
      return html`
      <md-icon-button
        id="${anchorId}"
        aria-label=${ariaLabel}
        aria-controls="${menuId}"
        @click=${() => this.toggleMenu()}
        @keydown=${(e: KeyboardEvent) => this.onKeydown(e)}
        .disabled=${disabled}
      >
        ${anchorContent}
      </md-icon-button>
    `;
    }

    return html`
      <md-filled-tonal-icon-button
        id="${anchorId}"
        aria-label=${ariaLabel}
        aria-controls="${menuId}"
        @click=${() => this.toggleMenu()}
        @keydown=${(e: KeyboardEvent) => this.onKeydown(e)}
        .disabled=${disabled}
      >
        ${anchorContent}
      </md-filled-tonal-icon-button>
    `;
  }

  private getArrowIcon() {
    return this.menuOpen ? ARROW_DROP_UP_SVG : ARROW_DROP_DOWN_SVG;
  }

  private getAnchorContent() {
    return html`
        <div class="button-icon-container">
          <span class="dropdown-icon"><slot name="button-icon"></slot></span>
          <span class="dropdown-arrow">${this.getArrowIcon()}</span>
        </div>
      `;
  }

  private onKeydown(e: KeyboardEvent) {
    if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault();
      e.stopPropagation();
      this.toggleMenu();
    }
  }

  private toggleMenu() {
    if (!this.menuRef.value) {
      return;
    }
    this.menuRef.value.open ? this.menuRef.value.close() :
                              this.menuRef.value.show();
  }

  private renderMenuContent() {
    return html`
      <cros-menu
        id="icon-dropdown-menu"
        anchor="button"
        ${ref(this.menuRef)}
        .anchorCorner=${this.anchorCorner}
        .menuCorner=${this.menuCorner}
        .open=${this.menuOpen}
        .type=${this.menuType}
        @opened=${this.onMenuOpened}
        @closed=${this.onMenuClosed}
        .defaultFocus=${FocusState.FIRST_ITEM}
        part="menu"
      >
        <slot></slot>
      </cros-menu>
    `;
  }

  protected maybeFocusOnActiveItem() {
    if (this.menuType === 'listbox') {
      const selectedItem = (this.crosMenu?.items as IconDropdownOption[] ??
                            []).find(option => option.value === this.value);
      selectedItem?.focus();
    }
  }

  // This is to ensure the menuOpen property is updated when the menu is opened.
  private onMenuOpened() {
    this.maybeFocusOnActiveItem();
    this.menuOpen = true;
  }

  // This is to ensure the menuOpen property is updated when the menu is closed.
  private onMenuClosed() {
    this.menuOpen = false;
  }

  /**
   * Handles dropdown option selection. If the checked option is the same as
   * the current value, do nothing. Otherwise, update the value and dispatch a
   * change event.
   *
   * @param e The event object.
   */
  private onOptionTriggered(e: OptionTriggeredEvent) {
    // OptionTriggeredEvent only notifies the parent dropdown and should not
    // propagate.
    e.stopPropagation();
    const menuItem = e.detail.menuItem as IconDropdownOption;
    if (!menuItem) {
      return;
    }
    if (menuItem.value === this.value) {
      return;
    }
    this.value = menuItem.value ?? '';
    this.dispatchEvent(
        new Event(IconDropdown.events.CHANGE, {bubbles: true, composed: true}));
  }

  private updateOptionsActivated() {
    for (const option of this.options as IconDropdownOption[]) {
      option.checked = option.value === this.value;
    }
  }

  private updateActiveDescendant() {
    const mdMenu = this.crosMenu?.mdMenu;
    const selectedIndex = this.selectedIndex;
    let activeDescendantId = '';
    // selectedIndex is -1 when no option is selected. Due to the implementation
    // it will never be larger than the number of options.
    if (selectedIndex > -1) {
      activeDescendantId = this.options[selectedIndex]!.id;
    }
    mdMenu?.setAttribute('aria-activedescendant', activeDescendantId);
  }
}

customElements.define('cros-icon-dropdown', IconDropdown);

declare global {
  interface HTMLElementEventMap {
    [IconDropdown.events.CHANGE]: Event;
    [IconDropdown.events.MENU_OPENED]: Event;
    [IconDropdown.events.MENU_CLOSED]: Event;
  }

  interface HTMLElementTagNameMap {
    'cros-icon-dropdown': IconDropdown;
  }
}
