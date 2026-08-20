// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef USER_SCRIPT_DATA_H
#define USER_SCRIPT_DATA_H

#include <string>
#include <vector>
#include "url/gurl.h"

namespace QtWebEngineCore {

struct UserScriptData {
  enum InjectionPoint {
    AfterLoad,
    DocumentLoadFinished,
    DocumentElementCreation
  };

  UserScriptData() = default;

  std::string source;
  GURL url;
  uint8_t injectionPoint = 0;
  bool injectForSubframes = false;
  uint8_t worldId = 1;
  uint64_t scriptId = 0;
  std::vector<std::string> globs;
  std::vector<std::string> excludeGlobs;
  std::vector<std::string> urlPatterns;
};
}  // namespace QtWebEngineCore
#endif  // USER_SCRIPT_DATA_H
