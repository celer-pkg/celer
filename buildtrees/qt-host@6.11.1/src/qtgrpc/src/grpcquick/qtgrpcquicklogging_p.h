// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTGRPCQUICKLOGGING_P_H
#define QTGRPCQUICKLOGGING_P_H

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

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(GrpcQuick)
QT_END_NAMESPACE

#define qGrpcQuickDebug(...) qCDebug(GrpcQuick, __VA_ARGS__)
#define qGrpcQuickInfo(...) qCInfo(GrpcQuick, __VA_ARGS__)
#define qGrpcQuickWarning(...) qCWarning(GrpcQuick, __VA_ARGS__)
#define qGrpcQuickCritical(...) qCCritical(GrpcQuick, __VA_ARGS__)

#endif // QTGRPCQUICKLOGGING_P_H
