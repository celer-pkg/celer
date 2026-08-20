// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QDebug>

#include "qgeosatelliteinfosource_android_p.h"
#include "jnipositioning.h"

Q_DECLARE_METATYPE(QList<QGeoSatelliteInfo>)

#define UPDATE_FROM_COLD_START 2*60*1000

QGeoSatelliteInfoSourceAndroid::QGeoSatelliteInfoSourceAndroid(QObject *parent) :
    QGeoSatelliteInfoSource(parent), m_error(NoError), updatesRunning(false)
{
    qRegisterMetaType< QGeoSatelliteInfo >();
    qRegisterMetaType< QList<QGeoSatelliteInfo> >();
    androidClassKeyForUpdate = AndroidPositioning::registerPositionInfoSource(this);
    androidClassKeyForSingleRequest = AndroidPositioning::registerPositionInfoSource(this);

    requestTimer.setSingleShot(true);
    QObject::connect(&requestTimer, SIGNAL(timeout()),
                     this, SLOT(requestTimeout()));
}

QGeoSatelliteInfoSourceAndroid::~QGeoSatelliteInfoSourceAndroid()
{
    stopUpdates();

    if (requestTimer.isActive()) {
        requestTimer.stop();
        AndroidPositioning::stopUpdates(androidClassKeyForSingleRequest);
    }

    AndroidPositioning::unregisterPositionInfoSource(androidClassKeyForUpdate);
    AndroidPositioning::unregisterPositionInfoSource(androidClassKeyForSingleRequest);
}


void QGeoSatelliteInfoSourceAndroid::setUpdateInterval(int msec)
{
    int previousInterval = updateInterval();
    msec = (((msec > 0) && (msec < minimumUpdateInterval())) || msec < 0)? minimumUpdateInterval() : msec;

    if (msec == previousInterval)
        return;

    QGeoSatelliteInfoSource::setUpdateInterval(msec);

    if (updatesRunning)
        reconfigureRunningSystem();
}

int QGeoSatelliteInfoSourceAndroid::minimumUpdateInterval() const
{
    return 50;
}

QGeoSatelliteInfoSource::Error QGeoSatelliteInfoSourceAndroid::error() const
{
    return m_error;
}

void QGeoSatelliteInfoSourceAndroid::startUpdates()
{
    if (updatesRunning)
        return;

    updatesRunning = true;

    m_error = QGeoSatelliteInfoSource::NoError;

    QGeoSatelliteInfoSource::Error error = AndroidPositioning::startSatelliteUpdates(
                androidClassKeyForUpdate, false, updateInterval());
    if (error != QGeoSatelliteInfoSource::NoError) {
        updatesRunning = false;
        setError(error);
    }
}

void QGeoSatelliteInfoSourceAndroid::stopUpdates()
{
    if (!updatesRunning)
        return;

    updatesRunning = false;
    AndroidPositioning::stopUpdates(androidClassKeyForUpdate);
}

void QGeoSatelliteInfoSourceAndroid::requestUpdate(int timeout)
{
    if (requestTimer.isActive())
        return;

    m_error = QGeoSatelliteInfoSource::NoError;

    if (timeout != 0 && timeout < minimumUpdateInterval()) {
        setError(QGeoSatelliteInfoSource::UpdateTimeoutError);
        return;
    }

    if (timeout == 0)
        timeout = UPDATE_FROM_COLD_START;

    requestTimer.start(timeout);

    // if updates already running with interval equal or less then timeout
    // then we wait for next update coming through
    // assume that a single update will not be quicker than regular updates anyway
    if (updatesRunning && updateInterval() <= timeout)
        return;

    QGeoSatelliteInfoSource::Error error = AndroidPositioning::startSatelliteUpdates(
                androidClassKeyForSingleRequest, true, timeout);
    if (error != QGeoSatelliteInfoSource::NoError) {
        requestTimer.stop();
        setError(error);
    }
}

void
QGeoSatelliteInfoSourceAndroid::processSatelliteUpdate(const QList<QGeoSatelliteInfo> &satsInView,
                                                       const QList<QGeoSatelliteInfo> &satsInUse,
                                                       bool isSingleUpdate)
{
    if (!isSingleUpdate) {
        //if single update is requested while regular updates are running
        if (requestTimer.isActive())
            requestTimer.stop();
        emit QGeoSatelliteInfoSource::satellitesInViewUpdated(satsInView);
        emit QGeoSatelliteInfoSource::satellitesInUseUpdated(satsInUse);
        return;
    }

    m_satsInView = satsInView;
    m_satsInUse = satsInUse;

    if (!m_satsInView.isEmpty() || !m_satsInUse.isEmpty()) {
        requestTimer.stop();
        requestTimeout();
    }
}

void QGeoSatelliteInfoSourceAndroid::requestTimeout()
{
    AndroidPositioning::stopUpdates(androidClassKeyForSingleRequest);

    if (m_satsInView.isEmpty() && m_satsInUse.isEmpty()) {
        setError(QGeoSatelliteInfoSource::UpdateTimeoutError);
        return;
    }

    emit QGeoSatelliteInfoSource::satellitesInViewUpdated(m_satsInView);
    emit QGeoSatelliteInfoSource::satellitesInUseUpdated(m_satsInUse);

    m_satsInUse.clear();
    m_satsInView.clear();
}

/*
  Updates the system assuming that updateInterval
  and/or preferredPositioningMethod have changed.
 */
void QGeoSatelliteInfoSourceAndroid::reconfigureRunningSystem()
{
    if (!updatesRunning)
        return;

    stopUpdates();
    startUpdates();
}

void QGeoSatelliteInfoSourceAndroid::setError(QGeoSatelliteInfoSource::Error error)
{
    m_error = error;
    if (m_error != QGeoSatelliteInfoSource::NoError)
        emit QGeoSatelliteInfoSource::errorOccurred(m_error);
}

void QGeoSatelliteInfoSourceAndroid::locationProviderDisabled()
{
    setError(QGeoSatelliteInfoSource::ClosedError);
}
