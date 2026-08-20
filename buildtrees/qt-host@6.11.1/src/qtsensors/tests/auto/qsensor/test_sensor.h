// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_SENSOR_H
#define TEST_SENSOR_H

#include <qsensor.h>

class TestSensorReadingPrivate;

class TestSensorReading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(int test READ test)
    DECLARE_READING(TestSensorReading)
public:
    int test() const;
    void setTest(int test);
};

class TestSensorFilter : public QSensorFilter
{
public:
    virtual bool filter(TestSensorReading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override { return filter(static_cast<TestSensorReading*>(reading)); }
};

class TestSensor : public QSensor
{
    Q_OBJECT
public:
    explicit TestSensor(QObject *parent = 0)
        : QSensor(TestSensor::sensorType, parent)
        , sensorsChangedEmitted(0)
    {
        connect(this, SIGNAL(availableSensorsChanged()), this, SLOT(s_availableSensorsChanged()));
    }
    virtual ~TestSensor() {}
    TestSensorReading *reading() const { return static_cast<TestSensorReading*>(QSensor::reading()); }
    static const char *sensorType;

    // used by the testSensorsChangedSignal test function
    int sensorsChangedEmitted;
private slots:
    void s_availableSensorsChanged()
    {
        sensorsChangedEmitted++;
    }
};

#endif
