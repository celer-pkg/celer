// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEGRAVITY_H
#define QQUICK3DPARTICLEGRAVITY_H

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
#include <QtQuick3DParticles/private/qquick3dparticleaffector_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleGravity : public QQuick3DParticleAffector
{
    Q_OBJECT
    Q_PROPERTY(float magnitude READ magnitude WRITE setMagnitude NOTIFY magnitudeChanged)
    Q_PROPERTY(QVector3D direction READ direction WRITE setDirection NOTIFY directionChanged)
    QML_NAMED_ELEMENT(Gravity3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleGravity(QQuick3DNode *parent = nullptr);

    float magnitude() const;
    const QVector3D &direction() const;

public Q_SLOTS:
    void setDirection(const QVector3D &direction);
    void setMagnitude(float magnitude);

Q_SIGNALS:
    void magnitudeChanged();
    void directionChanged();

protected:
    void affectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time) override;

private:
    float m_magnitude = 100.0f;
    QVector3D m_direction = {0.0f, -1.0f, 0.0f};
    QVector3D m_directionNormalized = {0.0f, -1.0f, 0.0f};
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEGRAVITY_H
