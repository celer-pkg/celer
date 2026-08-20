// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLESCALEAFFECTOR_H
#define QQUICK3DPARTICLESCALEAFFECTOR_H

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
#include <qeasingcurve.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleScaleAffector : public QQuick3DParticleAffector
{
    Q_OBJECT
    Q_PROPERTY(float minSize READ minSize WRITE setMinSize NOTIFY minSizeChanged)
    Q_PROPERTY(float maxSize READ maxSize WRITE setMaxSize NOTIFY maxSizeChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(ScalingType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QEasingCurve easingCurve READ easingCurve WRITE setEasingCurve NOTIFY easingCurveChanged)
    QML_NAMED_ELEMENT(ScaleAffector3D)
    QML_ADDED_IN_VERSION(6, 4)
public:
    QQuick3DParticleScaleAffector(QQuick3DNode *parent = nullptr);

    enum ScalingType
    {
        Linear,
        SewSaw,
        SineWave,
        AbsSineWave,
        Step,
        SmoothStep,
    };
    Q_ENUM(ScalingType)

    float minSize() const;
    float maxSize() const;
    int duration() const;
    ScalingType type() const;
    QEasingCurve easingCurve() const;

public Q_SLOTS:
    void setMinSize(float size);
    void setMaxSize(float size);
    void setDuration(int duration);
    void setType(ScalingType type);
    void setEasingCurve(const QEasingCurve &curve);

Q_SIGNALS:
    void minSizeChanged();
    void maxSizeChanged();
    void durationChanged();
    void typeChanged();
    void easingCurveChanged();

protected:
    void prepareToAffect() override;
    void affectParticle(const QQuick3DParticleData &, QQuick3DParticleDataCurrent *d, float time) override;

private:
    float m_minSize = 1.0f;
    float m_maxSize = 1.0f;
    int m_duration = 1000;
    ScalingType m_type = Linear;
    QEasingCurve m_easing;
};

QT_END_NAMESPACE

#endif
