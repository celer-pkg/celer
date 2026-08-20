// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtCore/QDebug>

#include <QtTest/private/qpropertytesthelper_p.h>
#include <QtSensorsQuick/private/qmlsensor_p.h>
#include <qsensorbackend.h>
#include "qsensormanager.h"

#include "../../common/test_backends.h"
#include <QtSensorsQuick/private/qmlaccelerometer_p.h>
#include <QtSensorsQuick/private/qmlpressuresensor_p.h>
#include <QtSensorsQuick/private/qmlgyroscope_p.h>
#include <QtSensorsQuick/private/qmltapsensor_p.h>
#include <QtSensorsQuick/private/qmlcompass_p.h>
#include <QtSensorsQuick/private/qmlproximitysensor_p.h>
#include <QtSensorsQuick/private/qmlorientationsensor_p.h>
#include <QtSensorsQuick/private/qmlambientlightsensor_p.h>
#include <QtSensorsQuick/private/qmlmagnetometer_p.h>
#include <QtSensorsQuick/private/qmllidsensor_p.h>
#include <QtSensorsQuick/private/qmltiltsensor_p.h>
#include <QtSensorsQuick/private/qmlrotationsensor_p.h>
#include <QtSensorsQuick/private/qmlhumiditysensor_p.h>
#include <QtSensorsQuick/private/qmlambienttemperaturesensor_p.h>
#include <QtSensorsQuick/private/qmllightsensor_p.h>
#include <QtSensorsQuick/private/qmlirproximitysensor_p.h>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

class tst_sensors_qmlcpp : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testReadingBindings();
    // void testGesture();
    void testSensorRanges();
};

void tst_sensors_qmlcpp::initTestCase()
{
    qputenv("QT_SENSORS_LOAD_PLUGINS", "0"); // Do not load plugins
}

template<typename SensorClass, typename ReadingClass, typename ValueType>
void testSensorReadings(const char* identifier, const QVariantMap& values)
{
    SensorClass sensor;
    sensor.setIdentifier(identifier);
    sensor.componentComplete();
    sensor.start();

    for (const auto& key : values.keys()) {
        ValueType initialValue = values[key].toList()[0].value<ValueType>();
        ValueType changedValue = values[key].toList()[1].value<ValueType>();
        QTestPrivate::testReadOnlyPropertyBasics<ReadingClass, ValueType>(
                    *static_cast<ReadingClass*>(sensor.reading()),
                    initialValue, changedValue, key.toStdString().c_str(),
                    [&](){ set_test_backend_reading(sensor.sensor(), {{key, changedValue}}); });
        if (QTest::currentTestFailed()) {
            qWarning() << identifier << "::" << key << "test failed.";
            return;
        }
    }
}

void tst_sensors_qmlcpp::testReadingBindings()
{
    register_test_backends();

    testSensorReadings<QmlAccelerometer, QmlAccelerometerReading, qreal>(
                "QAccelerometer",
                {{"x", QVariantList{1.0, 2.0}},
                 {"y", QVariantList{1.0, 2.0}},
                 {"z", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlAccelerometer, QmlAccelerometerReading, quint64>(
                "QAccelerometer",
                {{"timestamp", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlAmbientLightSensor, QmlAmbientLightSensorReading, QAmbientLightReading::LightLevel>(
                "QAmbientLightSensor",
                {{"lightLevel", QVariantList{QAmbientLightReading::Twilight, QAmbientLightReading::Sunny}}});
    testSensorReadings<QmlPressureSensor, QmlPressureReading, qreal>(
                "QPressureSensor",
                {{"pressure", QVariantList{1.0, 2.0}},
                 {"temperature", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlGyroscope, QmlGyroscopeReading, qreal>(
                "QGyroscope",
                {{"x", QVariantList{1.0, 2.0}},
                 {"y", QVariantList{1.0, 2.0}},
                 {"z", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlTapSensor, QmlTapSensorReading, bool>(
                "QTapSensor",
                {{"doubleTap", QVariantList{true, false}}});
    testSensorReadings<QmlTapSensor, QmlTapSensorReading, QTapReading::TapDirection>(
                "QTapSensor",
                {{"tapDirection", QVariantList{QTapReading::Z_Both, QTapReading::X_Both}}});
    testSensorReadings<QmlCompass, QmlCompassReading, qreal>(
                "QCompass",
                {{"azimuth", QVariantList{1.0, 2.0}},
                 {"calibrationLevel", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlProximitySensor, QmlProximitySensorReading, bool>(
                "QProximitySensor",
                {{"near", QVariantList{true, false}}});
    testSensorReadings<QmlOrientationSensor, QmlOrientationSensorReading, QOrientationReading::Orientation>(
                "QOrientationSensor",
                {{"orientation", QVariantList{QOrientationReading::LeftUp, QOrientationReading::RightUp}}});
    testSensorReadings<QmlMagnetometer, QmlMagnetometerReading, qreal>(
                "QMagnetometer",
                {{"x", QVariantList{1.0, 2.0}},
                 {"y", QVariantList{1.0, 2.0}},
                 {"z", QVariantList{1.0, 2.0}},
                 {"calibrationLevel", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlLidSensor, QmlLidReading, bool>(
                "QLidSensor",
                {{"backLidClosed", QVariantList{true, false}},
                 {"frontLidClosed", QVariantList{true, false}}});
    testSensorReadings<QmlTiltSensor, QmlTiltSensorReading, qreal>(
                "QTiltSensor",
                {{"yRotation", QVariantList{1.0, 2.0}},
                 {"xRotation", QVariantList{1.0, 2.0}}});
    // rotation sensor properties need to be tested separately because the setter function is
    // not symmetric with getter functions ("setFromEuler()" vs. "x() & y() & z()")
    testSensorReadings<QmlRotationSensor, QmlRotationSensorReading, qreal>(
                "QRotationSensor",
                {{"x", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlRotationSensor, QmlRotationSensorReading, qreal>(
                "QRotationSensor",
                {{"y", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlRotationSensor, QmlRotationSensorReading, qreal>(
                "QRotationSensor",
                {{"z", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlHumiditySensor, QmlHumidityReading, qreal>(
                "QHumiditySensor",
                {{"relativeHumidity", QVariantList{1.0, 2.0}},
                 {"absoluteHumidity", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlAmbientTemperatureSensor, QmlAmbientTemperatureReading, qreal>(
                "QAmbientTemperatureSensor",
                {{"temperature", QVariantList{30.0, 40.0}}});
    testSensorReadings<QmlLightSensor, QmlLightSensorReading, qreal>(
                "QLightSensor",
                {{"illuminance", QVariantList{1.0, 2.0}}});
    testSensorReadings<QmlIRProximitySensor, QmlIRProximitySensorReading, qreal>(
                "QIRProximitySensor",
                {{"reflectance", QVariantList{0.5, 0.6}}});

    // The following tests QmlSensor (the baseclass) 'readingChanged' which is
    // emitted every time a sensor value changes. For that we instantiate a
    // concrete sensor. The actual 'reading' value (a QObject pointer) of the
    // 'readingChanged' property will not change, but rather the
    // 'readingChanged' is used to indicate that the value it contains has changed.
    QmlAccelerometer accelerometer;
    accelerometer.setIdentifier("QAccelerometer");
    accelerometer.componentComplete();
    accelerometer.start();
    QTestPrivate::testReadOnlyPropertyBasics<QmlSensor, QmlSensorReading*>(
                accelerometer, accelerometer.reading(), accelerometer.reading(), "reading",
                [&](){ set_test_backend_reading(accelerometer.sensor(), {{"x", 2.0}}); });

    unregister_test_backends();
}

class QDummySensorBackend : public QSensorBackend
{
    Q_OBJECT
public:
    QDummySensorBackend(QSensor *sensor) : QSensorBackend(sensor)
    {
        addDataRate(2, 3);
        addDataRate(5, 7);
        addOutputRange(100, 200, 1);
        addOutputRange(600, 700, 10);
        addOutputRange(0, 1, 2);
    }

    void start() override {}
    void stop() override {}
};

class QDummySensorReading : public QSensorReading
{
    Q_OBJECT
public:
    QDummySensorReading(QObject *parent) : QSensorReading(parent, nullptr) {}
};

class QmlDummySensorReading : public QmlSensorReading
{
    Q_OBJECT
public:
    QmlDummySensorReading() :
        m_reading(new QDummySensorReading(this))
    {}

    QSensorReading *reading() const override { return m_reading; }
    void readingUpdate() override {}

private:
    QSensorReading *m_reading = nullptr;
};

class QmlDummySensor : public QmlSensor
{
    Q_OBJECT
public:
    QmlDummySensor(QObject *parent = nullptr) :
        QmlSensor(parent),
        m_sensor(new QSensor("dummy", this))
    {
        QDummySensorBackend b(m_sensor);
        Q_UNUSED(b);
    }

    QSensor *sensor() const override { return m_sensor; }
    QmlSensorReading *createReading() const override { return new QmlDummySensorReading(); }

    void componentComplete() override { QmlSensor::componentComplete(); }

private:
    QSensor *m_sensor = nullptr;
};

void tst_sensors_qmlcpp::testSensorRanges()
{
    QScopedPointer<QmlDummySensor> qmlSensor(new QmlDummySensor);
    qmlSensor->componentComplete();

    auto ranges = qmlSensor->availableDataRates();
    QCOMPARE(ranges.count(&ranges), 2);

    const auto range0 = ranges.at(&ranges, 0);
    QCOMPARE(range0->minimum(), 2);
    QCOMPARE(range0->maximum(), 3);
    QSignalSpy range0Spy(range0, SIGNAL(destroyed()));

    const auto range1 = ranges.at(&ranges, 1);
    QCOMPARE(range1->minimum(), 5);
    QCOMPARE(range1->maximum(), 7);
    QSignalSpy range1Spy(range1, SIGNAL(destroyed()));

    auto outputs = qmlSensor->outputRanges();
    QCOMPARE(outputs.count(&outputs), 3);

    const auto output0 = outputs.at(&outputs, 0);
    QCOMPARE(output0->minimum(), 100);
    QCOMPARE(output0->maximum(), 200);
    QCOMPARE(output0->accuracy(), 1);
    QSignalSpy output0Spy(output0, SIGNAL(destroyed()));

    const auto output1 = outputs.at(&outputs, 1);
    QCOMPARE(output1->minimum(), 600);
    QCOMPARE(output1->maximum(), 700);
    QCOMPARE(output1->accuracy(), 10);
    QSignalSpy output1Spy(output1, SIGNAL(destroyed()));

    const auto output2 = outputs.at(&outputs, 2);
    QCOMPARE(output2->minimum(), 0);
    QCOMPARE(output2->maximum(), 1);
    QCOMPARE(output2->accuracy(), 2);
    QSignalSpy output2Spy(output2, SIGNAL(destroyed()));

    qmlSensor.reset();
    QCOMPARE(range0Spy.size(), 1);
    QCOMPARE(range1Spy.size(), 1);
    QCOMPARE(output0Spy.size(), 1);
    QCOMPARE(output1Spy.size(), 1);
    QCOMPARE(output2Spy.size(), 1);
}

QT_END_NAMESPACE

QTEST_MAIN(tst_sensors_qmlcpp)
#include "tst_sensors_qmlcpp.moc"
