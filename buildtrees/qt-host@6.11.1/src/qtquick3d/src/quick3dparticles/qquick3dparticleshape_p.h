// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLESHAPE_H
#define QQUICK3DPARTICLESHAPE_H

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

#include "qquick3dparticleabstractshape_p.h"
#include <QVector3D>

QT_BEGIN_NAMESPACE

class QQuick3DParticleSystem;
class QQuick3DModel;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleShape : public QQuick3DParticleAbstractShape
{
    Q_OBJECT
    Q_PROPERTY(bool fill READ fill WRITE setFill NOTIFY fillChanged)
    Q_PROPERTY(ShapeType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QVector3D extents READ extents WRITE setExtents NOTIFY extentsChanged)

    QML_NAMED_ELEMENT(ParticleShape3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    enum ShapeType
    {
        Cube = 0,
        Sphere,
        Cylinder
    };
    Q_ENUM(ShapeType)

    QQuick3DParticleShape(QObject *parent = nullptr);

    bool fill() const;
    ShapeType type() const;
    QVector3D extents() const;

    // Returns point inside this shape
    QVector3D getPosition(int particleIndex) override;
    Q_REVISION(6, 10) QVector3D getSurfaceNormal(int particleIndex) override;

public Q_SLOTS:
    void setFill(bool fill);
    void setType(QQuick3DParticleShape::ShapeType type);
    void setExtents(QVector3D extends);

Q_SIGNALS:
    void fillChanged();
    void typeChanged();
    void extentsChanged();

private:
    QVector3D randomPositionCube(int particleIndex) const;
    QVector3D randomPositionSphere(int particleIndex) const;
    QVector3D randomPositionCylinder(int particleIndex) const;

    bool m_fill = true;
    ShapeType m_type = ShapeType::Cube;
    QVector3D m_extents = QVector3D(50, 50, 50);
    QVector3D m_cachedPosition;
    int m_cachedIndex = -1;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLESHAPE_H
