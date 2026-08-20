// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEABSTRACTSHAPE_H
#define QQUICK3DPARTICLEABSTRACTSHAPE_H

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
#include <QtQml/qqmlparserstatus.h>
#include <QtQml/qqml.h>
#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DParticleSystem;
class QQuick3DNode;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleAbstractShape : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_INTERFACES(QQmlParserStatus)
    QML_ADDED_IN_VERSION(6, 2)

public:
    explicit QQuick3DParticleAbstractShape(QObject *parent = nullptr);
    // Returns position inside the shape
    virtual QVector3D getPosition(int particleIndex) = 0;
    Q_REVISION(6, 10) virtual QVector3D getSurfaceNormal(int particleIndex) = 0;

protected:
    // These need access to m_system
    friend class QQuick3DParticleEmitter;
    friend class QQuick3DParticleAttractor;

    // From QQmlParserStatus
    void componentComplete() override;
    void classBegin() override {}
    QQuick3DNode *parentNode();

    QQuick3DNode *m_parentNode = nullptr;
    QQuick3DParticleSystem *m_system = nullptr;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEABSTRACTSHAPE_H
