// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DEXTENSIONHELPERS_H
#define QQUICK3DEXTENSIONHELPERS_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtQuick3D API, with limited compatibility guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtQuick3D/qtquick3dglobal.h>
#include <ssg/qssgrenderbasetypes.h>
#include <ssg/qssgrendergraphobject.h>

QT_BEGIN_NAMESPACE

class QQuick3DObject;

#ifdef Q_QDOC
typedef quint64 QSSGNodeId;
typedef quint64 QSSGResourceId;
typedef quint64 QSSGCameraId;
typedef quint64 QSSGExtensionId;
#endif

class Q_QUICK3D_EXPORT QQuick3DExtensionHelpers
{
public:
    QQuick3DExtensionHelpers();

    [[nodiscard]] static QSSGNodeId getNodeId(const QQuick3DObject &node);
    [[nodiscard]] static QSSGResourceId getResourceId(const QQuick3DObject &resource);
    [[nodiscard]] static QSSGCameraId getCameraId(const QQuick3DObject &camera);
    [[nodiscard]] static QSSGExtensionId getExtensionId(const QQuick3DObject &extension);

    [[nodiscard]] static QSSGRenderGraphObject::Type getNodeIdType(QSSGNodeId nodeId);

    template<typename QSSGTypeId>
    [[nodiscard]] static constexpr bool isNull(QSSGTypeId id) { return (id == QSSGTypeId::Invalid); }
};

QT_END_NAMESPACE

#endif // QQUICK3DEXTENSIONHELPERS_H
