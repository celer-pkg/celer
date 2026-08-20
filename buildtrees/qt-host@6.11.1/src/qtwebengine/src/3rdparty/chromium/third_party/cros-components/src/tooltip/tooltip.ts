/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';

/**
 * Allow popover show and hide functions. See
 * https://developer.mozilla.org/en-US/docs/Web/API/Popover_API
 * go/typescript-reopening
 */
declare interface HTMLElementWithPopoverAPI extends HTMLElement {
  showPopover: () => void;
  hidePopover: () => void;
}

// The name of the anchor to use for the CSS Anchor Positioning API.
const ANCHOR_NAME = css`--tooltip-anchor`;

// Attribute names set for when the tooltip label should be shifted to remain
// on-screen.
const ATTR_SHIFT_INLINE_END = 'shift-inline-end';
const ATTR_SHIFT_INLINE_START = 'shift-inline-start';

// The attribute names here are used in the CSS below. For security reasons, the
// constants above cannot be substituted into the CSS, hence the duplicated
// literals here.
const ATTR_SHIFT_INLINE_END_CSS = css`shift-inline-end`;
const ATTR_SHIFT_INLINE_START_CSS = css`shift-inline-start`;

// The amount of time to wait before closing the tooltip, after the user stopped
// focusing/hovering the anchor or label.
const BLUR_OR_UNFOCUS_TIMEOUT_DURATION = 200;

/** A cros-compliant tooltip component. */
export class Tooltip extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    /* Remove default CSS Popover API styles. */
    :host {
      border: 0;
      margin: 0;
      padding: 0;
    }

    :host(:not(:popover-open)) {
      opacity: 0;
      transition: all 200ms allow-discrete;
    }

    :host(:popover-open) {
      opacity: 1;
      transition: all 200ms allow-discrete;
    }

    @starting-style {
      :host(:popover-open) {
        opacity: 0;
      }
    }

    #tooltip-anchor-overlay {
      anchor-name: ${ANCHOR_NAME};
      opacity: 0;
      pointer-events: none;
      position: fixed;
    }

    #label {
      background-color: var(--cros-sys-inverse_surface);
      border-radius: 6px;
      color: var(--cros-sys-surface);
      font: var(--cros-annotation-1-font);
      inset-block-start: var(--cros-tooltip-vertical-offset, 4px);
      max-width: 296px;
      padding: 5px 8px;
      position-anchor: ${ANCHOR_NAME};
      position-area: block-end span-all;
      position: fixed;
      text-align: center;
      text-wrap: wrap;
    }

    #label[${ATTR_SHIFT_INLINE_END_CSS}] {
      position-area: block-end span-inline-end;
      text-align: start;
    }

    #label[${ATTR_SHIFT_INLINE_START_CSS}] {
      position-area: block-end span-inline-start;
      text-align: start;
    }

    :host([truncate]) #label-inner {
      display: -webkit-box;
      -webkit-box-orient: vertical;
      -webkit-line-clamp: 3;
      overflow: hidden;
    }
  `;

  /** @nocollapse */
  static override properties = {
    anchor: {type: String},
    label: {type: String},
    truncate: {type: Boolean, reflect: true},
    followAnchorOnScroll:
        {type: Boolean, reflect: true, attribute: 'follow-anchor'},
  };

  /** @export */
  declare anchor: string;
  /** @export */
  declare label: string;
  /**
   * If true, the tooltip will be truncated with an ellipsis after 3 lines.
   * @export
   */
  declare truncate: boolean;
  /**
   * If true, the tooltip will follow the anchor element when the page scrolls.
   * @export
   */
  declare followAnchorOnScroll: boolean;

  constructor() {
    super();
    this.anchorElement = null;
    this.anchor = '';
    this.label = '';
    this.truncate = true;
    this.followAnchorOnScroll = false;

    this.anchorOrLabelFocused = this.anchorOrLabelFocused.bind(this);
    this.anchorOrLabelBlurred = this.anchorOrLabelBlurred.bind(this);
  }

  /** @export */
  get anchorElement(): HTMLElement|null {
    if (this.anchor) {
      return (this.getRootNode() as Document | ShadowRoot)
          .querySelector(`#${this.anchor}`);
    }
    return this.currentAnchorElement;
  }

  /** @export */
  set anchorElement(element: HTMLElement|null) {
    this.currentAnchorElement = element;
    this.requestUpdate('anchorElement');
  }

  private currentAnchorElement: HTMLElement|null = null;
  private isAnchorOrLabelFocused = false;
  private blurOrUnfocusTimeout: number|null = null;
  /**
   * The event listener for the document scroll event, bound to this element.
   */
  private onDocumentScrollBound: EventListener|null = null;

  override connectedCallback() {
    super.connectedCallback();
    this.onDocumentScrollBound = this.onDocumentScroll.bind(this);
  }

  override firstUpdated() {
    // Adds needed popover properties to cros-tooltip.
    this.setAttribute('popover', 'auto');
    this.setAttribute('id', 'tooltip');

    // Sets anchor to this element's ID & adds listeners on hover.
    if (this.anchorElement) {
      this.anchorElement.addEventListener(
          'mouseover', this.anchorOrLabelFocused);
      this.anchorElement.addEventListener('focusin', this.anchorOrLabelFocused);
      this.anchorElement.addEventListener(
          'mouseout', this.anchorOrLabelBlurred);
      this.anchorElement.addEventListener(
          'focusout', this.anchorOrLabelBlurred);
      if (this.onDocumentScrollBound) {
        document.addEventListener('scroll', this.onDocumentScrollBound);
      }
    }
  }

  override disconnectedCallback() {
    if (this.anchorElement) {
      this.anchorElement.removeEventListener(
          'mouseover', this.anchorOrLabelFocused);
      this.anchorElement.removeEventListener(
          'focus', this.anchorOrLabelFocused);
      this.anchorElement.removeEventListener(
          'mouseout', this.anchorOrLabelBlurred);
      this.anchorElement.removeEventListener('blur', this.anchorOrLabelBlurred);
    }
    if (this.onDocumentScrollBound) {
      document.removeEventListener('scroll', this.onDocumentScrollBound);
    }
    super.disconnectedCallback();
  }

  get labelElement(): HTMLDivElement {
    return this.shadowRoot!.querySelector<HTMLDivElement>('#label')!;
  }

  override updated(changedProperties: PropertyValues<this>) {
    if (changedProperties.has('label') && !!this.anchorElement) {
      this.anchorElement.ariaDescription = this.label;
    }
  }

  openPopover() {
    // When the popover opens, move the tooltip to the anchor.
    this.updateAnchorPosition();

    const tooltip = this as unknown as HTMLElementWithPopoverAPI;
    tooltip.showPopover();
  }

  closePopover() {
    const tooltip = this as unknown as HTMLElementWithPopoverAPI;
    tooltip.hidePopover();
  }

  private anchorOrLabelFocused(e: MouseEvent|FocusEvent) {
    // Touch events should not trigger tooltips. Note: Sometimes, touch long
    // presses can have `sourceCapabilities` set as undefined.
    const {sourceCapabilities} =
        e as unknown as {sourceCapabilities?: {firesTouchEvents?: boolean}};
    if (!sourceCapabilities || sourceCapabilities.firesTouchEvents) {
      return;
    }
    this.isAnchorOrLabelFocused = true;
    this.openPopover();
  }

  private anchorOrLabelBlurred() {
    // After a mouseout event, the anchor may still have keyboard focus, and the
    // tooltip should not close if the keyboard is still focused on the anchor.
    const anchorStillHasKeyboardFocus = this.anchorElement?.matches(':focus');
    if (!anchorStillHasKeyboardFocus) {
      this.isAnchorOrLabelFocused = false;
      this.maybeCloseTooltipAfterTimeout();
    }
  }

  private maybeCloseTooltipAfterTimeout() {
    // Restart the timeout if one already exists.
    if (this.blurOrUnfocusTimeout) {
      clearTimeout(this.blurOrUnfocusTimeout);
    }

    // Close the tooltip after a set amount of time, but don't close the tooltip
    // if the user is still focusing the label or the anchor.
    this.blurOrUnfocusTimeout = setTimeout(() => {
      if (!this.isAnchorOrLabelFocused) {
        this.closePopover();
      }
    }, BLUR_OR_UNFOCUS_TIMEOUT_DURATION);
  }

  // The CSS Anchor Positioning API does not yet allow elements in a shadow DOM
  // to see anchor names defined in the outer tree scope. To work around this,
  // there is an internal anchor point (#tooltip-anchor-overlay) that we
  // position to overlap exactly with the actual anchor element. From there, we
  // can use the CSS Anchor Positioning API to position the tooltip. See
  // https://github.com/w3c/csswg-drafts/issues/9408#issuecomment-2105453734 for
  // more context.
  private updateAnchorPosition() {
    if (!this.anchorElement) return;

    const anchorOverlay =
        this.shadowRoot!.getElementById('tooltip-anchor-overlay')!;
    const anchorRect = this.anchorElement.getBoundingClientRect();

    // Move the anchor overlay so that it overlaps the anchor element.
    anchorOverlay.style.top = `${anchorRect.top}px`;
    anchorOverlay.style.left = `${anchorRect.left}px`;
    anchorOverlay.style.width = `${anchorRect.width}px`;
    anchorOverlay.style.height = `${anchorRect.height}px`;

    // After the tooltip has been opened, test different alignments
    // (start/center/end) to determine which alignment allows for the largest
    // tooltip width.
    requestAnimationFrame(() => {
      const label = this.labelElement;

      // Move the label back to its normal position, centered under the anchor
      // element, and get its width + check if it's offscreen.
      label.removeAttribute(ATTR_SHIFT_INLINE_START);
      label.removeAttribute(ATTR_SHIFT_INLINE_END);
      const boundingClientRectWhenCentered = label.getBoundingClientRect();
      const widthWhenInCenter = boundingClientRectWhenCentered.width;
      const isOffscreenWhenInCenter =
          boundingClientRectWhenCentered.x + widthWhenInCenter >
              document.body.clientWidth ||
          boundingClientRectWhenCentered.x < 0;

      // Shift the label to the inline-end (i.e. right for ltr languages), and
      // get its width.
      label.setAttribute(ATTR_SHIFT_INLINE_END, '');
      const widthWhenShiftedToEnd = label.getBoundingClientRect().width;

      // Shift the label to the inline-start (i.e. left for ltr languages), and
      // get its width.
      label.removeAttribute(ATTR_SHIFT_INLINE_END);
      label.setAttribute(ATTR_SHIFT_INLINE_START, '');
      const widthWhenShiftedToStart = label.getBoundingClientRect().width;

      // Reset the label back to the center before applying the final
      // adjustments.
      label.removeAttribute(ATTR_SHIFT_INLINE_END);
      label.removeAttribute(ATTR_SHIFT_INLINE_START);

      if (!isOffscreenWhenInCenter &&
          widthWhenInCenter >= widthWhenShiftedToStart &&
          widthWhenInCenter >= widthWhenShiftedToEnd) {
        // Keep the element in the center.
        label.removeAttribute(ATTR_SHIFT_INLINE_START);
        label.removeAttribute(ATTR_SHIFT_INLINE_END);
      } else if (widthWhenShiftedToStart > widthWhenShiftedToEnd) {
        // Shift the element to the start.
        label.setAttribute(ATTR_SHIFT_INLINE_START, '');
      } else {
        // Shift the element to the end.
        label.setAttribute(ATTR_SHIFT_INLINE_END, '');
      }
    });
  }

  private onDocumentScroll() {
    if (this.followAnchorOnScroll) {
      this.updateAnchorPosition();
    }
  }

  override render() {
    return html`
      <div id="tooltip-anchor-overlay">
      </div>
      <div id="label"
          aria-hidden="true"
          role="tooltip"
          @mouseover=${this.anchorOrLabelFocused}
          @mouseout=${this.anchorOrLabelBlurred}
          @focus=${this.anchorOrLabelFocused}
          @blur=${this.anchorOrLabelBlurred}>
        <div id="label-inner">${this.label}</div>
    </div>
    `;
  }
}

customElements.define('cros-tooltip', Tooltip);

declare global {
  interface HTMLElementTagNameMap {
    'cros-tooltip': Tooltip;
  }
}
