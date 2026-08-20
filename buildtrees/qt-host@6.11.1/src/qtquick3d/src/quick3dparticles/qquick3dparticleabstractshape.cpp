// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticleabstractshape_p.h"
#include <QtQuick3D/private/qquick3dnode_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleAbstractShape3D
    \inherits QtObject
    \inqmlmodule QtQuick3D.Particles3D
    \brief Abstract base type of particle shapes.
    \since 6.2

    The ParticleAbstractShape3D is an abstract base type of shapes like \l ParticleShape3D
    and \l ParticleModelShape3D. Shapes can be used to provide start and end positions
    for the particles.
*/
QQuick3DParticleAbstractShape::QQuick3DParticleAbstractShape(QObject *parent)
    : QObject(parent)
{
}

void QQuick3DParticleAbstractShape::componentComplete()
{
    if (!parentNode())
        qWarning() << "Shape requires parent Node to function correctly!";
}

QQuick3DNode *QQuick3DParticleAbstractShape::parentNode()
{
    QQuick3DNode *node = qobject_cast<QQuick3DNode *>(parent());
    if (!m_parentNode || m_parentNode != node)
        m_parentNode = node;
    return m_parentNode;
}

QT_END_NAMESPACE
