/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/menu/menu.js';
import '@material/web/focus/md-focus-ring.js';

import {Corner, FocusState, MdMenu, Menu as MenuInterface} from '@material/web/menu/menu.js';
import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';

/**
 * Type used to set menuType property on cros-menu. Aligns with the `role`
 * attribute of the `md-menu` component.
 */
export type MenuType = 'menu'|'listbox';

/**
 * A chromeOS menu component.
 */
export class Menu extends LitElement implements MenuInterface {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: flex;
      /* Do not affect parent width */
      width: 0px;
      --cros-menu-width: 96px;
    }
    md-menu {
      --md-menu-container-color: var(--cros-sys-base_elevated);
      --md-menu-item-container-color: var(--cros-sys-base_elevated);
      --md-menu-item-label-text-font: var(--cros-button-2-font-family);
      --md-menu-item-label-text-size: var(--cros-button-2-font-size);
      --md-menu-item-label-text-line-height: var(--cros-button-2-line-height);
      --md-menu-item-label-text-weight: var(--cros-button-2-font-weight);
      --md-menu-item-one-line-container-height: 36px;
      --md-menu-item-top-space: 0px;
      --md-menu-item-bottom-space: 0px;
      --md-menu-container-shape: 8px;
      --md-menu-container-surface-tint-layer-color: white;
      min-width: max(var(--cros-menu-width), 96px);
      /*
       * By default menu is display:contents. To make it focusable, it must be
       * something visible.
       */
      display: flex;
    }
    md-menu:not([open]) {
      /*
       * Makes menu non-tab-focusable when closed. We can do this as long as
       * there  is no animation set on menu as this might interfere with the
       * animation.
       *
       * Additionally we don't set tabindex on the menu when open as submenus
       * programmatically call .focus() when this.open still might not reflect
       * the actual state of the menu being open.
       */
      display: none;
    }
    md-focus-ring {
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-width: 2px;
      --md-focus-ring-active-width: 2px;
      --md-focus-ring-shape: 8px;
    }
    md-menu::part(elevation) {
      --md-menu-container-elevation: 0;
      box-shadow: 0px 12px 12px 0px rgba(var(--cros-sys-shadow-rgb), 0.2);
    }
    md-menu:focus-visible {
      outline: none;
    }
  `;
  /** @nocollapse */
  static override properties = {
    anchor: {type: String},
    anchorCorner: {type: String, attribute: 'anchor-corner'},
    hasOverflow: {type: Boolean, attribute: 'has-overflow'},
    open: {type: Boolean},
    quick: {type: Boolean},
    menuCorner: {type: String, attribute: 'menu-corner'},
    defaultFocus: {type: String, attribute: 'default-focus'},
    skipRestoreFocus: {type: Boolean, attribute: 'skip-restore-focus'},
    stayOpenOnOutsideClick:
        {type: Boolean, attribute: 'stay-open-on-outside-click'},
    isSubmenu: {type: Boolean, attribute: 'is-submenu'},
    usePopover: {type: Boolean, attribute: 'use-popover'},
    type: {type: String}
  };

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };

  private readonly anchorKeydownListener = async (e: KeyboardEvent) => {
    // We need to let this propagate and see if another listener has handled
    // this event. e.g. in the case of a submenu anchoring to a sub-menu-item
    // we do not want to open this menu if the user is just pressing down to
    // navigate past that menu item.
    await new Promise(resolve => {
      setTimeout(resolve);
    });

    // This was handled by another listener e.g. md-menu
    if (e.defaultPrevented) return;

    if (e.key === 'ArrowDown') {
      await this.focusFirstItem();
    } else if (e.key === 'ArrowUp') {
      await this.focusLastItem();
    }
  };

  async focusFirstItem() {
    this.mdMenu.defaultFocus = FocusState.FIRST_ITEM;
    await this.mdMenu.updateComplete;
    this.show();
  }

  async focusLastItem() {
    this.mdMenu.defaultFocus = FocusState.LAST_ITEM;
    await this.mdMenu.updateComplete;
    this.show();
  }

  /** @export */
  declare anchor: string;
  /** @export */
  declare anchorCorner: Corner;
  /** @export */
  declare hasOverflow: boolean;
  /**
   * Sets the initial state of the md-menu on first render. In general do not
   * set this and instead use show() and close().
   * @export
   */
  declare open: boolean;

  // The following properties are necessary for wrapping and keyboard
  // navigation
  /**
   * Skips the opening and closing animations.
   * @export
   */
  declare quick: boolean;
  /**
   * The corner of the menu which to align the anchor in the standard logical
   * property style of <block>_<inline>.
   * @export
   */
  declare menuCorner: Corner;
  /**
   * The element that should be focused by default once opened.
   * @export
   */
  declare defaultFocus: FocusState;
  /**
   * After closing, does not restore focus to the last focused element before
   * the menu was opened.
   * @export
   */
  declare skipRestoreFocus: boolean;
  /**
   * Set automatically by `cros-sub-menu-item`. Whether or not this menu is a
   * submenu and should defer handling left keyboard navigations to the parent
   * `cros-sub-menu-item`.
   */
  declare isSubmenu: boolean;
  /**
   * If cros-menu should use the build-in popover API. If true cros-menu is
   * rendered in an isolated layer on top of all other contents in the web,
   * regardless of positioning in the DOM or current stacking context. However
   * the menu then positions exclusviely using the javascript accessible x and y
   * position of the provided `anchor`.
   * @export
   */
  declare usePopover: boolean;

  /**
   * Determines which role menu should use thus adjusting keyboard navigation.
   * @export
   */
  declare type: MenuType;

  get mdMenu(): MdMenu {
    return this.renderRoot.querySelector('md-menu') as MdMenu;
  }

  /** @private */
  currentAnchorElement: HTMLElement|null = null;

  /** @export */
  get anchorElement(): HTMLElement|null {
    if (this.anchor) {
      return (this.getRootNode() as Document | ShadowRoot)
          .querySelector(`#${this.anchor}`);
    }

    return this.currentAnchorElement;
  }

  /** @export */
  set anchorElement(element: HTMLElement|null) {
    this.currentAnchorElement = element;
    this.requestUpdate('anchorElement');
  }

  constructor() {
    super();
    this.anchor = '';
    this.anchorElement = null;
    this.anchorCorner = Corner.END_START;
    this.hasOverflow = false;
    this.open = false;
    this.quick = true;
    this.menuCorner = Corner.START_START;
    // TODO: b/300001060 - confirm whether should change to FIRST_ITEM
    // NOTE this comment also applies to the tabindex=0 in the template
    this.defaultFocus = FocusState.LIST_ROOT;
    this.skipRestoreFocus = false;
    this.isSubmenu = false;
    this.usePopover = true;
    this.type = 'menu';
  }

  override firstUpdated() {
    // Per spec, the submenu needs to be offset to account for the 8px padding
    // at the top of the menu. This way the first element of the submenu is
    // inline with the parent (anchor) sub-menu-item as per spec.
    if (this.slot === 'submenu') {
      this.mdMenu.yOffset = -8;
    }
  }

  // We add the listener & aria-expanded here instead of
  // connectedCallback/firstUpdated in case the anchor is added after initial
  // menu render.
  override updated(changedProperties: PropertyValues<this>) {
    super.updated(changedProperties);

    const anchorElement = this.anchorElement;

    if ((changedProperties.has('anchor') ||
         changedProperties.has('anchorElement')) &&
        anchorElement) {
      anchorElement.addEventListener('keydown', this.anchorKeydownListener);
      anchorElement.setAttribute('aria-expanded', 'false');
      anchorElement.setAttribute('aria-haspopup', 'menu');
    }
  }

  override disconnectedCallback() {
    const anchorElement = this.anchorElement;
    if (anchorElement) {
      anchorElement.removeEventListener('keydown', this.anchorKeydownListener);
    }
    super.disconnectedCallback();
  }

  /**
   * The menu items associated with this menu. The items must be `MenuItem`s and
   * have both the `md-menu-item` and `md-list-item` attributes.
   */
  get items() {
    // This accessor is required for keyboard navigation
    if (!this.mdMenu) {
      return [];
    }

    return this.mdMenu.items;
  }

  override render() {
    return html`
      <md-menu
          id="menu"
          tabindex="0"
          role=${this.type}
          .anchorElement=${this.anchorElement}
          .open=${this.open}
          .hasOverflow=${this.hasOverflow}
          .anchorCorner=${this.anchorCorner}
          .menuCorner=${this.menuCorner}
          .quick=${this.quick}
          .defaultFocus=${this.defaultFocus}
          .skipRestoreFocus=${this.skipRestoreFocus}
          .isSubmenu=${this.isSubmenu}
          .positioning=${this.usePopover ? 'popover' : 'absolute'}
          @closed=${this.onMdMenuClosed}
          @opened=${this.show}
          @closing=${this.onClosing}
          @opening=${this.onOpening}
        >
        <slot></slot>
        <md-focus-ring for="menu"></md-focus-ring>
      </md-menu>
    `;
  }

  /**
   * Opens the menu.
   * @export
   */
  show() {
    this.open = true;
    // these non-bubbling events need to be re-dispatched for keyboard
    // navigation to work on submenus.
    this.dispatchEvent(new Event('opened'));
    this.anchorElement?.setAttribute('aria-expanded', 'true');
  }

  /**
   * Closes the menu.
   * @export
   */
  close() {
    this.performClose();
  }

  private performClose() {
    this.open = false;
    // Set default focus back correctly (in case keyboard arrow nav has
    // changed it temporarily). We avoid doing this in focusFirstItem() to
    // avoid race conditions with menu open.
    this.renderRoot.querySelector('md-menu')!.defaultFocus = this.defaultFocus;
    this.anchorElement?.setAttribute('aria-expanded', 'false');
  }

  /**
   * Handles the md-menu closed event. Dispatch the 'closed' event here to
   * ensure that only a single 'closed' event is dispatched regardless of
   * whether the menu is closed via UI interaction or via the `close()` method.
   */
  private onMdMenuClosed() {
    this.dispatchEvent(new Event('closed'));
    this.performClose();
  }

  private onClosing() {
    this.dispatchEvent(new Event('closing'));
  }

  private onOpening() {
    this.dispatchEvent(new Event('opening'));
  }

  // This is just in case `delegatesFocus` doesn't do what is intended (which is
  // unfortunately often across browser updates).
  override focus() {
    this.renderRoot.querySelector('md-menu')?.focus();
  }

  override getBoundingClientRect() {
    if (!this.mdMenu) {
      return super.getBoundingClientRect();
    }
    return this.mdMenu.getBoundingClientRect();
  }
}

customElements.define('cros-menu', Menu);

declare global {
  interface HTMLElementTagNameMap {
    'cros-menu': Menu;
  }
}
