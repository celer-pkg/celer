// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "sensorfwlightsensor.h"

char const * const SensorfwLightSensor::id("sensorfw.lightsensor");

SensorfwLightSensor::SensorfwLightSensor(QSensor *sensor)
    : SensorfwSensorBase(sensor)
    , m_initDone(false)
{
    init();
    setReading<QLightReading>(&m_reading);
    sensor->setDataRate(10);//set a default rate
}

void SensorfwLightSensor::slotDataAvailable(const Unsigned& data)
{
    m_reading.setLux(data.UnsignedData().value_);
    m_reading.setTimestamp(data.UnsignedData().timestamp_);
    newReadingAvailable();
}

bool SensorfwLightSensor::doConnect()
{
    Q_ASSERT(m_sensorInterface);
    return QObject::connect(m_sensorInterface, SIGNAL(ALSChanged(Unsigned)),
                            this, SLOT(slotDataAvailable(Unsigned)));
}


QString SensorfwLightSensor::sensorName() const
{
    return "alssensor";
}
void SensorfwLightSensor::init()
{
    m_initDone = false;
    initSensor<ALSSensorChannelInterface>(m_initDone);
}

void SensorfwLightSensor::start()
{
    if (reinitIsNeeded)
        init();
    SensorfwSensorBase::start();
}
