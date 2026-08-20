// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef LOGFILEPOSITIONSOURCE_H
#define LOGFILEPOSITIONSOURCE_H

#include <QtPositioning/qgeopositioninfosource.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class LogFilePositionSource : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    // This class is optimized to reduce the file IO.
    // Initially it was reading the file line-by-line.
    // It does not modify the data, so it was optimized to just hold the
    // const reference to the pre-existing data, that can now be read once
    // for all the instances of this class (for example, during the
    // initTestCase() call).
    LogFilePositionSource(const QList<QByteArray> &data, QObject *parent = 0);

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;

    PositioningMethods supportedPositioningMethods() const override;
    int minimumUpdateInterval() const override;
    Error error() const override;

signals:
    void noDataLeft();
    void updatesStarted();
    void updatesStopped();

public slots:
    virtual void startUpdates() override;
    virtual void stopUpdates() override;

    virtual void requestUpdate(int timeout = 5000) override;

private slots:
    void readNextPosition();

private:
    bool canReadLine() const;

    QTimer *timer;
    QGeoPositionInfo lastPosition;
    Error lastError = QGeoPositionInfoSource::NoError;
    const QList<QByteArray> &lines;
    qsizetype index = -1;
    bool noDataEmitted = false;
};

#endif
