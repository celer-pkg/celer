// Copyright (C) 2017 Lorn Potter.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QFile>

class QAccelerometer;
class QOrientationSensor;
class QProximitySensor;
class QIRProximitySensor;
class QTapSensor;

class Collector : public QObject
{
    Q_OBJECT

public:
    explicit Collector(QObject *parent = 0);
    ~Collector();

public slots:
    void startCollecting();
    void stopCollecting();

private Q_SLOTS:
    void accelChanged();
    void orientationChanged();
    void proximityChanged();
    void irProximityChanged();
    void tapChanged();

private:

    QAccelerometer *accel;
    QOrientationSensor *orientation;
    QProximitySensor *proximity;
    QIRProximitySensor *irProx;
    QTapSensor *tapSensor;
    QFile dataFile;

    bool isActive;
    size_t fileCounter;

    Q_DISABLE_COPY(Collector)
};

#endif // COLLECTOR_H

