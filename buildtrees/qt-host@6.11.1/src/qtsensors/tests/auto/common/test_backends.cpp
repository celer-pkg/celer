// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QVariantMap>
#include "qsensorbackend.h"


typedef QSensorBackend* (*CreateFunc) (QSensor *sensor);
class Record
{
public:
    QByteArray type;
    CreateFunc func;
};
static QList<Record> records;

static bool registerTestBackend(const char *className, CreateFunc func)
{
    Record record;
    record.type = className;
    record.func = func;
    records << record;
    return true;
}

#define REGISTER_TOO
#include "test_backends.h"
#include <QDebug>

// The sensor-to-backend mapping is maintained in order to be able to change
// the sensor reading values in the backend
static QMap<QSensor*, QSensorBackend*> sensorToBackend;

void set_test_backend_busy(QSensor* sensor, bool busy)
{
    Q_ASSERT(sensor->isConnectedToBackend());
    QSensorBackend* backend = sensorToBackend.value(sensor);
    backend->sensorBusy(busy);
}

void set_test_backend_reading(QSensor* sensor, const QVariantMap& values)
{
    Q_ASSERT(sensor->isConnectedToBackend());
    QSensorBackend* backend = sensorToBackend.value(sensor);
    // timestamp is common to all readings
    if (values.contains("timestamp"))
        backend->reading()->setTimestamp(values["timestamp"].toInt());
    if (sensor->type() == "QAccelerometer") {
        QAccelerometerReading* reading = static_cast<QAccelerometerReading*>(backend->reading());
        if (values.contains("x")) reading->setX(values["x"].value<qreal>());
        if (values.contains("y")) reading->setY(values["y"].value<qreal>());
        if (values.contains("z")) reading->setZ(values["z"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QPressureSensor") {
        QPressureReading* reading = static_cast<QPressureReading*>(backend->reading());
        if (values.contains("pressure")) reading->setPressure(values["pressure"].value<qreal>());
        if (values.contains("temperature")) reading->setTemperature(values["temperature"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QGyroscope") {
        QGyroscopeReading* reading = static_cast<QGyroscopeReading*>(backend->reading());
        if (values.contains("x")) reading->setX(values["x"].value<qreal>());
        if (values.contains("y")) reading->setY(values["y"].value<qreal>());
        if (values.contains("z")) reading->setZ(values["z"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QTapSensor") {
        QTapReading* reading = static_cast<QTapReading*>(backend->reading());
        if (values.contains("doubleTap")) reading->setDoubleTap(values["doubleTap"].value<bool>());
        if (values.contains("tapDirection")) reading->setTapDirection(QTapReading::TapDirection(values["tapDirection"].toInt()));
        backend->newReadingAvailable();
    } else if (sensor->type() == "QCompass") {
        QCompassReading* reading = static_cast<QCompassReading*>(backend->reading());
        if (values.contains("azimuth")) reading->setAzimuth(values["azimuth"].value<qreal>());
        if (values.contains("calibrationLevel")) reading->setCalibrationLevel(values["calibrationLevel"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QProximitySensor") {
        QProximityReading* reading = static_cast<QProximityReading*>(backend->reading());
        reading->setClose(values["near"].value<bool>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QOrientationSensor") {
        QOrientationReading* reading = static_cast<QOrientationReading*>(backend->reading());
        reading->setOrientation(QOrientationReading::Orientation(values["orientation"].toInt()));
        backend->newReadingAvailable();
    } else if (sensor->type() == "QAmbientLightSensor") {
        QAmbientLightReading* reading = static_cast<QAmbientLightReading*>(backend->reading());
        reading->setLightLevel(QAmbientLightReading::LightLevel(values["lightLevel"].toInt()));
        backend->newReadingAvailable();
    } else if (sensor->type() == "QMagnetometer") {
        QMagnetometerReading* reading = static_cast<QMagnetometerReading*>(backend->reading());
        if (values.contains("x")) reading->setX(values["x"].value<qreal>());
        if (values.contains("y")) reading->setY(values["y"].value<qreal>());
        if (values.contains("z")) reading->setZ(values["z"].value<qreal>());
        if (values.contains("calibrationLevel")) reading->setCalibrationLevel(values["calibrationLevel"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QLidSensor") {
        QLidReading* reading = static_cast<QLidReading*>(backend->reading());
        if (values.contains("backLidClosed")) reading->setBackLidClosed(values["backLidClosed"].value<bool>());
        if (values.contains("frontLidClosed")) reading->setFrontLidClosed(values["frontLidClosed"].value<bool>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QTiltSensor") {
        QTiltReading* reading = static_cast<QTiltReading*>(backend->reading());
        if (values.contains("yRotation")) reading->setYRotation(values["yRotation"].value<qreal>());
        if (values.contains("xRotation")) reading->setXRotation(values["xRotation"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QRotationSensor") {
        QRotationReading* reading = static_cast<QRotationReading*>(backend->reading());
        reading->setFromEuler(values["x"].value<qreal>(), values["y"].value<qreal>(), values["z"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QHumiditySensor") {
        QHumidityReading* reading = static_cast<QHumidityReading*>(backend->reading());
        if (values.contains("relativeHumidity")) reading->setRelativeHumidity(values["relativeHumidity"].value<qreal>());
        if (values.contains("absoluteHumidity")) reading->setAbsoluteHumidity(values["absoluteHumidity"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QAmbientTemperatureSensor") {
        QAmbientTemperatureReading* reading = static_cast<QAmbientTemperatureReading*>(backend->reading());
        reading->setTemperature(values["temperature"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QLightSensor") {
        QLightReading* reading = static_cast<QLightReading*>(backend->reading());
        reading->setLux(values["illuminance"].value<qreal>());
        backend->newReadingAvailable();
    } else if (sensor->type() == "QIRProximitySensor") {
        QIRProximityReading* reading = static_cast<QIRProximityReading*>(backend->reading());
        reading->setReflectance(values["reflectance"].value<qreal>());
        backend->newReadingAvailable();
    } else {
        qWarning() << "Unsupported test sensor backend:" << sensor->type();
    }
}

class BackendFactory : public QSensorBackendFactory
{
    QSensorBackend *createBackend(QSensor *sensor) override
    {
        for (const Record &record : records) {
            if (sensor->identifier() == record.type) {
                QSensorBackend* backend = record.func(sensor);
                sensorToBackend.insert(sensor, backend);
                return backend;
            }
        }
        return nullptr;
    }
};
static BackendFactory factory;

void register_test_backends()
{
    sensorToBackend.clear();
    for (const Record &record : records)
        QSensorManager::registerBackend(record.type, record.type, &factory);
}

void unregister_test_backends()
{
    sensorToBackend.clear();
    for (const Record &record : records)
        QSensorManager::unregisterBackend(record.type, record.type);
}
