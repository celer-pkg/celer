/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import './snackbar-item';

import {css, CSSResultGroup, html, LitElement} from 'lit';

/**
 * Event interface for snackbar toggle events. Closure fires off the wrong event
 * type that is missing these fields.
 * TODO: b/307830635 - Remove once ToggleEvents are supported.
 */
declare interface ToggleEvent extends Event {
  newState: 'open'|'closed';
  oldState: 'open'|'closed';
}

/** A chromeOS compliant snackbar. */
export class Snackbar extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host(:popover-open) {
      background: none;
      border: none;
    }

    #container {
      bottom: 0px;
      left: 0px;
      padding-bottom: 24px;
      padding-inline-start: 24px;
      position: fixed;
      width: 100%;
      box-sizing: border-box;
    }
  `;

  /**
   * The visible message on the snackbar surface.
   * @export
   */
  declare message: string;

  /** @export */
  override role = 'alert';

  /** @export */
  declare closeOnEscape: boolean;

  /** @export */
  /** Timeout between the snackbar showing and hiding, in milliseconds. */
  declare timeoutMs: number;

  /** @nocollapse */
  static override properties = {
    message: {type: String, reflect: true},
    closeOnEscape: {type: Boolean, reflect: true},
    timeoutMs: {type: Number, reflect: true},
  };

  /** @nocollapse */
  static events = {
    /** The snackbar popup timed out and closed itself. */
    TIMEOUT: 'cros-snackbar-timeout',
  } as const;

  constructor() {
    super();
    this.message = '';
    this.closeOnEscape = false;
    this.timeoutMs = -1;
    this.popover = 'manual';
  }

  /** @export */
  get open() {
    return this.openInternal;
  }

  private pendingTimeout: number|null = null;
  private openInternal = false;

  private readonly handleKeyDown = (event: KeyboardEvent) => {
    if (this.openInternal && event.key === 'Escape' && this.closeOnEscape) {
      this.hidePopover();
    }
  };

  override connectedCallback() {
    super.connectedCallback();
    this.addEventListener('toggle', (e) => this.onToggle(e as ToggleEvent));
    document.addEventListener('keydown', this.handleKeyDown);
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    document.removeEventListener('keydown', this.handleKeyDown);
  }

  override render() {
    return html`
        <div id="container">
          <cros-snackbar-item message=${this.message}>
            <slot name="icon" slot="icon"></slot>
            <slot
                name="action"
                slot="action"
                @slotchange=${this.handleSlotChanged}>
            </slot>
            <slot
                name="dismiss"
                slot="dismiss"
                @slotchange=${this.handleSlotChanged}>
            </slot>
          </cros-snackbar-item>
        </div>
    `;
  }

  private onToggle(e: ToggleEvent) {
    this.openInternal = e.newState === 'open';
    if (e.newState === 'open') {
      // `toggle` events are coalesced:
      // https://developer.mozilla.org/en-US/docs/Web/API/HTMLElement/toggle_event#a_note_on_toggle_event_coalescing.
      // Clearing the timeout when toggling from 'open' to 'open' allows
      // `hidePopover()` and `showPopover()` to be called in the same event
      // loop cycle to refresh the popover timeout.
      if (e.oldState === 'open') {
        this.clearTimeout();
      }
      this.startTimeout();
    } else if (e.newState === 'closed') {
      // If the snackbar was closed before the timer fired cancel it.
      this.clearTimeout();
    }
  }

  private clearTimeout() {
    if (this.pendingTimeout) {
      clearTimeout(this.pendingTimeout);
      this.pendingTimeout = null;
    }
  }

  private startTimeout() {
    if (this.timeoutMs < 1 || this.pendingTimeout) return;

    this.pendingTimeout = setTimeout(() => {
      this.hidePopover();
      this.dispatchEvent(new CustomEvent(Snackbar.events.TIMEOUT));
      this.clearTimeout();
    }, this.timeoutMs);
  }

  private handleSlotChanged() {
    const hasActionButtons =
        this.querySelectorAll(`*[slot="action"], [slot="dismiss"]`);
    for (const snackbarItem of this.shadowRoot!.querySelectorAll(
             `cros-snackbar-item`)) {
      snackbarItem.classList.toggle(
          'has-action-button', hasActionButtons.length > 0);
    }
  }
}

customElements.define('cros-snackbar', Snackbar);

declare global {
  interface HTMLElementTagNameMap {
    'cros-snackbar': Snackbar;
  }
  interface HTMLElementEventMap {
    [Snackbar.events.TIMEOUT]: Event;
  }
}
