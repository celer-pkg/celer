/**
 * @license
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {MdOutlinedTextField} from '@material/web/textfield/outlined-text-field.js';
import {cast, castExists} from 'google3/javascript/common/asserts/asserts';
import {isInstanceOf} from 'google3/javascript/common/asserts/guards';
import {IconButton} from 'google3/third_party/javascript/cros_components/icon_button/icon-button';
import {IconButtonHarness as MdIconButtonHarness} from 'google3/third_party/javascript/material/web/iconbutton/harness';
import {TextFieldHarness as MdTextFieldHarness} from 'google3/third_party/javascript/material/web/textfield/harness';

import {Textfield} from './textfield';

/** Get the internal `md-textfield` element. */
export function getMdTextfield(textfield: Textfield): MdOutlinedTextField {
  return cast(
      textfield.renderRoot.querySelector('md-outlined-text-field'),
      isInstanceOf(MdOutlinedTextField));
}

function getMDIconHarness(iconButton: IconButton) {
  const mdIconButton =
      castExists(iconButton.renderRoot.querySelector('md-icon-button'));
  return new MdIconButtonHarness(mdIconButton);
}

/** Harness for the `cros-textfield` element. */
export class TextfieldHarness extends MdTextFieldHarness {
  constructor(readonly textfield: Textfield) {
    super(getMdTextfield(textfield));
  }

  override async startHover() {
    const icons = [...this.textfield.querySelectorAll('cros-icon-button')];
    const harnesses = icons.map(
        icon => getMDIconHarness(cast(icon, isInstanceOf(IconButton))));
    await Promise.all(harnesses.map(harness => harness.startHover()));
  }
}

/** Get a harness for the `cros-textfield` element. */
export function getHarnessFor(textfield: Textfield) {
  return new MdTextFieldHarness(getMdTextfield(textfield));
}

/**
 * Change events are only fired on blur, so for accuracy here we simulate the
 * full sequence of events on the textfield.
 */
export async function simulateInputChange(
    harness: MdTextFieldHarness, input: string) {
  await harness.focusWithKeyboard();
  await harness.inputValue(input);
  await harness.blur();
}
