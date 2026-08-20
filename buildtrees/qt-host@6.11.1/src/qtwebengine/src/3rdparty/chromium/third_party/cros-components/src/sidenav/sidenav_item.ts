/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/ripple/ripple.js';
import '@material/web/focus/md-focus-ring.js';
import '../tooltip/tooltip';

import {css, CSSResultGroup, html, LitElement, nothing, PropertyValues} from 'lit';
import {classMap} from 'lit/directives/class-map';
import {StyleInfo, styleMap} from 'lit/directives/style-map';

import {castExists} from '../helpers/helpers';

import {isSidenav, isSidenavItem} from './sidenav_util';

/** Type of the tree item expanded custom event. */
export type SidenavItemExpandedEvent = CustomEvent<{
  /** The tree item which has been expanded. */
  item: SidenavItem,
}>;

/** Type of the tree item collapsed custom event. */
export type SidenavItemCollapsedEvent = CustomEvent<{
  /** The tree item which has been collapsed. */
  item: SidenavItem,
}>;

/** Type of the tree item collapsed custom event. */
export type SidenavItemRenamedEvent = CustomEvent<{
  /** The tree item which has been renamed. */
  item: SidenavItem,
  /** The label before rename. */
  oldLabel: string,
  /** The label after rename. */
  newLabel: string,
}>;

/** Type of the tree item enabled changed custom event. */
export type SidenavItemEnabledChangedEvent = CustomEvent<{
  /** The tree item which has been (un)enabled. */
  item: SidenavItem,
  /** The new enabled value. */
  enabled: boolean,
}>;

/** Type of the tree item triggered custom event. */
export type SidenavItemTriggeredEvent = CustomEvent<{
  /** The tree item which has been triggered. */
  item: SidenavItem,
}>;

/** The number of pixels to indent per level. */
export const TREE_ITEM_INDENT_PX = 20;

/** The width and height of the expand and label icons. */
const ICON_SIZE = css`20px`;

/**
 * Spacing between items in the a row (in LTR):
 *
 * In flat Sidenavs:
 *
 *   1. [ICON_LEADING_MARGIN]
 *      <icon>
 *      [ICON_LABEL_GAP]
 *      <label>
 *      [LABEL_TRAILING_MARGIN]
 *
 *   2. [LABEL_ONLY_MARGIN]
 *      <label>
 *      [LABEL_TRAILING_MARGIN]
 *
 * In layered sidenavs:
 * (the expand icon takes space even when a layered item has no children)
 *
 *   3. [EXPAND_LEADING_MARGIN]
 *      <expand icon>
 *      <icon>
 *      [ICON_LABEL_GAP]
 *      <label>
 *      [LABEL_TRAILING_MARGIN]
 *
 *   4. [EXPAND_LEADING_MARGIN]
 *      <expand icon>
 *      [EXPAND_LABEL_GAP]
 *      <label>
 *      [LABEL_TRAILING_MARGIN]
 */
const ICON_LABEL_GAP = css`8px`;
const EXPAND_LABEL_GAP = css`4px`;
const ICON_LEADING_MARGIN = css`28px`;
const EXPAND_LEADING_MARGIN = css`8px`;
const LABEL_ONLY_MARGIN = css`24px`;
const LABEL_TRAILING_MARGIN = css`12px`;
const ITEM_GAP = css`8px`;

/** Selects a host that is in a layered Sidenav. */
const LAYERED_SELECTOR = css`:host(:is([inLayered], [mayHaveChildren]))`;

// The #tree-item blocks clicks from reaching the host, to prevent clicks on
// empty space between children from triggering the item.
function stopPropagation(e: Event) {
  e.stopPropagation();
}

/** An item for a ChromeOS compliant sidenav element. */
export class SidenavItem extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: block;
      margin-top: ${ITEM_GAP};
    }

    li {
      display: block;
      font: var(--cros-button-2-font);
    }

    :host([separator])::before {
      border-top: 1px solid var(--cros-separator-color);
      content: '';
      display: block;
      padding-bottom: 8px;
      width: 100%;
    }

    #tree-row {
      align-items: center;
      background: none;
      border: none;
      border-inline-start-width: 0 !important;
      border-radius: 20px;
      box-sizing: border-box;
      color: var(--cros-sys-on_surface);
      cursor: pointer;
      display: flex;
      font: var(--cros-button-2-font);
      height: 40px;
      padding-inline-end: ${LABEL_TRAILING_MARGIN};
      position: relative;
      text-align: start;
      user-select: none;
      white-space: nowrap;
      width: 100%;
    }

    #tree-row:focus-visible {
      outline: none;
    }

    md-focus-ring {
      animation-duration: 0s;
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-shape: 20px;
      --md-focus-ring-width: 2px;
    }

    :host([enabled]) #tree-row {
      background-color: var(--cros-sys-primary);
      color: var(--cros-sys-on_primary);
    }

    :host([disabled]) #tree-row {
      color: var(--cros-sys-disabled);
      pointer-events: none;
    }

    :host([error]) #tree-row {
      background-color: var(--cros-sys-error_container);
      color: var(--cros-sys-on_error_container);
    }

    :host-context(.pointer-active) #tree-row:not(:active) {
      cursor: default;
    }

    .expand-icon {
      display: none;
      fill: currentcolor;
      flex: none;
      height: ${ICON_SIZE};
      -webkit-mask-position: center;
      -webkit-mask-repeat: no-repeat;
      position: relative;
      transform: rotate(-90deg);
      transition: all 150ms;
      visibility: hidden;
      width: ${ICON_SIZE};
    }

    ${LAYERED_SELECTOR} .expand-icon {
      display: unset;
    }


    [aria-expanded] .expand-icon {
      visibility: visible;
    }

    .expand-icon,
    slot[name="icon"]::slotted(*) {
      width: 20px;
      height: 20px;
      align-items: center;
    }

    :host-context([dir=rtl]) .expand-icon {
      transform: rotate(90deg);
    }

    :host([expanded]) .expand-icon {
      transform: rotate(0);
    }

    slot[name="icon"]::slotted(*) {
      color: var(--cros-sys-on_surface);
      flex: none;
      height: ${ICON_SIZE};
      margin-inline-start: ${ICON_LEADING_MARGIN};
      width: ${ICON_SIZE};
    }

    :host([enabled]) slot[name="icon"]::slotted(*) {
      color: var(--cros-sys-on_primary)
    }

    :host([disabled]) slot[name="icon"]::slotted(*) {
      color: var(--cros-sys-disabled);
    }

    :host([error]) slot[name="icon"]::slotted(*) {
      color: var(--cros-sys-on_error_container);
    }

    .tree-label {
      display: block;
      flex: auto;
      margin-inline-start: ${LABEL_ONLY_MARGIN};
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: pre;
    }

    ${LAYERED_SELECTOR} .expand-icon {
      margin-inline-start: ${EXPAND_LEADING_MARGIN}
    }

    ${LAYERED_SELECTOR} slot[name="icon"]::slotted(*) {
      margin-inline-start: 0px;
      margin-inline-end: 0px;
    }

    ${LAYERED_SELECTOR} .tree-label {
      margin-inline-start: ${EXPAND_LABEL_GAP};
    }

    ${LAYERED_SELECTOR} .has-icon .tree-label,
    :host([expanded]) .tree-label,
    .has-icon + .tree-label, {
      margin-inline-start: ${ICON_LABEL_GAP};
    }

    .rename {
      background-color: var(--cros-sys-app_base);
      border: none;
      border-radius: 4px;
      color: var(--cros-sys-on_surface);
      height: 20px;
      margin-inline-start: ${LABEL_ONLY_MARGIN};
      outline: 2px solid var(--cros-sys-focus_ring);
      overflow: hidden;
      font: var(--cros-button-2-font);
    }

    .has-icon :is(.tree-label, .rename) {
      margin-inline-start: ${ICON_LABEL_GAP};
    }

    ${LAYERED_SELECTOR} .rename {
      margin-inline-start: ${EXPAND_LABEL_GAP};
    }

    ${LAYERED_SELECTOR} .has-icon :is(.tree-label, .rename) {
      margin-inline-start: ${ICON_LABEL_GAP};
    }

    /* We need to ensure that even empty labels take up space */
    #tree-label:empty::after {
      content: ' ';
      white-space: pre;
    }

    ul.tree-children {
      height: 0px;
      list-style: none;
      margin: 0;
      outline: none;
      overflow: hidden;
      padding: 0;
    }

    :host([expanded]) ul.tree-children {
      display: block;
      height: auto;
      margin: 0;
      overflow: visible;
    }

    :host([enabled]) .rename {
      outline: 2px solid var(--cros-sys-inverse_primary);
    }

    .rename::selection {
      background-color: var(--cros-sys-highlight_text)
    }

    md-ripple {
      border-radius: 20px;
    }

    :host(:not([renaming])) md-ripple {
      color: var(--cros-sys-ripple_primary);
      --md-ripple-hover-color: var(--cros-sys-hover_on_subtle);
      --md-ripple-hover-opacity: 100%;
      --md-ripple-pressed-color: var(--cros-sys-ripple_primary);
      --md-ripple-pressed-opacity: 100%;
    }
  `;
  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };
  /** @nocollapse */
  static override properties = {
    separator: {type: Boolean, reflect: true},
    disabled: {type: Boolean, reflect: true},
    enabled: {type: Boolean, reflect: true},
    expanded: {type: Boolean, reflect: true},
    renaming: {type: Boolean, reflect: true},
    error: {type: Boolean, reflect: true},
    mayHaveChildren: {type: Boolean, reflect: true},
    label: {type: String, reflect: true},
    tabIndex: {attribute: false},
    layer: {type: Number, reflect: true},
    inLayered: {type: Boolean, reflect: true},
    ignoreLayer: {type: Boolean},
    hasIcon: {type: Boolean, reflect: true},
    // Aria properties are forwarded to the `li` element and should not be
    // reflected on `cros-sidenav-item`.
    ariaSetSize: {type: String},
    ariaPosInSet: {type: String},
  };

  /** @nocollapse */
  static get events() {
    return {
      /** Triggers when a sidenav item has been expanded. */
      SIDENAV_ITEM_EXPANDED: 'cros-sidenav-item-expanded',
      /** Triggers when a sidenav item has been collapsed. */
      SIDENAV_ITEM_COLLAPSED: 'cros-sidenav-item-collapsed',
      /** Triggers when a sidenav item's label has been renamed. */
      SIDENAV_ITEM_RENAMED: 'cros-sidenav-item-renamed',
      /** Triggers when a sidenav item's changed status changed. */
      SIDENAV_ITEM_ENABLED_CHANGED: 'cros-sidenav-item-enabled-changed',
      /** Triggers when a sidenav item is clicked or equivalent. */
      SIDENAV_ITEM_TRIGGERED: 'cros-sidenav-item-triggered',
    } as const;
  }

  /**
   * The `separator` attribute will show a top border for the tree item. It's
   * mainly used to identify this tree item is a start of the new section.
   * @export
   */
  declare separator: boolean;
  /**
   * Indicate if a tree item is disabled or not. Disabled tree item will have
   * a greyed out color, can't be enabled, can't get focus. It can still have
   * children, but it can't be expanded, and the expand icon will be hidden.
   * @export
   */
  declare disabled: boolean;
  /**
   * Indicate if a tree item has been enabled or not.
   * @export
   */
  declare enabled: boolean;
  /**
   * Indicate if a tree item has been expanded or not.
   * @export
   */
  declare expanded: boolean;
  /**
   * Indicate if a tree item is in renaming mode or not.
   * @export
   */
  declare renaming: boolean;
  /**
   * Indicate the item contains an error.
   * @export
   */
  declare error: boolean;
  /**
   * A tree item will have children if the child tree items have been inserted
   * into its default slot. Only use `mayHaveChildren` if we want the tree item
   * to appear as having children even without the actual child tree items
   * (e.g. no DOM children). This is mainly used when we asynchronously load
   * child tree items.
   * @export
   */
  declare mayHaveChildren: boolean;
  /**
   * The label text of the tree item.
   * @export
   */
  declare label: string;
  /**
   * If the user navigates to this item, tabs away from the sidenav and then
   * tabs back into the sidenav, the item that receives focus should be the one
   * they had navigated to. `tabIndex` is forwarded to the `<li>` element.
   * @export
   */
  declare tabIndex: number;
  /**
   * Indicate the depth of this tree item, we use it to calculate the padding
   * indentation. Note: "aria-level" can be calculated by DOM structure so
   * no need to provide it explicitly.
   * @export
   */
  declare layer: number;

  /**
   * Always render as if layer is 0.
   * @export
   */
  declare ignoreLayer: boolean;

  /**
   * Whether an icon has been slotted. CSS cannot distinguish slots that have
   * slotted items, so we check manually and set a class.
   * @export
   */
  declare hasIcon: boolean;

  /**
   * Whether this item is in a Sidenav with nested SidenavItems. If this
   * SidenavItem has a Sidenav parent, the parent will control this attribute.
   * @export
   */
  declare inLayered: boolean;

  private get treeRowElement(): HTMLDivElement {
    return castExists(
        this.shadowRoot!.getElementById('tree-row') as HTMLDivElement);
  }

  private get renameInputElement(): HTMLInputElement {
    return castExists(
        this.renderRoot.querySelector('.rename') as HTMLInputElement);
  }

  private get childrenSlotElement(): HTMLSlotElement {
    return castExists(
        this.renderRoot.querySelector('slot:not([name])') as HTMLSlotElement);
  }

  private get iconSlotElement(): HTMLSlotElement {
    return castExists(
        this.renderRoot.querySelector('slot[name="icon"]') as HTMLSlotElement);
  }

  constructor() {
    super();
    this.separator = false;
    this.disabled = false;
    this.enabled = false;
    this.expanded = false;
    this.renaming = false;
    this.error = false;
    this.mayHaveChildren = false;
    this.label = '';
    this.tabIndex = -1;
    this.layer = 0;
    this.ignoreLayer = false;
    this.hasIcon = false;
    this.inLayered = false;
  }

  /** The child tree items which can be tabbed to. */
  get selectableItems(): SidenavItem[] {
    return this.items.filter(item => !item.disabled);
  }

  hasChildren(): boolean {
    return this.mayHaveChildren || this.items.length > 0;
  }

  /**
   * Return the parent SidenavItem, if there is one. For top level SidenavItems
   * with no parent, return null.
   */
  get parentItem(): SidenavItem|null {
    let p = this.parentElement;
    while (p) {
      if (isSidenavItem(p)) {
        return p;
      }
      if (isSidenav(p)) {
        return null;
      }
      p = p.parentElement;
    }
    return p;
  }

  /** Expands all parent items. */
  reveal() {
    let pi = this.parentItem;
    while (pi) {
      pi.expanded = true;
      pi = pi.parentItem;
    }
  }

  /** The tree items that are direct children of this. */
  items: SidenavItem[] = [];

  /** Indicate if we should commit the rename on input blur or not. */
  private shouldRenameOnBlur = true;

  override render() {
    const showExpandIcon = this.hasChildren();
    const effectiveLayer = this.ignoreLayer ? 0 : this.layer;
    const treeRowStyles: StyleInfo = {
      /** @export */
      'paddingInlineStart':
          `max(0px, calc(${TREE_ITEM_INDENT_PX} * ${effectiveLayer}px))`,
    };

    const treeRowClasses = {'has-icon': this.hasIcon};

    return html`
      <li
          id="tree-item"
          role="none"
          @click=${stopPropagation}>
        <!-- TODO: b/302435119 - Use cros-tooltip instead of title. -->
        <div
            id="tree-row"
            role="treeitem"
            aria-current=${this.enabled ? 'page' : nothing}
            aria-setsize=${this.ariaSetSize ?? nothing}
            aria-posinset=${this.ariaPosInSet ?? nothing}
            aria-level=${this.layer + 1}
            aria-labelledby="tree-label"
            aria-expanded=${showExpandIcon ? this.expanded : nothing}
            aria-disabled=${this.disabled}
            tabindex=${this.tabIndex}
            @click=${this.onRowClicked}
            @keydown=${this.onKeyDown}
            class=${classMap(treeRowClasses)}
            style=${styleMap(treeRowStyles)}>
          <md-ripple
              for="tree-row"
              ?disabled=${this.disabled}>
          </md-ripple>
          <md-focus-ring
              for="tree-row"
              ?disabled=${this.disabled}>
          </md-focus-ring>
          <!-- TODO: b/231672472 - Implement icon from spec -->
          <span
              class="expand-icon"
              aria-hidden="true"
              @click=${this.onExpandIconClicked}>
            <svg xmlns="http://www.w3.org/2000/svg"
                width="20"
                height="20"
                viewBox="0 0 20 20">
              <polyline points="6 8 10 12 14 8 6 8"/>
            </svg>
          </span>
          <slot
              name="icon"
              class=${this.hasIcon ? 'has-icon' : ''}
              aria-hidden="true"
              @slotchange=${this.onIconSlotChanged}>
          </slot>
          ${this.renderTreeLabel()}
        </div>
        <ul
            aria-hidden=${!this.expanded}
            class="tree-children"
            role="group">
          <slot @slotchange=${this.onSlotChanged}></slot>
        </ul>
        ${this.renderTooltip()}
       </li>
     `;
  }

  override firstUpdated() {
    // This is needed in addition to a listener on #tree-row, so that we can
    // react to clicks on a slotted icon.
    this.addEventListener('click', this.onRowClicked);
  }

  override updated(changedProperties: PropertyValues<this>) {
    super.updated(changedProperties);
    if (changedProperties.has('expanded')) {
      this.onExpandChanged();
    }
    if (changedProperties.has('enabled')) {
      this.onEnabledChanged();
    }
    if (changedProperties.has('renaming')) {
      this.onRenamingChanged();
    }
    if (changedProperties.has('layer')) {
      for (const item of this.items) {
        item.layer = this.layer + 1;
      }
    }
  }

  private renderTreeLabel() {
    if (this.renaming) {
      // Stop propagation of some events to prevent them being captured by
      // tree when the tree item is in renaming mode.
      const interceptPropagation = (e: MouseEvent) => {
        e.stopPropagation();
      };
      return html`
        <input
          class="rename"
          type="text"
          spellcheck="false"
          .value=${this.label}
          @click=${interceptPropagation}
          @dblclick=${interceptPropagation}
          @mouseup=${interceptPropagation}
          @mousedown=${interceptPropagation}
          @blur=${this.onRenameInputBlur}
          @keydown=${this.onRenameInputKeydown}
        />
      `;
    }
    return html`
    <span
      class="tree-label"
      id="tree-label"
    >${this.label || ''}</span>`;
  }

  /**
   * Renders a tooltip if the label is truncated.
   */
  private renderTooltip() {
    return html`
        <cros-tooltip
          ?hidden=${!this.isLabelTruncated()}
          anchor="tree-row"
          label=${this.label}
          follow-anchor>
        </cros-tooltip>
      `;
  }

  private onIconSlotChanged() {
    this.hasIcon = this.iconSlotElement.assignedElements().length > 0;
    this.iconSlotElement.assignedElements().forEach(icon => {
      icon.setAttribute('tabIndex', '-1');
    });
  }

  // TODO: b/302632527 - Refactor this to avoid storing a copy of children.
  private onSlotChanged() {
    // Whether the old set of children contained the enabled item.
    const oldItemsContainEnabledItem =
        this.items.some((item: SidenavItem) => item.enabled);
    // Update `items` every time when the children slot changes (e.g.
    // add/remove).
    this.items =
        this.childrenSlotElement.assignedElements().filter(isSidenavItem);

    let updateScheduled = false;

    // If an expanded item's last children is deleted, update expanded property.
    if (this.items.length === 0 && this.expanded) {
      this.expanded = false;
      updateScheduled = true;
    }

    if (oldItemsContainEnabledItem &&
        !this.items.some((item: SidenavItem) => item.enabled)) {
      // If the currently enabled item was in the old set of children but not in
      // the new set, it means it's being removed from the children slot. The
      // parent becomes the enabled item.
      this.enabled = true;
      updateScheduled = true;
    }

    this.items.forEach((item: SidenavItem, i: number) => {
      item.layer = this.layer + 1;
      item.ariaSetSize = `${this.items.length}`;
      item.ariaPosInSet = `${i + 1}`;
    });

    // Explicitly trigger an update because render() relies on hasChildren().
    if (!updateScheduled) {
      this.requestUpdate();
    }
  }

  private onExpandChanged() {
    if (this.expanded) {
      const expandedEvent: SidenavItemExpandedEvent =
          new CustomEvent(SidenavItem.events.SIDENAV_ITEM_EXPANDED, {
            bubbles: true,
            composed: true,
            detail: {item: this},
          });
      this.dispatchEvent(expandedEvent);
    } else {
      const collapseEvent: SidenavItemCollapsedEvent =
          new CustomEvent(SidenavItem.events.SIDENAV_ITEM_COLLAPSED, {
            bubbles: true,
            composed: true,
            detail: {item: this},
          });
      this.dispatchEvent(collapseEvent);
    }
  }

  private onEnabledChanged() {
    const event: SidenavItemEnabledChangedEvent =
        new CustomEvent(SidenavItem.events.SIDENAV_ITEM_ENABLED_CHANGED, {
          bubbles: true,
          composed: true,
          detail: {item: this, enabled: this.enabled},
        });
    this.dispatchEvent(event);
  }

  private onExpandIconClicked(e: MouseEvent) {
    e.stopPropagation();
    if (this.disabled) return;
    this.expanded = !this.expanded;
  }

  private onRowClicked(e: MouseEvent) {
    // Prevent the parent `SidenavItem` from acting on this event.
    e.stopPropagation();
    if (this.disabled) return;
    this.enabled = true;

    this.dispatchEvent(
        new CustomEvent(SidenavItem.events.SIDENAV_ITEM_TRIGGERED, {
          bubbles: false,
          detail: {item: this},
        }));
  }

  private onKeyDown(e: KeyboardEvent) {
    if (this.renaming) return;

    switch (e.key) {
      case 'Enter':
      case ' ':
        if (this.disabled) break;
        this.enabled = true;

        this.dispatchEvent(
            new CustomEvent(SidenavItem.events.SIDENAV_ITEM_TRIGGERED, {
              bubbles: false,
              detail: {item: this},
            }));
        break;
      default:
        break;
    }
  }

  private onRenamingChanged() {
    if (this.renaming) {
      this.renameInputElement?.focus();
      this.renameInputElement?.select();
    }
  }

  private onRenameInputKeydown(e: KeyboardEvent) {
    // Make sure that the tree does not handle the key.
    e.stopPropagation();

    if (e.repeat) {
      return;
    }

    // Calling this.focus() blurs the input which will make the tree item
    // non editable.
    switch (e.key) {
      case 'Escape':
        // By default blur() will trigger the rename, but when ESC is pressed
        // we don't want the blur() (triggered by treeRowElement.focus() below)
        // to commit the rename.
        this.shouldRenameOnBlur = false;
        this.treeRowElement.focus();
        e.preventDefault();
        break;
      case 'Enter':
        // treeRowElement.focus() will trigger blur() for the rename input
        // which will commit the rename.
        this.treeRowElement.focus();
        e.preventDefault();
        break;
      default:
        break;
    }
  }

  private onRenameInputBlur() {
    // Renaming is already false then the rename input element is not available
    // and castExists will throw an error. This can occur in tests which
    // programmatically call blur() on the input element.
    if (!this.renaming) {
      return;
    }
    this.renaming = false;
    if (this.shouldRenameOnBlur) {
      this.commitRename(this.renameInputElement?.value || '');
    } else {
      this.shouldRenameOnBlur = true;
    }
  }

  private commitRename(newName: string) {
    const isEmpty = newName.trim() === '';
    const isChanged = newName !== this.label;
    if (isEmpty || !isChanged) {
      return;
    }
    const oldLabel = this.label;
    this.label = newName;
    const renameEvent: SidenavItemRenamedEvent =
        new CustomEvent(SidenavItem.events.SIDENAV_ITEM_RENAMED, {
          bubbles: true,
          composed: true,
          detail: {item: this, oldLabel, newLabel: newName},
        });
    this.dispatchEvent(renameEvent);
  }

  private isLabelTruncated(): boolean {
    const treeLabel = this.shadowRoot?.querySelector('.tree-label');
    if (!treeLabel) {
      return false;
    }
    return treeLabel.scrollWidth > treeLabel.clientWidth;
  }
}

customElements.define('cros-sidenav-item', SidenavItem);

declare global {
  interface HTMLElementEventMap {
    [SidenavItem.events.SIDENAV_ITEM_EXPANDED]: SidenavItemExpandedEvent;
    [SidenavItem.events.SIDENAV_ITEM_COLLAPSED]: SidenavItemCollapsedEvent;
    [SidenavItem.events.SIDENAV_ITEM_RENAMED]: SidenavItemRenamedEvent;
    [SidenavItem.events.SIDENAV_ITEM_ENABLED_CHANGED]:
        SidenavItemEnabledChangedEvent;
    [SidenavItem.events.SIDENAV_ITEM_TRIGGERED]: SidenavItemTriggeredEvent;
  }

  interface HTMLElementTagNameMap {
    'cros-sidenav-item': SidenavItem;
  }
}
