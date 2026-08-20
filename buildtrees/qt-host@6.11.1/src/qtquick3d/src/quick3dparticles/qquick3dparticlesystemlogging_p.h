// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLESYSTEMLOGGING_H
#define QQUICK3DPARTICLESYSTEMLOGGING_H

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
#include <QQmlEngine>
#include <private/qglobal_p.h>

#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleSystemLogging : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int loggingInterval READ loggingInterval WRITE setLoggingInterval NOTIFY loggingIntervalChanged)
    Q_PROPERTY(int updates READ updates NOTIFY updatesChanged)
    Q_PROPERTY(int particlesMax READ particlesMax NOTIFY particlesMaxChanged)
    Q_PROPERTY(int particlesUsed READ particlesUsed NOTIFY particlesUsedChanged)
    Q_PROPERTY(float time READ time NOTIFY timeChanged)
    Q_PROPERTY(float timeAverage READ timeAverage NOTIFY timeAverageChanged)
    Q_PROPERTY(float timeDeviation  READ timeDeviation NOTIFY timeDeviationChanged REVISION(6, 3))
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleSystemLogging(QObject *parent = nullptr);

    int loggingInterval() const;
    int updates() const;
    int particlesMax() const;
    int particlesUsed() const;
    float time() const;
    float timeAverage() const;
    Q_REVISION(6, 3) float timeDeviation() const;

public Q_SLOTS:
    void setLoggingInterval(int interval);

Q_SIGNALS:
    void loggingIntervalChanged();
    void updatesChanged();
    void particlesMaxChanged();
    void particlesUsedChanged();
    void timeChanged();
    void timeAverageChanged();
    Q_REVISION(6, 3) void timeDeviationChanged();

private:
    void updateTimes(qint64 time);
    void resetData();

    friend class QQuick3DParticleSystem;
    int m_loggingInterval = 1000;
    int m_updates = 0;
    int m_particlesMax = 0;
    int m_particlesUsed = 0;
    float m_time = 0.0f;
    float m_timeAverage = 0.0f;
    float m_timeDeviation = 0.0f;
    QList<float> m_totalTimesList;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLESYSTEMLOGGING_H
