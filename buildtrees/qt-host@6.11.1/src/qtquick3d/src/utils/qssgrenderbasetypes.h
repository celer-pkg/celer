// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERBASETYPES_H
#define QSSGRENDERBASETYPES_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtQuick3D API, with limited compatibility guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtCore/qtypes.h>

#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

enum class QSSGNodeId : quint64 { Invalid = 0 };
enum class QSSGResourceId : quint64 { Invalid = 0 };
enum class QSSGCameraId : quint64 { Invalid = 0 };
enum class QSSGExtensionId : quint64 { Invalid = 0 };

using QSSGNodeIdList = QList<QSSGNodeId>;

QT_END_NAMESPACE

#endif // QSSGRENDERBASETYPES_H
