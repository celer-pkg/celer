// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtwebengine/userscript/user_script_data_mojom_traits.h"

namespace mojo {

bool StructTraits<qtwebengine::mojom::UserScriptDataDataView,
                  QtWebEngineCore::UserScriptData>::
    Read(qtwebengine::mojom::UserScriptDataDataView data,
         QtWebEngineCore::UserScriptData* out_data) {
  if (!data.ReadSource(&out_data->source))
    return false;
  if (!data.ReadUrl(&out_data->url))
    return false;
  out_data->injectionPoint = data.injectionPoint();
  out_data->injectForSubframes = data.injectForSubframes();
  out_data->worldId = data.worldId();
  out_data->scriptId = data.scriptId();
  if (!data.ReadGlobs(&out_data->globs))
    return false;
  if (!data.ReadExcludeGlobs(&out_data->excludeGlobs))
    return false;
  if (!data.ReadUrlPatterns(&out_data->urlPatterns))
    return false;
  return true;
}

}  // namespace mojo
