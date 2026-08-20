// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTimer>
#include "logfilepositionsource.h"

LogFilePositionSource::LogFilePositionSource(const QList<QByteArray> &data, QObject *parent)
    : QGeoPositionInfoSource(parent),
      timer(new QTimer(this)),
      lines(data)
{
    connect(timer, SIGNAL(timeout()), this, SLOT(readNextPosition()));

    if (lines.isEmpty())
        qWarning() << "Error: the input data is empty!";
    else
        index = 0; // ready to read
}

QGeoPositionInfo LogFilePositionSource::lastKnownPosition(bool /*fromSatellitePositioningMethodsOnly*/) const
{
    return lastPosition;
}

LogFilePositionSource::PositioningMethods LogFilePositionSource::supportedPositioningMethods() const
{
    return AllPositioningMethods;
}

int LogFilePositionSource::minimumUpdateInterval() const
{
    return 50;
}

void LogFilePositionSource::startUpdates()
{
    lastError = QGeoPositionInfoSource::NoError;
    int interval = updateInterval();
    if (interval < minimumUpdateInterval())
        interval = minimumUpdateInterval();

    if (!timer->isActive()) {
        if (QMetaObject::invokeMethod(timer, "start", Q_ARG(int, interval)))
            emit updatesStarted();
    }
}

void LogFilePositionSource::stopUpdates()
{
    if (timer->isActive()) {
        if (QMetaObject::invokeMethod(timer, "stop"))
            emit updatesStopped();
    }
}

void LogFilePositionSource::requestUpdate(int /*timeout*/)
{
    // For simplicity, ignore timeout - assume that if data is not available
    // now, no data will be added to the file later
    lastError = QGeoPositionInfoSource::NoError;
    if (canReadLine()) {
        readNextPosition();
    } else {
        lastError = QGeoPositionInfoSource::UpdateTimeoutError;
        emit errorOccurred(lastError);
    }
}

void LogFilePositionSource::readNextPosition()
{
    if (canReadLine()) {
        const QByteArray line = lines.at(index);
        if (!line.isEmpty()) {
            QList<QByteArray> data = line.split(' ');
            double latitude;
            double longitude;
            bool hasLatitude = false;
            bool hasLongitude = false;
            QDateTime timestamp = QDateTime::fromString(QString(data.value(0)), Qt::ISODate);
            latitude = data.value(1).toDouble(&hasLatitude);
            longitude = data.value(2).toDouble(&hasLongitude);

            if (hasLatitude && hasLongitude && timestamp.isValid()) {
                QGeoCoordinate coordinate(latitude, longitude);
                QGeoPositionInfo info(coordinate, timestamp);
                if (info.isValid()) {
                    lastPosition = info;
                    emit positionUpdated(info);
                }
            }
        }
        index++;
    } else if (!noDataEmitted) {
        emit noDataLeft();
        noDataEmitted = true;
    }
}

bool LogFilePositionSource::canReadLine() const
{
    return (index >= 0) && (index < lines.size());
}

QGeoPositionInfoSource::Error LogFilePositionSource::error() const
{
    return lastError;
}
