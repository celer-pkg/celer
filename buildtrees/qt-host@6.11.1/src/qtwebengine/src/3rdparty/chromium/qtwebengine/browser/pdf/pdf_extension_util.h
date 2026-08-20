// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/pdf/pdf_extension_util.h
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QTWEBENGINE_BROWSER_PDF_PDF_EXTENSION_UTIL_H_
#define QTWEBENGINE_BROWSER_PDF_PDF_EXTENSION_UTIL_H_

#include <string>

#include "base/values.h"
#include "pdf/buildflags.h"

#if !BUILDFLAG(ENABLE_PDF)
#error "PDF must be enabled"
#endif

namespace pdf_extension_util {

// Represents the context within which the PDF Viewer runs.
enum class PdfViewerContext {
  kPdfViewer,
  kPrintPreview,
  kAll,
};

// Adds all strings used by the PDF Viewer depending on the provided |context|.
void AddStrings(PdfViewerContext context, base::Value::Dict* dict);

// Adds additional data used by the PDF Viewer UI in `dict`, for example
// whether certain features are enabled/disabled.
// `enable_annotations` only applies on platforms that supports annotations.
void AddAdditionalData(bool enable_annotations, base::Value::Dict* dict);

}  // namespace pdf_extension_util

#endif  // QTWEBENGINE_BROWSER_PDF_PDF_EXTENSION_UTIL_H_
