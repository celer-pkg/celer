/* Copyright (C) 2025 The Qt Company Ltd.
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
*/

import {PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {ExtensionsUIBrowserProxy} from './extensions_ui_browser_proxy.js'

import {getTemplate} from './toolbar.html.js';

export class ExtensionsToolbarElement extends PolymerElement {
  static get is() {
    return 'extension-toolbar';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      showToolbar_: {
        type: Boolean,
        value: () => true,
      },
    };
  }

  declare private showToolbar_: boolean;
  private proxy_: ExtensionsUIBrowserProxy =
      ExtensionsUIBrowserProxy.getInstance();

  private onLoadUnpackedClick_() {
    this.proxy_.callbackRouter.reloadPage.addListener(
    () => {
      window.location.reload();
    });
    this.proxy_.handler.loadExtension();
  }

  private onInstallExtensionClick_() {
    this.proxy_.callbackRouter.reloadPage.addListener(() => {
      window.location.reload();
    });
    this.proxy_.handler.installExtension();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'extension-toolbar': ExtensionsToolbarElement;
  }
}

customElements.define(ExtensionsToolbarElement.is, ExtensionsToolbarElement);
