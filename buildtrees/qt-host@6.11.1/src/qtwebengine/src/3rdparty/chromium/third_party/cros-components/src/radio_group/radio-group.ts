/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {RadioChip} from 'google3/third_party/javascript/cros_components/chip/radio-chip';
import {css, CSSResultGroup, html, LitElement} from 'lit';

/**
 * A chromeOS compliant radio group.
 */
export class RadioGroup extends LitElement {
  static override styles: CSSResultGroup = css`
    :host {
      display: inline-block;
    }
  `;

  override render() {
    return html`
      <div id="group" part="group">
        <slot @slotchange=${this.slotChange}></slot>
      </div>
    `;
  }

  private chipArray: RadioChip[] = [];

  override firstUpdated() {
    this.shadowRoot!.querySelector('slot')!.addEventListener(
        'click', this.clickListener);
  }

  private slotChange() {
    const elements = this.renderRoot.querySelector('slot')!.assignedElements();
    this.chipArray = elements.filter(isRadioChip) as RadioChip[];
  }

  private readonly clickListener = (event: Event) => {
    const chip = event.target as RadioChip;
    this.chipArray.forEach(chip => {
      chip.selected = false;
    });
    chip.selected = true;
  };
}

function isRadioChip(element: unknown): element is RadioChip {
  return element instanceof Element && element.tagName === 'CROS-RADIO-CHIP';
}

customElements.define('cros-radio-group', RadioGroup);

declare global {
  interface HTMLElementTagNameMap {
    'cros-radio-group': RadioGroup;
  }
}
