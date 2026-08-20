// Copyright (C) 2025 Intel Corporation
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:header-decls-only

#ifndef QSTRINGCONVERTER_BASE_DEPRECATED_H
#define QSTRINGCONVERTER_BASE_DEPRECATED_H

#if 0
#pragma qt_class(QStringConverterBase)
#pragma qt_no_master_include
#pragma qt_sync_stop_processing
#endif

#include <QtCore/qcompilerdetection.h>
#include <QtCore/qstringconverter.h>

#ifdef Q_CC_GNU
#  warning "QStringConverterBase is not a public class and including " \
        "<QtCore/QStringConverterBase> is deprecated. The header will disappear" \
        " in Qt 7"
#elif defined(Q_CC_MSVC)
#  pragma message("QStringConverterBase is not a public class and including " \
    "<QtCore/QStringConverterBase> is deprecated. The header will disappear" \
    " in Qt 7")
#endif

#endif // QSTRINGCONVERTER_BASE_DEPRECATED_H
