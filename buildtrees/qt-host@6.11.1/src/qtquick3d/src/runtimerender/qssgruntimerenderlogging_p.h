// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRUNTIMERENDERLOGGING_H
#define QSSGRUNTIMERENDERLOGGING_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(GL_ERROR)
Q_DECLARE_LOGGING_CATEGORY(INVALID_PARAMETER)
Q_DECLARE_LOGGING_CATEGORY(INVALID_OPERATION)
Q_DECLARE_LOGGING_CATEGORY(OUT_OF_MEMORY)
Q_DECLARE_LOGGING_CATEGORY(INTERNAL_ERROR)
Q_DECLARE_LOGGING_CATEGORY(PERF_WARNING)
Q_DECLARE_LOGGING_CATEGORY(PERF_INFO)
Q_DECLARE_LOGGING_CATEGORY(WARNING)

QT_END_NAMESPACE

#endif // QSSGRUNTIMERENDERLOGGING_H
