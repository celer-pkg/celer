// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEDIRECTION_H
#define QQUICK3DPARTICLEDIRECTION_H

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

#include <QObject>
#include <QVector3D>
#include <QtQml/qqml.h>

#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>
#include <QtQuick3DParticles/private/qquick3dparticledata_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DParticleSystem;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleDirection : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleDirection(QObject *parent = nullptr);
    virtual QVector3D sample(const QQuick3DParticleData &d) = 0;

protected:
    friend class QQuick3DParticleEmitter;

    QQuick3DParticleSystem *m_system = nullptr;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEDIRECTION_H
