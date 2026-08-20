// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticleshape_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticlesystem_p.h"
#include "qquick3dparticleutils_p.h"
#include <QtCore/qdir.h>
#include <QtQml/qqmlfile.h>
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <algorithm>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleShape3D
    \inherits ParticleAbstractShape3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Offers 3D shapes for emitters and affectors.
    \since 6.2

    The ParticleShape3D element supports shapes like \c Cube, \c Sphere and \c Cylinder for particles needs.
    For example, emitter can use \l {ParticleEmitter3D::shape}{shape} property to emit particles from the
    shape area.

    Shapes don't have position, scale or rotation. Instead, they use parent node for these properties.
*/

QQuick3DParticleShape::QQuick3DParticleShape(QObject *parent)
    : QQuick3DParticleAbstractShape(parent)
{
}

/*!
    \qmlproperty bool ParticleShape3D::fill

    This property defines if the shape should be filled or just use the shape outlines.

    The default value is \c true.
*/
bool QQuick3DParticleShape::fill() const
{
    return m_fill;
}

void QQuick3DParticleShape::setFill(bool fill)
{
    if (m_fill == fill)
        return;

    m_fill = fill;
    Q_EMIT fillChanged();
}

/*!
    \qmlproperty ShapeType ParticleShape3D::type

    This property defines the type of the shape.

    The default value is \c ParticleShape3D.Cube.
*/

/*!
    \qmlproperty enumeration ParticleShape3D::ShapeType

    Defines the type of the shape.

    \value ParticleShape3D.Cube
        Cube shape.
    \value ParticleShape3D.Sphere
        Sphere shape.
    \value ParticleShape3D.Cylinder
        Cylinder shape.
*/

QQuick3DParticleShape::ShapeType QQuick3DParticleShape::type() const
{
    return m_type;
}

/*!
    \qmlproperty vector3d ParticleShape3D::extents

    This property defines the extents of the shape.

    The default value for each axis is \c 50.
*/
QVector3D QQuick3DParticleShape::extents() const
{
    return m_extents;
}

void QQuick3DParticleShape::setType(QQuick3DParticleShape::ShapeType type)
{
    if (m_type == type)
        return;

    m_type = type;
    Q_EMIT typeChanged();
}

void QQuick3DParticleShape::setExtents(QVector3D extents)
{
    if (m_extents == extents)
        return;

    m_extents = extents;
    Q_EMIT extentsChanged();
}

QVector3D QQuick3DParticleShape::getPosition(int particleIndex)
{
    if (!parentNode() || !m_system)
        return QVector3D();
    if (m_cachedIndex == particleIndex)
        return m_cachedPosition;

    switch (m_type) {
    case QQuick3DParticleShape::ShapeType::Cube:
        m_cachedPosition = randomPositionCube(particleIndex);
        break;
    case QQuick3DParticleShape::ShapeType::Sphere:
        m_cachedPosition = randomPositionSphere(particleIndex);
        break;
    case QQuick3DParticleShape::ShapeType::Cylinder:
        m_cachedPosition = randomPositionCylinder(particleIndex);
        break;
    }
    m_cachedIndex = particleIndex;
    return m_cachedPosition;
}

QVector3D QQuick3DParticleShape::getSurfaceNormal(int particleIndex)
{
    if (m_fill)
        return QVector3D();

    auto rand = m_system->rand();
    QVector3D n;
    switch (m_type) {
    case QQuick3DParticleShape::ShapeType::Cube: {
        int side = int(rand->get(particleIndex, QPRand::Shape4) * 6);
        if (side == 0)
            n.setX(-1.0f);
        else if (side == 1)
            n.setX(1.0f);
        else if (side == 2)
            n.setY(-1.0f);
        else if (side == 3)
            n.setY(1.0f);
        else if (side == 4)
            n.setZ(-1.0f);
        else
            n.setZ(1.0f);
        QMatrix4x4 mat;
        mat.rotate(m_parentNode->rotation());
        n = mat.mapVector(n);
    } break;
    case QQuick3DParticleShape::ShapeType::Sphere:
        if (particleIndex != m_cachedIndex)
            getPosition(particleIndex);
        n = m_cachedPosition.normalized();
        break;
    case QQuick3DParticleShape::ShapeType::Cylinder:
        QVector3D scale = m_parentNode->scale();
        float theta = rand->get(particleIndex, QPRand::Shape3) * float(M_PI) * 2.0f;
        float x = QPCOS(theta);
        float z = QPSIN(theta);
        x = x * scale.x();
        z = z * scale.z();
        QVector3D normal(x, 0.0, z);
        QMatrix4x4 mat;
        mat.rotate(m_parentNode->rotation());
        n = mat.mapVector(normal.normalized());
        break;
    }
    return n;
}

QVector3D QQuick3DParticleShape::randomPositionCube(int particleIndex) const
{
    auto rand = m_system->rand();
    QVector3D s = m_parentNode->scale() * m_extents;
    float x = s.x() - (rand->get(particleIndex, QPRand::Shape1) * s.x() * 2.0f);
    float y = s.y() - (rand->get(particleIndex, QPRand::Shape2) * s.y() * 2.0f);
    float z = s.z() - (rand->get(particleIndex, QPRand::Shape3) * s.z() * 2.0f);
    if (!m_fill) {
        // Random 0..5 for cube sides
        int side = int(rand->get(particleIndex, QPRand::Shape4) * 6);
        if (side == 0)
            x = -s.x();
        else if (side == 1)
            x = s.x();
        else if (side == 2)
            y = -s.y();
        else if (side == 3)
            y = s.y();
        else if (side == 4)
            z = -s.z();
        else
            z = s.z();
    }
    QMatrix4x4 mat;
    mat.rotate(m_parentNode->rotation());
    return mat.mapVector(QVector3D(x, y, z));
}

QVector3D QQuick3DParticleShape::randomPositionSphere(int particleIndex) const
{
    auto rand = m_system->rand();
    QVector3D scale = m_parentNode->scale() * m_extents;
    float theta = rand->get(particleIndex, QPRand::Shape1) * float(M_PI) * 2.0f;
    float v = rand->get(particleIndex, QPRand::Shape2);
    float phi = acos((2.0f * v) - 1.0f);
    float r = m_fill ? pow(rand->get(particleIndex, QPRand::Shape3), 1.0f / 3.0f) : 1.0f;
    float x = r * QPSIN(phi) * QPCOS(theta);
    float y = r * QPSIN(phi) * QPSIN(theta);
    float z = r * QPCOS(phi);
    QVector3D pos(x, y, z);
    pos *= scale;
    QMatrix4x4 mat;
    mat.rotate(m_parentNode->rotation());
    return mat.mapVector(pos);
}

QVector3D QQuick3DParticleShape::randomPositionCylinder(int particleIndex) const
{
    auto rand = m_system->rand();
    QVector3D scale = m_parentNode->scale() * m_extents;
    float y = scale.y() - (rand->get(particleIndex, QPRand::Shape1) * scale.y() * 2.0f);
    float r = 1.0f;
    if (m_fill)
        r = sqrt(rand->get(particleIndex, QPRand::Shape2));
    float theta = rand->get(particleIndex, QPRand::Shape3) * float(M_PI) * 2.0f;
    float x = r * QPCOS(theta);
    float z = r * QPSIN(theta);
    x = x * scale.x();
    z = z * scale.z();
    QVector3D pos(x, y, z);
    QMatrix4x4 mat;
    mat.rotate(m_parentNode->rotation());
    return mat.mapVector(pos);
}

QT_END_NAMESPACE
