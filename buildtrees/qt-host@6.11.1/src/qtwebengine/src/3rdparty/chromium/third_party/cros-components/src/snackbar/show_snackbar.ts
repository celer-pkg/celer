/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {Snackbar} from './snackbar';

/**
 * Allow popover show and hide functions. See
 * https://developer.mozilla.org/en-US/docs/Web/API/Popover_API
 * go/typescript-reopening
 */
export declare interface HTMLElementWithPopoverAPI extends Snackbar {
  showPopover: () => void;
  hidePopover: () => void;
}

/** Snackbar configuration options. */
export declare interface SnackbarOptions {
  /** The text to display in the snackbar message. */
  messageText: string;
  /**
   * Automatic dismiss timeout in milliseconds. Value must be between 4000 and
   * 10000, or it can be -1 to disable the timeout completely. Default 10000ms.
   */
  durationMs?: number;
  /** The text to display in the snackbar button. */
  buttonText?: string;
  /** The function to run when the snackbar button is clicked. */
  buttonAction?: () => void;
  /** The function to run when the snackbar button is closed. */
  onCloseAction?: () => void;
}

/** Typed wrapper for the details of an snackbar event. */
export declare interface ShowSnackbarEventDetail {
  /** Options for showSnackbar. */
  options: SnackbarOptions;
}

/** Dispatches a new show snackbar event with the options. */
export function showSnackbar(options: SnackbarOptions) {
  document.body.dispatchEvent(new CustomEvent(
      'cros-show-snackbar',
      {detail: {options}, bubbles: true, composed: true}));
}

/** Dispatches a new close snackbar event. */
export function closeSnackbar() {
  document.body.dispatchEvent(
      new CustomEvent('cros-close-snackbar', {bubbles: true, composed: true}));
}

declare global {
  interface HTMLElementEventMap {
    'cros-show-snackbar': CustomEvent<ShowSnackbarEventDetail>;
    'cros-close-snackbar': CustomEvent;
  }
}
