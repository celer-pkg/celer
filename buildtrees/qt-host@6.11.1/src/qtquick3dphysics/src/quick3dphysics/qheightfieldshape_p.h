// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef HEIGHTFIELDMESHSHAPE_H
#define HEIGHTFIELDMESHSHAPE_H

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
#include <QtQuick/private/qquickimage_p.h>

namespace physx {
class PxBoxGeometry;
class PxTriangleMesh;
class PxTriangleMeshGeometry;
class PxHeightFieldGeometry;
class PxHeightField;
struct PxHeightFieldSample;
}

QT_BEGIN_NAMESPACE

class QQuick3DPhysicsHeightField;

class Q_QUICK3DPHYSICS_EXPORT QHeightFieldShape : public QAbstractCollisionShape
{
    Q_OBJECT
    Q_PROPERTY(QVector3D extents READ extents WRITE setExtents NOTIFY extentsChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged REVISION(6, 5))
    Q_PROPERTY(QQuickImage *image READ image WRITE setImage NOTIFY imageChanged REVISION(6, 7))
    QML_NAMED_ELEMENT(HeightFieldShape)
public:
    QHeightFieldShape();
    ~QHeightFieldShape();

    physx::PxGeometry *getPhysXGeometry() override;

    Q_REVISION(6, 5) const QUrl &source() const;
    Q_REVISION(6, 5) void setSource(const QUrl &newSource);

    const QVector3D &hfOffset() const { return m_hfOffset; }

    const QVector3D &extents() const;
    void setExtents(const QVector3D &newExtents);
    bool isStaticShape() const override { return true; }

    Q_REVISION(6, 7) QQuickImage *image() const;
    Q_REVISION(6, 7) void setImage(QQuickImage *newImage);

signals:
    Q_REVISION(6, 5) void sourceChanged();
    void extentsChanged();
    Q_REVISION(6, 7) void imageChanged();

private slots:
    void imageDestroyed(QObject *image);
    void imageGeometryChanged();

private:
    void updatePhysXGeometry();
    void getSamples();
    void updateExtents();

    QQuick3DPhysicsHeightField *m_heightField = nullptr;

    physx::PxHeightFieldGeometry *m_heightFieldGeometry = nullptr;
    QVector3D m_hfOffset;
    QUrl m_heightMapSource;
    bool m_dirtyPhysx = false;
    QVector3D m_extents = { 100, 100, 100 };
    bool m_extentsSetExplicitly = false;
    QQuickImage *m_image = nullptr;
};

QT_END_NAMESPACE

#endif // HEIGHTFIELDMESHSHAPE_H
