// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_BACKENDS_H
#define TEST_BACKENDS_H

#include <qsensorbackend.h>
#include <QtCore/QJsonObject>

void register_test_backends();
void unregister_test_backends();
void set_test_backend_reading(QSensor* sensor, const QVariantMap& values);
void set_test_backend_busy(QSensor* sensor, bool busy);

#include <qaccelerometer.h>
#include <qambientlightsensor.h>
#include <qambienttemperaturesensor.h>
#include <qcompass.h>
#include <qgyroscope.h>
#include <qlightsensor.h>
#include <qmagnetometer.h>
#include <qorientationsensor.h>
#include <qpressuresensor.h>
#include <qproximitysensor.h>
#include <qrotationsensor.h>
#include <qtapsensor.h>
#include <qirproximitysensor.h>
#include <qtiltsensor.h>
#include <qlidsensor.h>
#include <qhumiditysensor.h>

#define PREPARE_SENSORINTERFACE_DECLS(SensorClass, ReadingClass, FilterClass, readingcode)\
    class SensorClass ## _impl : public QSensorBackend\
    {\
    public:\
        SensorClass ## _impl(QSensor *sensor);\
        void start() override;\
        void stop() override;\
        bool isFeatureSupported(QSensor::Feature feature) const override;\
    };\
    class SensorClass ## _testfilter : public FilterClass { bool filter(ReadingClass *) override; };

#define PREPARE_SENSORINTERFACE_IMPLS(SensorClass, ReadingClass, FilterClass, readingcode)\
    SensorClass ## _impl::SensorClass ##_impl(QSensor *sensor) : QSensorBackend(sensor) {}\
    void SensorClass ## _impl::start() {\
        ReadingClass *reading = setReading<ReadingClass>(0);\
        readingcode\
        newReadingAvailable();\
    }\
    void SensorClass ##_impl::stop() {}\
    bool SensorClass ##_impl::isFeatureSupported(QSensor::Feature feature) const { \
        if (feature == QSensor::Feature::SkipDuplicates) \
            return true; \
        return false; \
    } \
    bool SensorClass ## _testfilter::filter(ReadingClass *) { return true; }\
    static QSensorBackend *create_ ## SensorClass ## _impl(QSensor *sensor) { return new SensorClass ## _impl(sensor); }\
    static bool registered_ ## SensorClass = registerTestBackend(#SensorClass, create_ ## SensorClass ## _impl);

#ifdef REGISTER_TOO
#define PREPARE_SENSORINTERFACE(SensorClass, ReadingClass, FilterClass, readingcode)\
        PREPARE_SENSORINTERFACE_DECLS(SensorClass, ReadingClass, FilterClass, readingcode)\
        PREPARE_SENSORINTERFACE_IMPLS(SensorClass, ReadingClass, FilterClass, readingcode)
#else
#define PREPARE_SENSORINTERFACE(SensorClass, ReadingClass, FilterClass, readingcode)\
        PREPARE_SENSORINTERFACE_DECLS(SensorClass, ReadingClass, FilterClass, readingcode)
#endif

PREPARE_SENSORINTERFACE(QAccelerometer, QAccelerometerReading, QAccelerometerFilter, {
    reading->setTimestamp(1);
    reading->setX(1.0);
    reading->setY(1.0);
    reading->setZ(1.0);
})
PREPARE_SENSORINTERFACE(QAmbientLightSensor, QAmbientLightReading, QAmbientLightFilter, {
    reading->setLightLevel(QAmbientLightReading::Twilight);
})
PREPARE_SENSORINTERFACE(QAmbientTemperatureSensor, QAmbientTemperatureReading, QAmbientTemperatureFilter, {
    reading->setTemperature(30);
})
PREPARE_SENSORINTERFACE(QCompass, QCompassReading, QCompassFilter, {
    reading->setAzimuth(1.0);
    reading->setCalibrationLevel(1.0);
})
PREPARE_SENSORINTERFACE(QGyroscope, QGyroscopeReading, QGyroscopeFilter, {
    reading->setX(1.0);
    reading->setY(1.0);
    reading->setZ(1.0);
})
PREPARE_SENSORINTERFACE(QLightSensor, QLightReading, QLightFilter, {
    reading->setLux(1.0);
})
PREPARE_SENSORINTERFACE(QMagnetometer, QMagnetometerReading, QMagnetometerFilter, {
    reading->setX(1.0);
    reading->setY(1.0);
    reading->setZ(1.0);
    reading->setCalibrationLevel(1.0);
})
PREPARE_SENSORINTERFACE(QOrientationSensor, QOrientationReading, QOrientationFilter, {
    reading->setOrientation(QOrientationReading::LeftUp);
})
PREPARE_SENSORINTERFACE(QPressureSensor, QPressureReading, QPressureFilter, {
    reading->setPressure(1.0);
    reading->setTemperature(1.0);
})
PREPARE_SENSORINTERFACE(QProximitySensor, QProximityReading, QProximityFilter, {
    reading->setClose(true);
})
PREPARE_SENSORINTERFACE(QRotationSensor, QRotationReading, QRotationFilter, {
    reading->setFromEuler(1.0, 1.0, 1.0);
})
PREPARE_SENSORINTERFACE(QTapSensor, QTapReading, QTapFilter, {
    reading->setTapDirection(QTapReading::Z_Both);
    reading->setDoubleTap(true);
})
PREPARE_SENSORINTERFACE(QIRProximitySensor, QIRProximityReading, QIRProximityFilter, {
    reading->setReflectance(0.5);
})
PREPARE_SENSORINTERFACE(QTiltSensor, QTiltReading, QTiltFilter, {
    reading->setYRotation(1.0);
    reading->setXRotation(1.0);
})
PREPARE_SENSORINTERFACE(QLidSensor, QLidReading, QLidFilter, {
    reading->setBackLidClosed(true);
    reading->setFrontLidClosed(true);
})
PREPARE_SENSORINTERFACE(QHumiditySensor, QHumidityReading, QHumidityFilter, {
    reading->setRelativeHumidity(1.0);
    reading->setAbsoluteHumidity(1.0);
})


#define TEST_SENSORINTERFACE(SensorClass, ReadingClass, readingcode)\
    do {\
        SensorClass sensor;\
        sensor.setIdentifier(#SensorClass); \
        SensorClass ## _testfilter filter;\
        sensor.addFilter(&filter);\
        sensor.start();\
        ReadingClass *reading = sensor.reading();\
        readingcode\
    } while (0);

#endif
