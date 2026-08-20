/**
 * @license
 * Copyright 2023 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {addColorSchemeChangeListener, removeColorSchemeChangeListener} from 'google3/third_party/py/chrome_styles/ts_helpers';
import {css, CSSResultGroup, html, LitElement, PropertyValues} from 'lit';
import {templateContent} from 'lit/directives/template-content';

const SVG_CONTENT_TYPE = 'image/svg+xml';

function isAbortError(error: unknown): boolean {
  return (error as Error).name === 'AbortError';
}

/**
 * A static illustration component for illustrations. This component supports:
 *   - External non dynamic color images (app logos for example).
 *   - Externally fetched SVGs with dynamic color tokens references.
 *   - Local HTMLTemplateElements.
 *
 * The external url will override any local template provided, eg the following
 * will result in ONLY the external SVG being fetched and rendererd:
 *
 * <cros-static-illustration url='<url>' .template=${templateRef}>
 *
 * To render a local HTMLTemplateElement, make sure there is no url set:
 * <cros-static-illustration .template=${templateRef}>
 *
 * A build rule "gen_gm3_illustrations" has been provided which takes in a
 * directory of GM3 .svg assets and compiles to a HTMLTemplateElement.
 *
 * In your BUILD file:
 *
 * load("//third_party/javascript/cros_components/static_illustration:build_defs.bzl",
 * "gen_gm3_illustrations")
 *
 * gen_gm3_illustrations(
 *  name = "gm3_assets",
 *  srcs = glob(["gm3_assets/**"]),
 * )
 *
 */
export class StaticIllustration extends LitElement {
  /** @nocollapse */
  static override properties = {
    url: {type: String},
    cachedExternalSVG: {type: String},
    imgSrc: {type: String},
    alt: {type: String},
    forceColor: {type: String},
    template: {attribute: false, type: HTMLTemplateElement}
  };

  /**
   * A url to a external static illustration.
   * @export
   */
  declare url: string|null;
  /**
   * Accessible text to inform screenreaders about this asset. If omitted it's
   * assumed the illustration is solely for visual aesthetic and will be aria
   * hidden to not clutter the a11y tree.
   * @export
   */
  declare alt: string|null;
  /**
   * A template containing a local SVG file. This will only be rendered if no
   * url to an external asset has been provided.
   * @export
   */
  declare template: HTMLTemplateElement;
  /**
   * If and only if `url` points to a SVG file all `fill`s in the svg will be
   * mapped to css variable specified in `fillColor`. Allows for legacy single
   * color svg's to be made dynamic. Pass the variable in as `--cros-name`.
   * @export
   */
  declare forceColor: string|null;

  /**
   * If `url` points at a external svg file from `url` we cache it so we can
   * recolor it without having to repeat the network request.
   */
  private declare cachedExternalSVG: string|null;
  /**
   * A src for a image tag that contains `url` but with after any necessary
   * dynamic color preprocessing.
   */
  private declare imgSrc: string|null;
  /**
   * Everytime we make a fetch we increment this token. This allows us to
   * gracefully handle the user changing the url multiple times before the first
   * request has had a chance to complete.
   */
  private currentFetchToken = 0;
  /** AbortController associated with a currently pending fetch request. */
  private abortController: AbortController|null = null;

  private readonly onColorChange = () => void this.recolorExternalAsset();
  private shouldRenderTemplate = false;


  /** @nocollapse */
  static override styles: CSSResultGroup = css`
    :host {
      display: block;
    }

    img, svg {
      height: 100%;
      object-fit: cover;
      width: 100%;
    }
  `;

  constructor() {
    super();
    this.url = null;
    this.cachedExternalSVG = null;
    this.imgSrc = null;
    this.alt = null;
    this.forceColor = null;
    this.template = document.createElement('template');
  }

  private recolorExternalAsset() {
    if (!this.cachedExternalSVG) {
      // If the user changed their color theme and we don't have an asset to
      // recolor we can silently exit. This isn't an error state however because
      // we expect this function to be called at any time the OS color theme
      // changes which can be before the element is initialized or when we
      // aren't rendering a external svg assert.
      return;
    }

    let resolvedSVG;
    const currentStyles = getComputedStyle(this);
    if (this.forceColor) {
      // Replaces all fills and strokes in the svg source i.e
      //     fill="#ff0000"
      //     <style> * { fill: #00ff00 } </style>
      // With `forceColor`, assuming forceColor resolves to #ff00ff:
      //     fill="#ff00ff"
      //     <style> * { fill: #ff00ff } </style>
      const resolvedForceColor =
          currentStyles.getPropertyValue(this.forceColor);
      resolvedSVG = this.cachedExternalSVG.replaceAll(
          /(fill|stroke)\s*=\s*['"][^'"]+['"]/gi,
          (match, attr) => `${attr} = "${resolvedForceColor}"`);
      resolvedSVG = resolvedSVG.replaceAll(
          /(fill|stroke)\s*:\s*[^;]+;/gi,
          (match, attr) => `${attr} : ${resolvedForceColor};`);
    } else {
      // Replaces references to css variables in the svg source i.e
      //     fill="var(--cros-sys-illo1)"
      //     <style> * { fill: var(--cros-sys-illo1); } </style>
      // With their actual hex values in the current color scheme.
      //     fill="#ff00ff"
      //     <style> * { fill: #ff00ff } </style>
      // This is because when rendering svg in a <img> tag the svg is isolated
      // from the page context and does not have access to any css variables.
      // Note that we also can't render external svg in page since it's
      // untrusted and would pose a XSS risk.
      resolvedSVG = this.cachedExternalSVG.replaceAll(
          /var\((--[\w\-_\.]+)\)/gi,
          (match, variable) => currentStyles.getPropertyValue(variable));
    }
    const b64 = btoa(resolvedSVG);
    this.imgSrc = `data:${SVG_CONTENT_TYPE};base64,${b64}`;
  }

  private async fetchUrl(url: string) {
    this.currentFetchToken++;
    const currentSession = this.currentFetchToken;

    // Abort a previous pending request.
    if (this.abortController) {
      this.abortController.abort();
      this.abortController = null;
    }
    let response: Response;
    try {
      this.abortController = new AbortController();
      const {signal} = this.abortController;
      response = await fetch(url, {signal});
    } catch (e: unknown) {
      if (isAbortError(e)) {
        // Request must have been aborted. Gracefully exit.
        return;
      }
      // Some other error happened. Rethrow.
      throw e;
    }

    // Check if while we were waiting for our response we sent out another
    // request, if so we ignore this response. Since we abort our previous
    // request this shouldn't ever happen but this catches the situation where
    // a response arrives in the period between us calling abort() and the
    // request actually be aborted.
    // See the dom spec (https://dom.spec.whatwg.org/#interface-abortcontroller)
    // which states that the abort() call will "store reason ... and signal to
    // any observers that the associated acitivity is to be aborted".
    if (this.currentFetchToken > currentSession) {
      return;
    }
    const code = response.status;
    const contentType = response.headers.get('Content-Type');
    if (code !== 200) {
      throw new Error(`cros-static-illustration could not fetch '${url}',
        got response (${code})`);
    } else if (contentType === SVG_CONTENT_TYPE) {
      this.cachedExternalSVG = await response.text();
      this.recolorExternalAsset();
    } else {
      // Some other external asset. Simply render it as is.
      this.imgSrc = url;
    }

    this.dispatchEvent(new CustomEvent('illustration-updated'));
  }

  override connectedCallback() {
    super.connectedCallback();

    addColorSchemeChangeListener(this.onColorChange);
  }

  override disconnectedCallback() {
    super.disconnectedCallback();

    removeColorSchemeChangeListener(this.onColorChange);
  }

  override willUpdate(changedProperties: PropertyValues<this>) {
    if (changedProperties.has('url')) {
      if (this.url) {
        this.shouldRenderTemplate = false;
        this.fetchUrl(this.url);
        return;
      }

      // If the url has been removed, and there is a template for a local
      // asset bound, on next render we want to render that rather than the
      // cached colored SVG.
      this.shouldRenderTemplate = true;
    }
  }

  override render() {
    if (!this.shouldRenderTemplate && !!this.imgSrc) {
      // If we don't have alt text set the alt attribute to empty string.
      // https://www.w3.org/WAI/WCAG21/Techniques/html/H67.html
      return html`
        <img
          alt=${this.alt !== null ? this.alt : ''}
          src="${this.imgSrc}"/>
      `;
    }
    // This will either render a local SVG template or the empty template.
    return html`${templateContent(this.template)}`;
  }
}

customElements.define('cros-static-illustration', StaticIllustration);

declare global {
  interface HTMLElementTagNameMap {
    'cros-static-illustration': StaticIllustration;
  }
}
