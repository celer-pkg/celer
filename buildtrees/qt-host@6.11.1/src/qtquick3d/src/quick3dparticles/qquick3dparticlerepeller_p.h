// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEREPELLER_H
#define QQUICK3DPARTICLEREPELLER_H

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

#include <QtQuick3DParticles/private/qquick3dparticleaffector_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleRepeller : public QQuick3DParticleAffector
{
    Q_OBJECT
    Q_PROPERTY(float radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(float outerRadius READ outerRadius WRITE setOuterRadius NOTIFY outerRadiusChanged)
    Q_PROPERTY(float strength READ strength WRITE setStrength NOTIFY strengthChanged)
    QML_NAMED_ELEMENT(Repeller3D)
    QML_ADDED_IN_VERSION(6, 4)
public:
    QQuick3DParticleRepeller(QQuick3DNode *parent = nullptr);

    float radius() const;
    float outerRadius() const;
    float strength() const;

public Q_SLOTS:
    void setRadius(float radius);
    void setOuterRadius(float radius);
    void setStrength(float strength);

Q_SIGNALS:
    void radiusChanged();
    void outerRadiusChanged();
    void strengthChanged();

protected:
    void prepareToAffect() override;
    void affectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time) override;

private:
    float m_radius = 0.0f;
    float m_outerRadius = 50.0f;
    float m_strength = 50.0f;
};

QT_END_NAMESPACE

#endif
