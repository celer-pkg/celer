// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/extensions/api/resources_private/resources_private_api.h
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QTWEBENGINE_BROWSER_EXTENSIONS_API_RESOURCES_PRIVATE_RESOURCES_PRIVATE_API_H_
#define QTWEBENGINE_BROWSER_EXTENSIONS_API_RESOURCES_PRIVATE_RESOURCES_PRIVATE_API_H_

#include "extensions/browser/extension_function.h"

namespace extensions {

class ResourcesPrivateGetStringsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("resourcesPrivate.getStrings",
                             RESOURCESPRIVATE_GETSTRINGS)
  ResourcesPrivateGetStringsFunction();

 protected:
  ~ResourcesPrivateGetStringsFunction() override;

  // Override from ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

}  // namespace extensions

#endif  // QTWEBENGINE_BROWSER_EXTENSIONS_API_RESOURCES_PRIVATE_RESOURCES_PRIVATE_API_H_
