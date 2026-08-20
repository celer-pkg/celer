// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef LOGFILEPOSITIONSOURCE_H
#define LOGFILEPOSITIONSOURCE_H

#include <QtPositioning/qgeopositioninfosource.h>

QT_BEGIN_NAMESPACE
class QFile;
class QTimer;
QT_END_NAMESPACE

class LogFilePositionSource : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    LogFilePositionSource(QObject *parent = 0);

    QGeoPositionInfo lastKnownPosition(bool satelliteMethodsOnly = false) const override;

    PositioningMethods supportedPositioningMethods() const override;
    int minimumUpdateInterval() const override;
    Error error() const override;

public slots:
    virtual void startUpdates() override;
    virtual void stopUpdates() override;

    virtual void requestUpdate(int timeout = 5000) override;

private slots:
    void readNextPosition();

private:
    QFile *logFile;
    QTimer *timer;
    QGeoPositionInfo lastPosition;
    Error lastError = QGeoPositionInfoSource::NoError;
};

#endif
