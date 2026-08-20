// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MESHSHAPE_H
#define MESHSHAPE_H

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

#include <QtQuick3DPhysics/qtquick3dphysicsglobal.h>
#include <QtQuick3DPhysics/private/qabstractcollisionshape_p.h>
#include <QtCore/QObject>
#include <QtGui/QVector3D>
#include <QtQml/QQmlEngine>
#include <QtQuick3D/QQuick3DGeometry>

namespace physx {
class PxBoxGeometry;
class PxConvexMesh;
class PxConvexMeshGeometry;
class PxTriangleMesh;
class PxTriangleMeshGeometry;
}

QT_BEGIN_NAMESPACE
class QQuick3DPhysicsMesh;

class Q_QUICK3DPHYSICS_EXPORT QMeshShape : public QAbstractCollisionShape
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged REVISION(6, 5))
    Q_PROPERTY(QQuick3DGeometry *geometry READ geometry WRITE setGeometry NOTIFY geometryChanged
                       REVISION(6, 7))
    QML_NAMED_ELEMENT(MeshShape)
    QML_UNCREATABLE("abstract interface")

public:
    ~QMeshShape();

    enum class MeshType { TRIANGLE, CONVEX };
    virtual MeshType shapeType() const = 0;

    physx::PxGeometry *getPhysXGeometry() override;

    Q_REVISION(6, 5) const QUrl &source() const;
    Q_REVISION(6, 5) void setSource(const QUrl &newSource);
    Q_REVISION(6, 7) QQuick3DGeometry *geometry() const;
    Q_REVISION(6, 7) void setGeometry(QQuick3DGeometry *newGeometry);

signals:
    Q_REVISION(6, 5) void sourceChanged();
    Q_REVISION(6, 7) void geometryChanged();

private slots:
    void geometryDestroyed(QObject *geometry);
    void geometryContentChanged();

private:
    void updatePhysXGeometry();

    bool m_dirtyPhysx = false;
    physx::PxConvexMeshGeometry *m_convexGeometry = nullptr;
    physx::PxTriangleMeshGeometry *m_triangleGeometry = nullptr;
    QUrl m_meshSource;
    QQuick3DPhysicsMesh *m_mesh = nullptr;
    QQuick3DGeometry *m_geometry = nullptr;
};

QT_END_NAMESPACE

#endif // MESHSHAPE_H
