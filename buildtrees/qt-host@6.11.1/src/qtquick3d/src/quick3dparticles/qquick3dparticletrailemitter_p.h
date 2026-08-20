// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLETRAILEMITTER_H
#define QQUICK3DPARTICLETRAILEMITTER_H

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

#include <QtQuick3DParticles/private/qquick3dparticleemitter_p.h>
#include <QtQuick3DParticles/private/qquick3dparticle_p.h>
#include <QtQuick3DParticles/private/qquick3dparticledata_p.h>
#include <QQmlEngine>
#include <QList>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleTrailEmitter : public QQuick3DParticleEmitter
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DParticle *follow READ follow WRITE setFollow NOTIFY followChanged)
    QML_NAMED_ELEMENT(TrailEmitter3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleTrailEmitter(QQuick3DNode *parent = nullptr);

    QQuick3DParticle *follow() const;

    Q_INVOKABLE void burst(int count) override;

public Q_SLOTS:
    void setFollow(QQuick3DParticle *follow);

Q_SIGNALS:
    void followChanged();

protected:
    friend class QQuick3DParticleSystem;
    void emitTrailParticles(const QVector3D &centerPos, int emitAmount, int triggerType, const QVector3D &normal, const QVector3D &velocity);
    bool hasBursts() const;
    void clearBursts();

private:
    QQuick3DParticle *m_follow = nullptr;
    QList<QQuick3DParticleEmitBurstData> m_bursts;

};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLETRAILEMITTER_H
