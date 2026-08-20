/**
 * @license
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import 'jasmine';

import {waitForEvent} from 'google3/third_party/javascript/cros_components/async_helpers/async_helpers';

import {ShowSnackbarEventDetail, SnackbarOptions} from './show_snackbar';

/**
 * Returns a Promise that watches for the cros-show-snackbar event to appear on
 * document, and resolves as the SnackbarOptions.
 */
export function waitForSnackbar(): Promise<SnackbarOptions> {
  return waitForEvent(document.body, 'cros-show-snackbar')
      .then(
          (event) => event.detail.options,
      );
}

/** Returns a state object for tracking all Snackbars shown. */
export function useSnackbarWatcher() {
  const state = {shown: [] as SnackbarOptions[], closed: [] as object[]};

  function onShow(event: CustomEvent<ShowSnackbarEventDetail>) {
    state.shown.push(event.detail.options);
  }
  function onClose(event: CustomEvent) {
    state.closed.push(event.detail);
  }

  beforeEach(() => {
    state.shown = [];
    state.closed = [];
    document.body.addEventListener('cros-show-snackbar', onShow);
    document.body.addEventListener('cros-close-snackbar', onClose);
  });

  afterEach(() => {
    document.body.removeEventListener('cros-show-snackbar', onShow);
    document.body.removeEventListener('cros-close-snackbar', onClose);
  });

  return state;
}
