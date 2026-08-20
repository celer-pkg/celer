// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticledirection_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Direction3D
    \inherits QtObject
    \inqmlmodule QtQuick3D.Particles3D
    \brief Directions assign velocity for the emitted particles.
    \since 6.2

    The Direction3D is an abstract base class of directions like \l TargetDirection3D and \l VectorDirection3D.
*/

QQuick3DParticleDirection::QQuick3DParticleDirection(QObject *parent)
    : QObject(parent)
{
}

QT_END_NAMESPACE
