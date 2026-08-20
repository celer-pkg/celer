// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef USER_SCRIPT_DATA_MOJOM_TRAITS_H_
#define USER_SCRIPT_DATA_MOJOM_TRAITS_H_

#include <string>
#include "mojo/public/cpp/bindings/struct_traits.h"
#include "qtwebengine/userscript/user_script_data.h"
#include "qtwebengine/userscript/userscript.mojom.h"
#include "url/gurl.h"

namespace mojo {

template <>
class StructTraits<qtwebengine::mojom::UserScriptDataDataView,
                   QtWebEngineCore::UserScriptData> {
 public:
  static std::string source(const QtWebEngineCore::UserScriptData& d) {
    return d.source;
  }
  static GURL url(const QtWebEngineCore::UserScriptData& d) { return d.url; }
  static uint8_t injectionPoint(const QtWebEngineCore::UserScriptData& d) {
    return d.injectionPoint;
  }
  static bool injectForSubframes(const QtWebEngineCore::UserScriptData& d) {
    return d.injectForSubframes;
  }
  static uint64_t worldId(const QtWebEngineCore::UserScriptData& d) {
    return d.worldId;
  }
  static uint8_t scriptId(const QtWebEngineCore::UserScriptData& d) {
    return d.scriptId;
  }
  static std::vector<std::string> globs(
      const QtWebEngineCore::UserScriptData& d) {
    return d.globs;
  }
  static std::vector<std::string> excludeGlobs(
      const QtWebEngineCore::UserScriptData& d) {
    return d.excludeGlobs;
  }
  static std::vector<std::string> urlPatterns(
      const QtWebEngineCore::UserScriptData& d) {
    return d.urlPatterns;
  }
  static bool Read(qtwebengine::mojom::UserScriptDataDataView data,
                   QtWebEngineCore::UserScriptData* out_data);
};

}  // namespace mojo
#endif
