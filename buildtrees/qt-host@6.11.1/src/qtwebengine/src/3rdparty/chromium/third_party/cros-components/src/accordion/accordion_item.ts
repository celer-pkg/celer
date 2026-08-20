/**
 * @license
 * Copyright 2024 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import '../icon_button/icon-button';
import '../tooltip/tooltip';
import '@material/web/focus/md-focus-ring.js';

import {css, CSSResultGroup, html, LitElement, nothing, PropertyValues} from 'lit';
import {ClassInfo, classMap} from 'lit/directives/class-map';
import {StyleInfo, styleMap} from 'lit/directives/style-map';

/**
 * Fired when an accordion item is expanded.
 */
export type AccordionItemExpandedEvent = CustomEvent<{
  accordionItem: AccordionItem,
}>;

/**
 * Fired when an accordion item is collapsed.
 */
export type AccordionItemCollapsedEvent = CustomEvent<{
  accordionItem: AccordionItem,
}>;

/**
 * Variants control how the accordion-item should be styled.
 */
export type AccordionItemVariant = 'default'|'compact'|'title-only';

/**
 * The SVG to use in the accordion item's icon button when the accordion row is
 * collapsed.
 */
const CHEVRON_DOWN_ICON = html`
    <svg
        width="20"
        height="20"
        viewBox="0 0 20 20"
        xmlns="http://www.w3.org/2000/svg"
        slot="icon"
        class="chevron-icon">
      <path
          fill-rule="evenodd"
          clip-rule="evenodd"
          d="M5.41 6L10 10.9447L14.59 6L16 7.52227L10 14L4 7.52227L5.41 6Z" />
    </svg>`;

/**
 * The SVG to use in the accordion item's icon button when the accordion row is
 * expanded.
 */
const CHEVRON_UP_ICON = html`
    <svg
        width="20"
        height="20"
        viewBox="0 0 20 20"
        xmlns="http://www.w3.org/2000/svg"
        slot="icon"
        class="chevron-icon">
      <path
          fill-rule="evenodd"
          clip-rule="evenodd"
          d="M5.41 14L10 9.05533L14.59 14L16 12.4777L10 6L4 12.4777L5.41 14Z" />
    </svg>`;

const LEADING_PADDING =
    css`var(--cros-accordion-item-leading-padding-inline-end, 16px)`;

// Anchor id for the button tooltip.
const BUTTON_ANCHOR_ID = 'button-anchor';

/** A chromeOS compliant accordion-item for use in <cros-accordion>. */
export class AccordionItem extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    .accordion-row {
      align-items: center;
      box-sizing: border-box;
      cursor: pointer;
      display: flex;
      flex-direction: row;
      justify-content: space-between;
      outline: none;
      padding: 16px;
      padding-inline-end: 12px;
      position: relative;
    }

    .leading {
      align-items: center;
      display: flex;
      justify-content: center;
    }

    :host([variant="title-only"]) .accordion-row {
      padding-block: 8px;
      padding-inline: 16px 12px;
    }

    :host([variant="compact"]) {
      .accordion-row {
        padding-block: 8px;
        padding-inline: 14px 8px;
      }

      .title {
        font: var(--cros-button-2-font);
      }
    }

    .title-and-subtitle {
      flex: 1;
      padding-inline-end: ${LEADING_PADDING};
    }

    .has-leading .title-and-subtitle {
      padding-inline-start: ${LEADING_PADDING};
    }

    .title {
      color: var(--cros-sys-on_surface);
      font: var(--cros-title-1-font);
    }

    .subtitle {
      color: var(--cros-sys-on_surface_variant);
      font: var(--cros-body-2-font);
    }

    .content {
      overflow: hidden;
      transition-duration: 300ms;
      transition-property: height;
      transition-timing-function: cubic-bezier(0.40, 0.00, 0.00, 0.97);
    }

    @media (prefers-reduced-motion: reduce) {
      section.content {
        transition-duration: 0s;
      }
    }

    .content:not([data-expanded]) {
      min-height: 0;
      height: 0;
    }

    .content:not([data-expanded]):not([data-transitioning]) {
      visibility: hidden;
    }

    .content[data-expanded] {
      height: var(--cros-accordion-item-content-height);
    }

    .content[data-expanded]:not([data-transitioning]) {
      height: auto;
    }

    .content-inner {
      padding: 16px;
      padding-block-start: 0;
      padding-inline-end: 12px;
    }

    .container::after {
      background: var(--cros-sys-separator);
      content: '';
      display: var(--cros-accordion-item-separator-display, block);
      height: 1px;
      width: 100%;
    }

    .chevron-icon {
      fill: var(--cros-sys-on_surface);
    }

    md-focus-ring {
      --md-focus-ring-active-width: 2px;
      --md-focus-ring-color: var(--cros-sys-focus_ring);
      --md-focus-ring-duration: 0s;
      --md-focus-ring-shape: 12px;
      --md-focus-ring-width: 2px;
    }

    :host([disabled]) {
      opacity: var(--cros-disabled-opacity);
      pointer-events: none;
    }
  `;

  /** @nocollapse */
  static override properties = {
    disabled: {type: Boolean, reflect: true},
    expanded: {type: Boolean, reflect: true},
    quick: {type: Boolean, reflect: true},
    variant: {type: String, reflect: true},
    showButtonTooltip:
        {type: Boolean, reflect: true, attribute: 'show-button-tooltip'},
    buttonTooltipLabel:
        {type: String, reflect: true, attribute: 'button-tooltip-label'},
  };

  /** @nocollapse */
  static events = {
    /** Triggers when an accordion item is expanded. */
    ACCORDION_ITEM_EXPANDED: 'cros-accordion-item-expanded',
    /** Triggers when an accordion item is collapsed. */
    ACCORDION_ITEM_COLLAPSED: 'cros-accordion-item-collapsed',
  } as const;

  /**
   * Whether or not the accordion item is disabled.
   * @export
   */
  declare disabled: boolean;

  /**
   * Whether or not the accordion content is visible.
   * @export
   */
  declare expanded: boolean;

  /**
   * Whether or not to skip animations.
   * @export
   */
  declare quick: boolean;

  // TODO(b/352151998): The `compact` variant should also change the
  // leading icon size.
  /**
   * How the accordion-item should be styled. One of {default, compact,
   * title-only}.
   * @export
   */
  declare variant: AccordionItemVariant;

  /**
   * Whether or not to show a tooltip on the arrow button.
   * @export
   */
  declare showButtonTooltip: boolean;

  /**
   * The label for the button tooltip.
   * @export
   */
  declare buttonTooltipLabel: string;

  constructor() {
    super();
    this.disabled = false;
    this.expanded = false;
    this.quick = false;
    this.variant = 'default';
    this.showButtonTooltip = false;
    this.buttonTooltipLabel = '';
  }

  override willUpdate(changedProperties: PropertyValues<this>) {
    super.willUpdate(changedProperties);

    if (changedProperties.has('expanded')) {
      this.setTransitioning(true);

      const eventName = this.expanded ?
          AccordionItem.events.ACCORDION_ITEM_EXPANDED :
          AccordionItem.events.ACCORDION_ITEM_COLLAPSED;
      this.dispatchEvent(new CustomEvent(
          eventName,
          {bubbles: true, composed: true, detail: {accordionItem: this}}));
    }
  }

  override render() {
    const containerClasses: ClassInfo = {
      'has-leading': this.hasLeading(),
    };

    const contentStyle: StyleInfo = {
      '--cros-accordion-item-content-height': `${this.contentHeight}px`
    };

    return html`
      <div class="container ${classMap(containerClasses)}" >
        <div
            class="accordion-row"
            id="accordion-row"
            part="row"
            aria-expanded=${this.expanded ? 'true' : 'false'}
            aria-controls="content-inner"
            aria-labelledby="title-and-subtitle"
            role="button"
            @click=${this.onRowClick}
            @keydown=${this.onRowKeyDown}
            tabindex=${this.disabled ? -1 : 0}>
          ${this.renderLeading()}
          <section
              aria-hidden="true"
              class="title-and-subtitle"
              id="title-and-subtitle">
            <div class="title">
              <slot name="title"></slot>
            </div>
            ${this.renderSubtitle()}
          </section>
          <cros-icon-button
            id=${BUTTON_ANCHOR_ID}
            tabindex="-1"
            aria-hidden="true"
            buttonStyle="floating"
            size=${this.variant === 'compact' ? 'small' : 'default'}
            surface="base"
            shape="square">
              ${this.expanded ? CHEVRON_UP_ICON : CHEVRON_DOWN_ICON}
          </cros-icon-button>
          ${this.renderButtonTooltip()}
          <md-focus-ring inward></md-focus-ring>
        </div>
        <section class="content"
            @transitionend=${this.onTransitionEnd}
            style=${styleMap(contentStyle)}
            ?data-expanded=${this.expanded ?? nothing}>
          <div
              aria-labelledby="title-and-subtitle"
              class="content-inner"
              id="content-inner"
              part="content"
              role="region">
            <slot></slot>
          </div>
        </section>
      </div>
    `;
  }

  private renderLeading() {
    if (this.variant === 'title-only') {
      return nothing;
    }

    return html`
      <div class="leading">
        <slot name="leading" @slotchange=${this.onLeadingSlotChange}>
        </slot>
      </div>
    `;
  }

  private renderSubtitle() {
    if (this.variant === 'title-only' || this.variant === 'compact') {
      return nothing;
    }

    return html`
      <div class="subtitle">
        <slot name="subtitle"></slot>
      </div>
    `;
  }

  private renderButtonTooltip() {
    if (!this.showButtonTooltip) {
      return nothing;
    }

    return html`
      <cros-tooltip anchor=${BUTTON_ANCHOR_ID} label=${this.buttonTooltipLabel}>
      </cros-tooltip>
    `;
  }

  /**
   * Returns true if the accordion item has content in its leading slot.
   */
  private hasLeading(): boolean {
    return (this.shadowRoot
                ?.querySelector<HTMLSlotElement>('slot[name="leading"]')
                ?.assignedElements()
                .length ??
            0) > 0;
  }

  // The padding before the title/subtitle changes based on whether there is
  // content in the `leading` slot. It's currently not possible to detect slot
  // content via CSS. When the `leading` slot changes, we request an update to
  // force the element to re-render and thus apply the `has-leading` class.
  private onLeadingSlotChange() {
    this.requestUpdate();
  }

  private onRowClick() {
    this.toggleExpanded();
  }

  private onRowKeyDown(e: KeyboardEvent) {
    if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault();
      this.toggleExpanded();
    }
  }

  /**
   * Returns the height of the accordion item's content, for the purposes of
   * setting a fixed height so that a CSS transition is possible.
   */
  private get contentHeight() {
    return this.shadowRoot?.querySelector('.content-inner')?.clientHeight ?? 0;
  }

  private onTransitionEnd() {
    this.setTransitioning(false);
  }

  private setTransitioning(isTransitioning: boolean) {
    if (this.quick) {
      return;
    }

    // If the user has set their system to prefer reduced motion, we
    // should skip applying the `data-transitioning` attribute. Note that
    // the CSS above also has a media query to skip the transition.
    const prefersReducedMotion =
        window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    if (prefersReducedMotion) {
      return;
    }

    const contentElement = this.shadowRoot?.querySelector('.content');
    if (isTransitioning) {
      contentElement?.setAttribute('data-transitioning', '');
    } else {
      contentElement?.removeAttribute('data-transitioning');
    }
  }

  private toggleExpanded() {
    if (this.disabled) {
      return;
    }

    this.expanded = !this.expanded;
  }
}

customElements.define('cros-accordion-item', AccordionItem);

declare global {
  interface HTMLElementEventMap {
    [AccordionItem.events.ACCORDION_ITEM_EXPANDED]: AccordionItemExpandedEvent;
    [AccordionItem.events.ACCORDION_ITEM_COLLAPSED]:
        AccordionItemCollapsedEvent;
  }
  interface HTMLElementTagNameMap {
    'cros-accordion-item': AccordionItem;
  }
}
