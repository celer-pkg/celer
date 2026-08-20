/* Copyright (C) 2025 The Qt Company Ltd.
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
*/

import {ExtensionsUIHandlerFactory, PageCallbackRouter, PageHandlerRemote} from './extensions_ui_qt.mojom-webui.js';

/** Holds Mojo interfaces for communication with the browser process. */
export class ExtensionsUIBrowserProxy {
  callbackRouter: PageCallbackRouter = new PageCallbackRouter();
  handler: PageHandlerRemote = new PageHandlerRemote();

  constructor() {
    const factory = ExtensionsUIHandlerFactory.getRemote();
    factory.createPageHandler(
        this.callbackRouter.$.bindNewPipeAndPassRemote(),
        this.handler.$.bindNewPipeAndPassReceiver());
  }

  static getInstance(): ExtensionsUIBrowserProxy {
    return instance || (instance = new ExtensionsUIBrowserProxy());
  }

  static setInstance(obj: ExtensionsUIBrowserProxy): void {
    instance = obj;
  }
}

let instance: ExtensionsUIBrowserProxy|null = null;
