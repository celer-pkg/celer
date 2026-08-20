// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosource_wasm_p.h"
#include "qgeopositioninfosourcedispatcher_wasm_p.h"

#include <QtCore/QDebug>
#include <QtCore/QPermission>
#include <QtCore/QCoreApplication>

#include <emscripten/val.h>
#include <emscripten/bind.h>


// QGeoPositionInfoSource implemented using the Web Geolocation API

QT_BEGIN_NAMESPACE

typedef emscripten::val GeolocationCoordinates;

QGeoPositionInfoSourceWasm::QGeoPositionInfoSourceWasm(QObject *parent)
    : QGeoPositionInfoSource(parent)
{
    // QGeoPositionInfoSourceWasm should not be constructed if
    // geolocation support is missing.
    Q_ASSERT(!geolocation().isUndefined());

    m_dispatcher = QGeoPositionInfoDispatcherWasm::instance();
    m_instanceId = m_dispatcher->registerInfoSourceInstance(this);

    successCallback = val::module_property("qtGeolocationPositionSuccess").call<val>("bind", val::undefined(), m_instanceId);
    errorCallback = val::module_property("qtGeolocationPositionError").call<val>("bind", val::undefined(), m_instanceId);

}

Geolocation QGeoPositionInfoSourceWasm::geolocation()
{
    return val::global("navigator")["geolocation"];
}

QGeoPositionInfoSourceWasm::~QGeoPositionInfoSourceWasm()
{
    stopUpdates();
    m_dispatcher->unregisterInfoSourceInstance(m_instanceId);
}

void QGeoPositionInfoSourceWasm::setUpdateInterval(int msec)
{
    // If msec is 0 we send updates as data becomes available, otherwise we force msec to be equal
    // to or larger than the minimum update interval.
    if (msec != 0 && msec < minimumUpdateInterval())
        msec = minimumUpdateInterval();

    QGeoPositionInfoSource::setUpdateInterval(msec);
}

void QGeoPositionInfoSourceWasm::startUpdates()
{
    if (m_updates_running) {
        qDebug() << "startUpdates() is called, but an update is already running.";
        return;
    }

    if (preferredPositioningMethods() == 0) {
        m_lastError = QGeoPositionInfoSource::UnknownSourceError;
        return;
    }

    m_updates_running = true;
    m_lastError = QGeoPositionInfoSource::NoError;

    val options = val::object();
    options.set("maximumAge", 0);

    if (preferredPositioningMethods() & PositioningMethod::SatellitePositioningMethods)
        options.set("enableHighAccuracy", true);

    m_handlerId = geolocation().call<val>("watchPosition", successCallback, errorCallback, options);
}

void QGeoPositionInfoSourceWasm::stopUpdates()
{
    geolocation().call<void>("clearWatch", m_handlerId);
    m_updates_running = false;
}

void QGeoPositionInfoSourceWasm::requestUpdate(int timeout)
{
    if (timeout < minimumUpdateInterval()) {
        emit errorOccurred(UpdateTimeoutError);
        return;
    }

    if (m_singleUpdatePending) {
        qDebug() << "requestUpdate() called while a previous update is still pending. Ignoring.";
        return;
    }

    m_singleUpdatePending = true;
    m_lastError = QGeoPositionInfoSource::NoError;

    val options = val::object();
    options.set("maximumAge", 0); // maximum cache age: 0: don't use cache
    options.set("timeout", timeout);

    if (preferredPositioningMethods() & PositioningMethod::SatellitePositioningMethods)
        options.set("enableHighAccuracy", true);

    geolocation().call<void>("getCurrentPosition", successCallback, errorCallback, options);
}

QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceWasm::supportedPositioningMethods() const
{
    return QGeoPositionInfoSource::AllPositioningMethods;
}

void QGeoPositionInfoSourceWasm::setPreferredPositioningMethods(PositioningMethods methods)
{
    PositioningMethods previousPreferredPositioningMethods = preferredPositioningMethods();
    QGeoPositionInfoSource::setPreferredPositioningMethods(methods);
    if (previousPreferredPositioningMethods == preferredPositioningMethods())
        return;

    if (m_updates_running)
        reconfigureRunningSystem();
}


int QGeoPositionInfoSourceWasm::minimumUpdateInterval() const
{
    return 1000;
}

QGeoPositionInfo QGeoPositionInfoSourceWasm::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    Q_UNUSED(fromSatellitePositioningMethodsOnly);
    return m_lastKnownPosition;
}

QGeoPositionInfoSource::Error QGeoPositionInfoSourceWasm::error() const
{
    qDebug() << "QGeoPositionInfoSourceWasm::error";
    return m_lastError;
}

void QGeoPositionInfoSourceWasm::geolocationSuccess(GeolocationPosition position)
{
    // Populate a QGeoPositionInfo from the native GeolocationPosition. Some
    // attributes are optional and may be null.
    QGeoPositionInfo info;
    info.setTimestamp(QDateTime::fromMSecsSinceEpoch(position["timestamp"].as<double>()));

    GeolocationCoordinates coords = position["coords"];
    val latitude = coords["latitude"];
    val longitude = coords["longitude"];
    val altitude = coords["altitude"];
    info.setCoordinate(altitude.isNull() ?
        QGeoCoordinate(latitude.as<double>(), longitude.as<double>()) :
        QGeoCoordinate(latitude.as<double>(), longitude.as<double>(), altitude.as<double>())
    );

    if (val speed = coords["speed"]; !speed.isNull())
        info.setAttribute(QGeoPositionInfo::GroundSpeed, speed.as<double>());
    if (val heading = coords["heading"]; !heading.isNull())
        info.setAttribute(QGeoPositionInfo::Direction, heading.as<double>());

    info.setAttribute(QGeoPositionInfo::HorizontalAccuracy, coords["accuracy"].as<double>());
    if (val altitudeAccuracy = coords["altitudeAccuracy"]; !altitudeAccuracy.isNull())
        info.setAttribute(QGeoPositionInfo::VerticalAccuracy, altitudeAccuracy.as<double>());

    m_lastKnownPosition = info;
    m_singleUpdatePending = false;

    emit positionUpdated(info);
}

void QGeoPositionInfoSourceWasm::geolocationError(GeolocationPositionError error)
{
    qDebug() << "geolocationError";
    qDebug() << "Handler ID:" << m_handlerId.as<int>();

    int code = error["code"].as<int>();
    QString message = error["message"].as<QString>();

    QGeoPositionInfoSource::Error qtError = QGeoPositionInfoSource::UnknownSourceError;

    switch (code) {
        case 1:
            qtError = QGeoPositionInfoSource::AccessError; // -> PERMISSION_DENIED
            break;
        case 2:
            qtError = QGeoPositionInfoSource::UnknownSourceError; // -> POSITION_UNAVAILABLE
            break;
        case 3:
            qtError = QGeoPositionInfoSource::UpdateTimeoutError; // -> TIMEOUT
            break;
    }

    qDebug() << "Message: " << message;

    m_lastError = qtError;
    m_singleUpdatePending = false;

    emit errorOccurred(qtError);
}

void QGeoPositionInfoSourceWasm::reconfigureRunningSystem()
{
    if (!m_updates_running)
        return;

    stopUpdates();
    startUpdates();
}


QT_END_NAMESPACE

#include "moc_qgeopositioninfosource_wasm_p.cpp"
