// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QTPROTOCDEFS_H
#define QTPROTOCDEFS_H

#include <google/protobuf/stubs/common.h>
#if GOOGLE_PROTOBUF_VERSION >= 3012000
// Idicates the major libprotoc API update that happened in version 3.12
#  define HAVE_PROTOBUF_SYNC_PIPER
#endif

#if GOOGLE_PROTOBUF_VERSION >= 4023000
// Idicates that libprotoc has Descriptor::real_oneof_decl method
#  define HAVE_REAL_ONEOF_DECL
#endif

#endif // QTPROTOCDEFS_H
