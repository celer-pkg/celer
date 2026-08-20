// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:header-decls-only

#ifndef QCORE5GLOBAL_H
#define QCORE5GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore5Compat/qtcore5compat-config.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#    if defined(QT_BUILD_CORE5COMPAT_LIB)
#        define Q_CORE5COMPAT_EXPORT Q_DECL_EXPORT
#    else
#        define Q_CORE5COMPAT_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define Q_CORE5COMPAT_EXPORT
#endif

QT_END_NAMESPACE

#endif // QCORE5GLOBAL_H
