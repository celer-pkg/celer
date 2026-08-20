// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlesystemlogging_p.h"
#include <float.h> // FLT_MAX

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleSystem3DLogging
    \inherits QtObject
    \inqmlmodule QtQuick3D.Particles3D
    \brief Provides information of the particle system.
    \since 6.2

    The \c ParticleSystem3DLogging type provides information about particle system statistics.
    This element cannot be created directly, but can be retrieved from a \l ParticleSystem3D.
*/

QQuick3DParticleSystemLogging::QQuick3DParticleSystemLogging(QObject *parent)
    : QObject(parent)
{
}

/*!
    \qmlproperty int ParticleSystem3DLogging::loggingInterval

    This property defines in milliseconds how often the logging data is updated.
    Longer update time increases the accuracy of \l {ParticleSystem3DLogging::time}{time} and
    \l {ParticleSystem3DLogging::timeAverage}{timeAverage}, while shorter update times keep
    the data more up to date.

    The default value is \c 1000.
*/

int QQuick3DParticleSystemLogging::loggingInterval() const
{
    return m_loggingInterval;
}

void QQuick3DParticleSystemLogging::setLoggingInterval(int interval)
{
    if (m_loggingInterval == interval)
        return;

    m_loggingInterval = interval;
    Q_EMIT loggingIntervalChanged();
}

/*!
    \qmlproperty int ParticleSystem3DLogging::updates
    \readonly

    This property holds the amount of particle system updates since the last logging.
    When \a loggingInterval is 1000 (default), this can be considered to match the fps.
*/
int QQuick3DParticleSystemLogging::updates() const
{
    return m_updates;
}

/*!
    \qmlproperty int ParticleSystem3DLogging::particlesMax
    \readonly

    This property holds the maximum amount of particles in this system.
    Maximum amount is the sum of system particles \l {Particle3D::maxAmount}{maxAmount} properties.
*/
int QQuick3DParticleSystemLogging::particlesMax() const
{
    return m_particlesMax;
}

/*!
    \qmlproperty int ParticleSystem3DLogging::particlesUsed
    \readonly

    This property holds the amount of particles currently in use in this system.
    This value should be close to \l particlesMax at some point of particle system
    animation. If it is much smaller, consider decreasing \l {Particle3D::maxAmount}{maxAmount} values.
    If it reaches \l particlesMax, particles are used effectively but it can also mean that
    particles are reused before they reach the end of their \l {ParticleEmitter3D::lifeSpan}{lifeSpan}.
    In this case, consider increasing the \l {Particle3D::maxAmount}{maxAmount} values.
*/
int QQuick3DParticleSystemLogging::particlesUsed() const
{
    return m_particlesUsed;
}

/*!
    \qmlproperty real ParticleSystem3DLogging::time
    \readonly

    This property holds the time in milliseconds used for emitting and animating particles
    in each frame.
*/
float QQuick3DParticleSystemLogging::time() const
{
    return m_time;
}

/*!
    \qmlproperty real ParticleSystem3DLogging::timeAverage
    \readonly

    This property holds the average time in milliseconds used for emitting and animating
    particles in each frame. Average is calculated from the middle 50% of the past
    max 100 logging updates. So when \l loggingInterval is 1000, this represents an
    average \l time in past 100 seconds. This can be used for measuring the performance
    of current particle system.
*/
float QQuick3DParticleSystemLogging::timeAverage() const
{
    return m_timeAverage;
}

/*!
    \qmlproperty real ParticleSystem3DLogging::timeDeviation
    \since 6.3
    \readonly

    This property holds the deviation of the average times in milliseconds.
    The value is the difference between maximum and minimum values of middle 50%
    of the results, also called interquartile range (IQR).
    Bigger deviation means that the times fluctuate more so \l timeAverage
    can be considered to be less accurate.
*/
float QQuick3DParticleSystemLogging::timeDeviation() const
{
    return m_timeDeviation;
}

void QQuick3DParticleSystemLogging::updateTimes(qint64 time)
{
    m_time = float(time / 1000000.0) / m_updates;

    m_totalTimesList.append(m_time);

    // Keep max amount of times stored and remove the oldest values
    const int MAX_TIMES = 100;
    if (m_totalTimesList.size() > MAX_TIMES)
        m_totalTimesList.removeFirst();

    auto sortedTimes = m_totalTimesList;
    std::sort(sortedTimes.begin(), sortedTimes.end());

    // Calculate average from stored times.
    // Only take into account the middle 50% of the values.
    // This gives us interquartile range (IQR) and the average time among IQR.
    if (sortedTimes.size() > 5) {
        // Skip 25%, count 50%, so maxItem at 75%
        const int skipAmount = roundf(0.25f * float(sortedTimes.size()));
        const int maxItem = sortedTimes.size() - skipAmount;
        int countAmount = 0;
        double totalTime = 0.0;
        float maxTime = 0.0f;
        float minTime = FLT_MAX;
        for (int i = skipAmount; i < maxItem; i++) {
            const float time = sortedTimes.at(i);
            totalTime += time;
            minTime = std::min(minTime, time);
            maxTime = std::max(maxTime, time);
            countAmount++;
        }
        m_timeAverage = float(totalTime / countAmount);
        m_timeDeviation = maxTime - minTime;
        Q_EMIT timeAverageChanged();
        Q_EMIT timeDeviationChanged();
    }
    Q_EMIT timeChanged();
}

void QQuick3DParticleSystemLogging::resetData()
{
    m_updates = 0;
    m_particlesMax = 0;
    m_particlesUsed = 0;
    m_time = 0.0f;
    m_timeAverage = 0.0f;
    m_timeDeviation = 0.0f;
    m_totalTimesList.clear();
    Q_EMIT updatesChanged();
    Q_EMIT particlesMaxChanged();
    Q_EMIT particlesUsedChanged();
    Q_EMIT timeChanged();
    Q_EMIT timeAverageChanged();
    Q_EMIT timeDeviationChanged();
}

QT_END_NAMESPACE
