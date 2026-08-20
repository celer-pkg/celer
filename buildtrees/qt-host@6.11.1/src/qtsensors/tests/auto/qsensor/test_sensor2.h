// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_SENSOR2_H
#define TEST_SENSOR2_H

#include "qsensor.h"

class TestSensor2ReadingPrivate;

class TestSensor2Reading : public QSensorReading
{
    Q_OBJECT
    Q_PROPERTY(int test READ test)
    DECLARE_READING(TestSensor2Reading)
public:
    int test() const;
    void setTest(int test);
};

class TestSensor2Filter : public QSensorFilter
{
public:
    virtual bool filter(TestSensor2Reading *reading) = 0;
private:
    bool filter(QSensorReading *reading) override { return filter(static_cast<TestSensor2Reading*>(reading)); }
};

class TestSensor2 : public QSensor
{
    Q_OBJECT
public:
    explicit TestSensor2(QObject *parent = 0) : QSensor(TestSensor2::sensorType, parent) {}
    virtual ~TestSensor2() {}
    TestSensor2Reading *reading() const { return static_cast<TestSensor2Reading*>(QSensor::reading()); }
    static char const * const sensorType;
};

#endif
