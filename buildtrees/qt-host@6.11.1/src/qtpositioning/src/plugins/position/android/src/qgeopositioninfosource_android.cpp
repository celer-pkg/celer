// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosource_android_p.h"
#include "jnipositioning.h"
#include <QGeoPositionInfo>
#include <QVariantMap>

using namespace Qt::StringLiterals;

static constexpr int kUpdateFromColdStart = 2 * 60 * 1000;
static constexpr int kRegularUpdatesTimerInterval = 30 * 1000;

static constexpr auto kUseAltitudeConverter = "useMslAltitude"_L1;

QGeoPositionInfoSourceAndroid::QGeoPositionInfoSourceAndroid(const QVariantMap &parameters,
                                                             QObject *parent) :
    QGeoPositionInfoSource(parent)
{
    androidClassKeyForUpdate = AndroidPositioning::registerPositionInfoSource(this);
    androidClassKeyForSingleRequest = AndroidPositioning::registerPositionInfoSource(this);

    parseParameters(parameters);

    //by default use all methods
    setPreferredPositioningMethods(AllPositioningMethods);

    m_requestTimer.setSingleShot(true);
    connect(&m_requestTimer, &QTimer::timeout, this,
            &QGeoPositionInfoSourceAndroid::requestTimeout);

    m_regularUpdatesTimer.setSingleShot(false);
    connect(&m_regularUpdatesTimer, &QTimer::timeout, this,
            &QGeoPositionInfoSourceAndroid::regularUpdatesTimeout);
}

QGeoPositionInfoSourceAndroid::~QGeoPositionInfoSourceAndroid()
{
    stopUpdates();

    if (m_requestTimer.isActive()) {
        m_requestTimer.stop();
        AndroidPositioning::stopUpdates(androidClassKeyForSingleRequest);
    }

    AndroidPositioning::unregisterPositionInfoSource(androidClassKeyForUpdate);
    AndroidPositioning::unregisterPositionInfoSource(androidClassKeyForSingleRequest);
}

void QGeoPositionInfoSourceAndroid::setUpdateInterval(int msec)
{
    int previousInterval = updateInterval();
    msec = (((msec > 0) && (msec < minimumUpdateInterval())) || msec < 0)? minimumUpdateInterval() : msec;

    if (msec == previousInterval)
        return;

    QGeoPositionInfoSource::setUpdateInterval(msec);

    if (updatesRunning)
        reconfigureRunningSystem();
}

QGeoPositionInfo QGeoPositionInfoSourceAndroid::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    return AndroidPositioning::lastKnownPosition(fromSatellitePositioningMethodsOnly,
                                                 useAltitudeConverter());
}

QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceAndroid::supportedPositioningMethods() const
{
    return AndroidPositioning::availableProviders();
}

void QGeoPositionInfoSourceAndroid::setPreferredPositioningMethods(QGeoPositionInfoSource::PositioningMethods methods)
{
    PositioningMethods previousPreferredPositioningMethods = preferredPositioningMethods();
    QGeoPositionInfoSource::setPreferredPositioningMethods(methods);
    if (previousPreferredPositioningMethods == preferredPositioningMethods())
        return;

    if (updatesRunning)
        reconfigureRunningSystem();
}

int QGeoPositionInfoSourceAndroid::minimumUpdateInterval() const
{
    return 50;
}

QGeoPositionInfoSource::Error QGeoPositionInfoSourceAndroid::error() const
{
    return m_error;
}

bool QGeoPositionInfoSourceAndroid::useAltitudeConverter() const
{
    return m_useAltitudeConverter;
}

void QGeoPositionInfoSourceAndroid::setError(Error error)
{
    m_error = error;
    if (error != QGeoPositionInfoSource::NoError)
        emit QGeoPositionInfoSource::errorOccurred(m_error);
}

void QGeoPositionInfoSourceAndroid::parseParameters(const QVariantMap &parameters)
{
    m_useAltitudeConverter = parameters.value(kUseAltitudeConverter, false).toBool();
}

void QGeoPositionInfoSourceAndroid::startUpdates()
{
    if (updatesRunning)
        return;

    m_error = QGeoPositionInfoSource::NoError;

    if (preferredPositioningMethods() == 0) {
        setError(UnknownSourceError);
        return;
    }

    updatesRunning = true;
    // Start calculating updates from now.
    m_lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    m_regularUpdatesErrorRaised = false;
    QGeoPositionInfoSource::Error error = AndroidPositioning::startUpdates(androidClassKeyForUpdate);
    if (error != QGeoPositionInfoSource::NoError) {
        updatesRunning = false;
        setError(error);
    } else {
        m_regularUpdatesTimer.start(kRegularUpdatesTimerInterval);
    }
}

void QGeoPositionInfoSourceAndroid::stopUpdates()
{
    if (!updatesRunning)
        return;

    updatesRunning = false;
    m_regularUpdatesTimer.stop();
    AndroidPositioning::stopUpdates(androidClassKeyForUpdate);
}

void QGeoPositionInfoSourceAndroid::requestUpdate(int timeout)
{
    if (m_requestTimer.isActive())
        return;

    m_error = QGeoPositionInfoSource::NoError;

    if (timeout != 0 && timeout < minimumUpdateInterval()) {
        setError(QGeoPositionInfoSource::UpdateTimeoutError);
        return;
    }

    if (timeout == 0)
        timeout = kUpdateFromColdStart;

    m_requestTimer.start(timeout);

    // if updates already running with interval equal to timeout
    // then we wait for next update coming through
    // assume that a single update will not be quicker than regular updates anyway
    if (updatesRunning && updateInterval() <= timeout)
        return;

    const QGeoPositionInfoSource::Error error =
            AndroidPositioning::requestUpdate(androidClassKeyForSingleRequest, timeout);
    if (error != QGeoPositionInfoSource::NoError) {
        m_requestTimer.stop();
        setError(error);
    }
}

void QGeoPositionInfoSourceAndroid::processPositionUpdate(const QGeoPositionInfo &pInfo)
{
    //single update request and served as part of regular update
    if (m_requestTimer.isActive())
        m_requestTimer.stop();

    m_lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    m_regularUpdatesErrorRaised = false;

    emit positionUpdated(pInfo);
}

// Might still be called multiple times (once for each provider)
void QGeoPositionInfoSourceAndroid::processSinglePositionUpdate(const QGeoPositionInfo &pInfo)
{
    //timeout but we received a late update -> ignore
    if (!m_requestTimer.isActive())
        return;

    queuedSingleUpdates.append(pInfo);
    // Calculate the maximum amount of possibly received updates. It depends on
    // preferred positioning methods. Two updates if we have both Satellite and
    // Network, and only one otherwise.
    const qsizetype maxPossibleUpdates =
            (preferredPositioningMethods() == QGeoPositionInfoSource::AllPositioningMethods)
            ? 2 : 1;
    // If we get the maximum number of updates, we do not need to wait for more
    if (queuedSingleUpdates.size() == maxPossibleUpdates) {
        m_requestTimer.stop();
        requestTimeout();
    }
}

void QGeoPositionInfoSourceAndroid::locationProviderDisabled()
{
    if (updatesRunning && !m_regularUpdatesErrorRaised) {
        m_regularUpdatesErrorRaised = true;
        setError(QGeoPositionInfoSource::UpdateTimeoutError);
    }

    setError(QGeoPositionInfoSource::ClosedError);
}

void QGeoPositionInfoSourceAndroid::locationProvidersChanged()
{
    emit supportedPositioningMethodsChanged();
}

void QGeoPositionInfoSourceAndroid::requestTimeout()
{
    AndroidPositioning::stopUpdates(androidClassKeyForSingleRequest);
    //no queued update to process -> timeout

    if (queuedSingleUpdates.isEmpty()) {
        setError(QGeoPositionInfoSource::UpdateTimeoutError);
        return;
    }

    auto byAccuracy = [](const QGeoPositionInfo &info, const QGeoPositionInfo &best) {
        //anything newer by 20s is always better
        const qint64 timeDelta = best.timestamp().secsTo(info.timestamp());
        if (abs(timeDelta) > 20)
            return timeDelta > 0;

        //compare accuracy
        if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy) &&
                best.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
        {
            return info.attribute(QGeoPositionInfo::HorizontalAccuracy) <
                    best.attribute(QGeoPositionInfo::HorizontalAccuracy);
        }

        //prefer info with accuracy information
        if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
            return true;

        return false;
    };

    QGeoPositionInfo best = *std::min_element(queuedSingleUpdates.begin(),
                                              queuedSingleUpdates.end(), byAccuracy);
    queuedSingleUpdates.clear();
    emit positionUpdated(best);
}

void QGeoPositionInfoSourceAndroid::regularUpdatesTimeout()
{
    if (!m_regularUpdatesErrorRaised) {
        const auto now = QDateTime::currentMSecsSinceEpoch();
        if ((now - m_lastUpdateTime) > (updateInterval() + kUpdateFromColdStart)) {
            m_regularUpdatesErrorRaised = true;
            setError(QGeoPositionInfoSource::UpdateTimeoutError);
        }
    }
}

/*
  Updates the system assuming that updateInterval
  and/or preferredPositioningMethod have changed.
 */
void QGeoPositionInfoSourceAndroid::reconfigureRunningSystem()
{
    if (!updatesRunning)
        return;

    stopUpdates();
    startUpdates();
}
