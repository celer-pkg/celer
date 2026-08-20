// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/extensions/api/resources_private/resources_private_api.cc
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "qtwebengine/browser/extensions/api/resources_private/resources_private_api.h"

#include <memory>
#include <string>
#include <utility>

#include "pdf/buildflags.h"
#if BUILDFLAG(ENABLE_PDF)
#include "qtwebengine/browser/pdf/pdf_extension_util.h"
#endif
#include "qtwebengine/common/extensions/api/resources_private.h"

namespace extensions {

namespace get_strings = api::resources_private::GetStrings;

ResourcesPrivateGetStringsFunction::ResourcesPrivateGetStringsFunction() {
}

ResourcesPrivateGetStringsFunction::~ResourcesPrivateGetStringsFunction() {}

ExtensionFunction::ResponseAction ResourcesPrivateGetStringsFunction::Run() {
  auto params = get_strings::Params::Create(args());
  base::Value::Dict dict;

  api::resources_private::Component component = params->component;

  switch (component) {
    case api::resources_private::Component::kIdentity:
      break;
    case api::resources_private::Component::kPdf:
#if BUILDFLAG(ENABLE_PDF)
      pdf_extension_util::AddStrings(pdf_extension_util::PdfViewerContext::kAll, &dict);
      pdf_extension_util::AddAdditionalData(true, &dict);
#else
      NOTREACHED();
#endif  // BUILDFLAG(ENABLE_PDF)
      break;
    case api::resources_private::Component::kNone:
      NOTREACHED();
  }

  return RespondNow(WithArguments(std::move(dict)));
}

}  // namespace extensions
