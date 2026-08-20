/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @fileoverview Defines the SnackbarManager class, which manages the operation
 * of the snackbar apps.
 *
 */

import './snackbar';

import {html, LitElement} from 'lit';

import {Button} from '../button/button';

import {HTMLElementWithPopoverAPI, ShowSnackbarEventDetail, SnackbarOptions} from './show_snackbar';

/**
 * SnackbarManager is responsible for handling requests to show the snackbar
 * across the entire app.
 */
export class SnackbarManager extends LitElement {
  private readonly showEventHandler =
      (event: CustomEvent<ShowSnackbarEventDetail>) => {
        this.showSnackbar(event.detail.options);
      };
  private readonly closeEventHandler = () => {
    this.closeSnackbar();
  };

  /** Function to run when the snackbar button is clicked. */
  private buttonAction?: () => void;
  /** Function to run when the snackbar is closed. */
  private onCloseAction?: () => void;

  override connectedCallback() {
    super.connectedCallback();
    document.body.addEventListener('cros-show-snackbar', this.showEventHandler);
    document.body.addEventListener('cros-close-snackbar', this.closeEventHandler);
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    document.body.removeEventListener('cros-show-snackbar', this.showEventHandler);
    document.body.removeEventListener('cros-close-snackbar', this.closeEventHandler);
  }

  private get snackbar() {
    return this.renderRoot!.querySelector('cros-snackbar') as
        HTMLElementWithPopoverAPI;
  }

  private get button() {
    return this.renderRoot!.querySelector('cros-button') as Button;
  }

  /**
   * Shows a snackbar with a given message and optional button. Snackbars will
   * not stack, and this function will force close and override any previous
   * snackbar regardless of whether it is currently opened or closed.
   * @param options The configuration options for the snackbar.
   */
  showSnackbar(options: SnackbarOptions) {
    if (this.snackbar.open) {
      // Close the snackbar to restart the snackbar timer.
      this.snackbar.hidePopover();
    }
    this.button.slot = options.buttonText ? 'action' : '';
    this.button.style.display = options.buttonText ? 'block' : 'none';
    this.button.label = options.buttonText || '';
    this.snackbar.message = options.messageText;
    this.snackbar.timeoutMs =
        (options.durationMs !== undefined) ? options.durationMs : 10000;
    this.buttonAction = options.buttonAction;
    this.onCloseAction = options.onCloseAction;
    this.snackbar.closeOnEscape = true;
    this.snackbar.showPopover();
  }

  override render() {
    return html`
        <cros-snackbar @toggle=${this.onToggle}>
          <cros-button
              slot="action"
              button-style="floating"
              inverted
              @click=${this.onButtonClick}>
          </cros-button>
        </cros-snackbar>
        `;
  }

  /** Close the snackbar if it is open. */
  closeSnackbar() {
    this.snackbar?.hidePopover();
  }

  private onToggle(event: ToggleEvent) {
    if (event.newState === 'closed') {
      // Calls the `onCloseAction` if it is defined.
      this.onCloseAction?.();
    }
  }

  private onButtonClick() {
    this.snackbar.hidePopover();
    if (this.buttonAction) this.buttonAction();
  }
}

customElements.define('cros-snackbar-manager', SnackbarManager);

declare global {
  interface HTMLElementTagNameMap {
    'cros-snackbar-manager': SnackbarManager;
  }
}
