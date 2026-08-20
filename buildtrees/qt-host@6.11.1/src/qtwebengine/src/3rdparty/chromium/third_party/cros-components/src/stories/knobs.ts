/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import 'google3/third_party/javascript/cros_components/checkbox/checkbox';

import {Knob, KnobUi} from 'google3/javascript/lit/stories';
import {html} from 'lit';

/**
 * A library of KnobUI types that utilize CrosComponents.
 * These allow the Jellybean team to pilot and use their own UI in the demo and
 * Web Story Catalog.
 */

/**
 * An type of KnobUi function that uses <cros-checkbox>.
 */
export class CrosCheckboxKnob implements KnobUi<boolean> {
  render(knob: Knob<boolean>, onChange: (val: boolean) => void) {
    const valueChanged = (e: Event) => {
      onChange((e.target as HTMLInputElement).checked);
    };

    // The KnobUI doesn't appear to support a style tag or styleMap, it renders
    // it as plain HTML, so for now we just use plain strings.
    const fontFamily = 'font-family: var(--cros-button-2-font-family);';
    const fontSize = 'font-size: var(--cros-button-2-font-size);';
    return html`
        <div>
          <cros-checkbox
              .checked=${!!knob.latestValue}
              @change="${valueChanged}">
          </cros-checkbox>
          <div id="label" style="${fontFamily + fontSize}">${knob.name}</div>
        </div>
      `;
  }

  deserialize(serialized: string) {
    return serialized !== 'false';
  }

  serialize(bool: boolean): string {
    return bool.toString();
  }
}
