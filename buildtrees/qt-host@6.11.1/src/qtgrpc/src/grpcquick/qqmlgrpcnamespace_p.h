// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQMLGRPCNAMESPACE_P_H
#define QQMLGRPCNAMESPACE_P_H

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

#include <QtGrpc/qtgrpcnamespace.h>

#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

namespace QtGrpcForeign {
Q_NAMESPACE
QML_FOREIGN_NAMESPACE(QtGrpc)
QML_NAMED_ELEMENT(QtGrpc)
QML_ADDED_IN_VERSION(6, 8)
}

QT_END_NAMESPACE

#endif // QQMLGRPCNAMESPACE_P_H
