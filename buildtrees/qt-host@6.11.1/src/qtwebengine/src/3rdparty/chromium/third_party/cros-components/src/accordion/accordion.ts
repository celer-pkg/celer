/**
 * @license
 * Copyright 2024 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '../card/card';

import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';

import {CardStyle} from '../card/card';

import {AccordionItem, type AccordionItemExpandedEvent, type AccordionItemVariant} from './accordion_item';

/** A chromeOS compliant accordion. */
export class Accordion extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
      cros-card {
        --cros-card-padding: 0;
      }

      ::slotted(cros-accordion-item:last-of-type) {
        --cros-accordion-item-separator-display: none;
      }
  `;

  /** @nocollapse */
  static override properties = {
    autoCollapse: {type: Boolean, attribute: 'auto-collapse'},
    quick: {type: Boolean},
    variant: {type: String},
    cardStyle: {type: String, attribute: 'card-style'},
  };

  /**
   * When true, opening a child `<cros-accordion-item>` in this component will
   * close any other open `<cros-accordion-item>`s. When false, allow multiple
   * `<cros-accordion-item>`s to be open at once.
   * @export
   */
  declare autoCollapse: boolean;

  /**
   * When true, all child `<cros-accordion-item>`s within this
   * `<cros-accordion>` will skip animations. This overrides the `quick`
   * property set on any child `<cros-accordion-item>`s.
   * @export
   */
  declare quick: boolean;

  /**
   * How each accordion-item child should be styled. One of {default,
   * compact, title-only}.
   * @export
   */
  declare variant: AccordionItemVariant;

  /**
   * The style of the card. One of {outline, filled, elevated}.
   * @export
   */
  declare cardStyle: CardStyle;

  constructor() {
    super();
    this.autoCollapse = false;
    this.quick = false;
    this.variant = 'default';
    this.cardStyle = 'outline';

    this.addEventListener(
        AccordionItem.events.ACCORDION_ITEM_EXPANDED,
        (e: AccordionItemExpandedEvent) => this.onAccordionItemExpanded(e));
  }

  override updated(changedProperties: PropertyValues<this>) {
    if (changedProperties.has('quick')) {
      // Set the `quick` property on all child `<cros-accordion-item>`s.
      for (const child of this.children) {
        if (child instanceof AccordionItem) {
          child.quick = this.quick;
        }
      }
    }

    if (changedProperties.has('variant')) {
      // Set the `variant` property on all child `<cros-accordion-item>`s.
      for (const child of this.children) {
        if (child instanceof AccordionItem) {
          child.variant = this.variant;
        }
      }
    }
  }

  override render() {
    return html`
      <cros-card cardStyle=${this.cardStyle} part="card">
        <slot></slot>
      </cros-card>
    `;
  }

  private onAccordionItemExpanded(e: AccordionItemExpandedEvent) {
    if (this.autoCollapse) {
      this.closeOtherAccordionItems(e.detail.accordionItem);
    }
  }

  private closeOtherAccordionItems(accordionItem: AccordionItem) {
    for (const child of this.children) {
      if (child instanceof AccordionItem && child !== accordionItem) {
        child.expanded = false;
      }
    }
  }
}

customElements.define('cros-accordion', Accordion);

declare global {
  interface HTMLElementTagNameMap {
    'cros-accordion': Accordion;
  }
}
