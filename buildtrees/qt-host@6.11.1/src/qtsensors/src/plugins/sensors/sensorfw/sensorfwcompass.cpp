// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include "sensorfwcompass.h"

char const * const SensorfwCompass::id("sensorfw.compass");

SensorfwCompass::SensorfwCompass(QSensor *sensor)
    : SensorfwSensorBase(sensor)
    , m_initDone(false)
{
    init();
    setReading<QCompassReading>(&m_reading);
    sensor->setDataRate(50);//set a default rate
}

void SensorfwCompass::slotDataAvailable(const Compass& data)
{
    // The scale for level is [0,3], where 3 is the best
    // Qt: Measured as a value from 0 to 1 with higher values being better.
    m_reading.setCalibrationLevel(((float) data.level()) / 3.0);

    // The scale for degrees from sensord is [0,359]
    // Value can be directly used as azimuth
    m_reading.setAzimuth(data.degrees());

    m_reading.setTimestamp(data.data().timestamp_);
    newReadingAvailable();
}


bool SensorfwCompass::doConnect()
{
    Q_ASSERT(m_sensorInterface);
    return QObject::connect(m_sensorInterface, SIGNAL(dataAvailable(Compass)),
                            this, SLOT(slotDataAvailable(Compass)));
}

QString SensorfwCompass::sensorName() const
{
    return "compasssensor";
}

void SensorfwCompass::init()
{
    m_initDone = false;
    initSensor<CompassSensorChannelInterface>(m_initDone);
}

void SensorfwCompass::start()
{
    if (reinitIsNeeded)
        init();
    SensorfwSensorBase::start();
}
