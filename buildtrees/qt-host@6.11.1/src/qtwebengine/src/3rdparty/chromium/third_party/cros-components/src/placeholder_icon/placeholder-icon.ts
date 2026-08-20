/**
 * @license
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

import {css, CSSResultGroup, html, LitElement} from 'lit';

/**
 * A simple jellybean icon for use in tests, demos and as a placeholder.
 * Not to be used in production, if you are looking for a icon component see
 * go/jellybean-library-guide#icons.
 **/
export class PlaceholderIcon extends LitElement {
  /** @nocollapse */
  static override styles: CSSResultGroup = css`
      :host {
        --icon-color: currentcolor;
        display: inline-block;
        height: 20px;
        width: 20px;
      }

      :host([dir="rtl"][rtlflip]) {
        transform: scaleX(-1);
      }

      svg {
        fill: var(--icon-color, currentcolor);
        stroke: var(--icon-color, currentcolor);
        width: 100%;
        height: 100%;
      }
    `;

  override render() {
    return html`
      <svg
          id="lord-bean"
          viewBox="0 0 250 246"
          opacity="1"
          version="1.1">
        <mask
          id="mask0_713_1056"
          maskUnits="userSpaceOnUse"
          x="38"
          y="36"
          fill="white"
          stroke="white"
          width="190"
          height="179">
          <path
            d="M 44.106,117.329 C 20.491,39.2312 87.0956,22.8142 109.578,49.4954 c 17.008,20.1851 20.025,44.4845 40.765,57.7686 19.297,12.361 38.634,11.671 55.226,22.298 30.7,19.664 27.816,53.26 -6.666,74.875 -34.482,21.614 -126.4323,6.697 -154.797,-87.108 z"
            stroke-width="2"/>
        </mask>
        <g
          mask="url(#mask0_713_1056)">
          <path
            d="M 97.917969,-9.3417971 C 61.528644,23.722657 25.139321,56.787111 -11.250003,89.851565 9.1848967,112.34179 29.619793,134.83204 50.05469,157.32226 c -2.551979,3.08089 -8.238506,6.01171 -8.981147,9.17922 10.613505,11.68049 21.227009,23.361 31.840514,35.04149 3.268072,-1.34974 6.874208,-9.75854 9.89443,-5.93305 8.291703,9.12546 16.583405,18.25094 24.875103,27.37641 3.43747,-1.19221 6.46566,6.18339 9.73675,8.47407 8.65842,9.52935 17.31956,19.05773 25.97629,28.5877 36.38932,-33.06446 72.77865,-66.12891 109.16797,-99.19336 C 201.01572,104.12256 149.46685,47.390383 97.917969,-9.3417971 Z" />
        </g>
      </svg>
    `;
  }
}

customElements.define('cros-placeholder-icon', PlaceholderIcon);

declare global {
  interface HTMLElementTagNameMap {
    'cros-placeholder-icon': PlaceholderIcon;
  }
}
