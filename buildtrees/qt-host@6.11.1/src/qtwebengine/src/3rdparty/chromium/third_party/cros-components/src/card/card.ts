/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '@material/web/ripple/ripple.js';
import '@material/web/focus/md-focus-ring.js';

import {css, CSSResultGroup, html, LitElement, nothing} from 'lit';

import {shadowPiercingActiveElements} from '../helpers/helpers';

const DEFAULT_TICK_MARGIN = css`14px`;

function renderTick() {
  return html`
      <svg
          class="selected-tick"
          width="20"
          height="20"
          xmlns="http://www.w3.org/2000/svg">
        <path
            fill-rule="evenodd"
            clip-rule="evenodd"
            d="M19 10a9 9 0 1 1-18 0 9 9 0 0 1 18 0Zm-4.83-3.045a1.125 1.125 0 0
                0-1.59 0l-3.705 3.704L7.42 9.204a1.125 1.125 0 0 0-1.59
                1.591l2.25 2.25c.439.44 1.151.44 1.59
                0l4.5-4.5c.44-.439.44-1.151 0-1.59Z"/>
      </svg>`;
}

/** The style options for the Card component. */
export type CardStyle = 'outline'|'filled'|'elevated';

/** A chromeOS compliant card. */
export class Card extends LitElement {
  /**
   * Note about #content display: Because `display: block` / `inline-block`
   * resize depending on any applicable line-height they often don't neatly wrap
   * user content. Instead we use flex here with direction column to "emulate"
   * `display: block` while ignoring line-height. Users are free to set this
   * to `block` via the shadow part and deal with the line-height sizing
   * themselves if they wish, but by default we want this to tightly wrap
   * user content exactly.
   * @nocollapse
   */
  static override styles: CSSResultGroup = css`
    :host {
      border-radius: 12px;
      color: var(--cros-sys-on_surface);
      display: block;
      font: var(--cros-body-0-font);
      width: fit-content;
      height: fit-content;
      min-width: 50px;
      min-height: 50px;
    }

    :host([interactive]) * {
      cursor: pointer;
    }

    #container {
      align-items: center;
      background-color: var(--cros-card-background-color, var(--cros-sys-app_base));
      border: none;
      border-radius: 12px;
      box-sizing: border-box;
      display: grid;
      font: inherit;
      color: inherit;
      height: 100%;
      outline: 1px solid var(--cros-card-border-color, var(--cros-sys-separator));
      padding: var(--cros-card-padding, 16px);
      position: relative;
      text-align: start;
      width: 100%;
      isolation: isolate;
    }

    #content {
      border-radius: inherit;
      width: inherit;
      height: inherit;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      z-index: 1;
    }

    #background {
      border-radius: inherit;
      inset: 0;
      overflow: hidden;
      position: absolute;
      z-index: 0;
    }

    #background slot::slotted(img) {
      height: 100%;
      object-fit: cover;
      width: 100%;
    }

    :host([cardStyle="filled"]) #container {
      outline: none;
    }

    :host([cardStyle="elevated"]) #container {
      background-color: var(--cros-card-background-color, var(--cros-sys-base_elevated));
      box-shadow: var(--cros-card-elevation-shadow, var(--cros-elevation-1-shadow));
      outline: none;
    }

    :host([selected]) #container {
      background-color: var(--cros-card-selected-background-color, var(--cros-sys-primary_container));
    }

    md-ripple {
      color: var(--cros-sys-ripple_primary);
      --md-ripple-hover-color: var(--cros-card-hover-color, var(--cros-sys-hover_on_subtle));
      --md-ripple-hover-opacity: 1;
      --md-ripple-pressed-color: var(--cros-card-pressed-color, var(--cros-sys-ripple_neutral_on_subtle));
      --md-ripple-pressed-opacity: 1;
      z-index: 3;
    }

    md-focus-ring {
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-duration: 0s;
      --md-focus-ring-outward-offset: 0px;
      --md-focus-ring-shape: 12px;
    }

    :host([disabled]) {
      opacity: var(--cros-disabled-opacity);
    }

    .selected-tick {
      display: none;
    }

    :host([selected]) .selected-tick {
      bottom: var(--cros-card-icon-bottom, ${DEFAULT_TICK_MARGIN});
      display: unset;
      fill: var(--cros-card-icon-color, var(--cros-sys-on_primary_container));
      height: 20px;
      inset-inline-end: var(--cros-card-icon-inline-end, ${
      DEFAULT_TICK_MARGIN});
      position: absolute;
      width: 20px;
      z-index: 2;
    }
  `;

  /** @nocollapse */
  static override shadowRootOptions = {
    ...LitElement.shadowRootOptions,
    delegatesFocus: true
  };

  /** @export */
  declare disabled: boolean;

  /** @export */
  declare selected: boolean;

  /** @export */
  declare interactive: boolean;

  /** @export */
  declare cardStyle: CardStyle;

  /**
   * The card's aria role. Allowed values are a short list of expected roles for
   * a card. They can be extended with types from
   * https://developer.mozilla.org/en-US/docs/Web/Accessibility/ARIA/Roles
   * @export
   */
  declare role: 'button'|'dialog'|'none'|'presentation'|'link'|'img'|'cell'|
      null;

  /** @nocollapse */
  static override properties = {
    cardStyle: {type: String, reflect: true},
    disabled: {type: Boolean, reflect: true},
    selected: {type: Boolean, reflect: true},
    interactive: {type: Boolean, reflect: true},
    ariaLabel: {type: String, attribute: 'aria-label'},
    ariaRoleDescription: {type: String, attribute: 'aria-roledescription'},
    role: {type: String},
    tabIndex: {type: Number},
  };

  constructor() {
    super();
    this.disabled = false;
    this.selected = false;
    this.interactive = false;
    this.cardStyle = 'outline';
    this.role = null;
  }

  override render() {
    const interactive = !this.disabled && this.interactive;
    const hasTabstop = this.hasAttribute('tabIndex') && this.tabIndex > -1;

    const maybeRipple =
        interactive ? html`<md-ripple for="container"></md-ripple>` : nothing;
    const maybeFocusRing = hasTabstop ?
        html`<md-focus-ring for="container"></md-focus-ring>` :
        nothing;

    return html`
        <div
            aria-label=${this.ariaLabel ?? nothing}
            aria-roledescription=${this.ariaRoleDescription ?? nothing}
            tabindex=${this.tabIndex ?? nothing}
            role=${this.role ?? 'none'}
            @keydown=${this.onKeyDown}
            id="container">
          <div id="background" part="background">
            <slot name="background"></slot>
          </div>
          ${maybeRipple}
          ${maybeFocusRing}
          ${renderTick()}
          <div id="content" part="content">
            <slot></slot>
          </div>
        </div>
    `;
  }

  /**
   * Sets focus on the card container if the card is interactive and tabbable.
   * Otherwise, attempts to set focus on a nested element.
   */
  override focus() {
    const isTabbable = this.tabIndex > -1;
    const isInteractive = !this.disabled && this.interactive && isTabbable;
    if (isInteractive || isTabbable) {
      this.renderRoot.querySelector<HTMLDivElement>('#container')?.focus();
      return;
    }
    this.attemptToFocusOnNestedElement();
  }

  private onKeyDown(e: KeyboardEvent) {
    switch (e.key) {
      case 'Enter':
      case ' ':
        if (e.composedPath()[0] !==
            this.shadowRoot!.querySelector('#container')) {
          // If a child element of this card was keyboard-activated, it should
          // not trigger a click on this card.
          break;
        }
        e.currentTarget?.dispatchEvent(
            new MouseEvent('click', {bubbles: true, composed: true}));
        break;
      default:
        break;
    }
  }

  /**
   * Returns true if setting focus on the element would make it the active
   * element.
   */
  private tryFocus(el: HTMLElement) {
    // Skip elements with attributes that make them not focusable.
    // https://web.dev/learn/html/focus#making_interactive_elements_inert
    const notTabbable = el.hasAttribute('tabindex') && el.tabIndex < 0;
    const disabled = el.hasAttribute('disabled');
    const inert = el.hasAttribute('inert');
    if (notTabbable || disabled || inert) {
      return false;
    }

    // Attempt to focus the element and verify if focus succeeded by checking
    // if it is the shadow piercing active element.
    el.focus();
    const activeElement = shadowPiercingActiveElements();
    if (activeElement.includes(el)) {
      return true;
    }

    return false;
  }

  /**
   * Recursively checks if provided element or any of the element's children are
   * focusable via the tryFocus method. Stops at the first focusable element and
   * focuses it.
   */
  private hasFocusableChild(parent: HTMLElement) {
    const orderedChildren =
        this.convertElementsToSortedHTMLElements(parent.children);
    for (const child of orderedChildren) {
      if (this.tryFocus(child)) {
        return true;
      }
      if (this.hasFocusableChild(child)) {
        return true;
      }
    }
    return false;
  }

  /**
   * Converts list of Elements or HTMLCollection to a list of HTMLElements
   * sorted in an attempt to honor tab order. For tabIndex, positive values are
   * tabbable in assending order. Followed by elements with tabIndex=0. In sort,
   * NaN is treated as zero.  Finally, elements
   * with tabIndex=-1 are not tabbable and thus filtered out. See:
   * http://go/mdn/HTML/Global_attributes/tabindex
   */
  private convertElementsToSortedHTMLElements(elements: Element[]|
                                              HTMLCollection) {
    if (elements.length === 0) {
      return [] as HTMLElement[];
    }
    const sorted =
        Array.from(elements)
            .map(el => el as HTMLElement)
            // Remove elements which are not tabbable (i.e. tabIndex=-1) or
            // became undefined when casting to a HTMLElement.
            .filter(
                el => el &&
                    (!el.hasAttribute('tabindex') ||
                     (el.hasAttribute('tabindex') && el.tabIndex > -1))) ??
        [];
    sorted.sort((a, b) => {
      if (a.tabIndex !== b.tabIndex && a.tabIndex === 0) {
        return 1;
      }

      if (a.tabIndex !== b.tabIndex && b.tabIndex === 0) {
        return -1;
      }

      return a.tabIndex - b.tabIndex;
    });
    return sorted;
  }

  /**
   * Attempts to set the active element to the first focusable child of the
   * card's default slot.
   */
  private attemptToFocusOnNestedElement() {
    const slot =
        this.renderRoot.querySelector<HTMLSlotElement>('#content > slot');
    const slottedItems =
        this.convertElementsToSortedHTMLElements(slot!.assignedElements());
    for (const element of slottedItems) {
      if (this.tryFocus(element)) {
        return;
      }
      if (this.hasFocusableChild(element)) {
        return;
      }
    }
  }
}

customElements.define('cros-card', Card);

declare global {
  interface HTMLElementTagNameMap {
    'cros-card': Card;
  }
}
