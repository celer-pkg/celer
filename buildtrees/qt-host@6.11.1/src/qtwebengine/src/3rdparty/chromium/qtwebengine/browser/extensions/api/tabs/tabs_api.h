// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/extensions/api/tabs/tabs_api.h
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QTWEBENGINE_BROWSER_EXTENSIONS_API_TABS_API_H_
#define QTWEBENGINE_BROWSER_EXTENSIONS_API_TABS_API_H_

#include "extensions/browser/extension_function.h"

namespace extensions {

class TabsUpdateFunction : public ExtensionFunction {
 public:
  TabsUpdateFunction();

 protected:
  ~TabsUpdateFunction() override {}
  bool UpdateURL(const std::string& url,
                 int tab_id,
                 std::string* error);
  ResponseValue GetResult();

  content::WebContents* web_contents_;

 private:
  ResponseAction Run() override;
  void OnExecuteCodeFinished(const std::string& error,
                             const GURL& on_url,
                             const base::Value::List& script_result);

  DECLARE_EXTENSION_FUNCTION("tabs.update", TABS_UPDATE)
};

}  // namespace extensions

#endif  // QTWEBENGINE_BROWSER_EXTENSIONS_API_TABS_API_H_
