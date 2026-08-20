// Copyright (C) 2016 Canonical, Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "sensorfwlidsensor.h"

char const * const SensorfwLidSensor::id("sensorfw.lidsensor");

SensorfwLidSensor::SensorfwLidSensor(QSensor *sensor)
    : SensorfwSensorBase(sensor)
    , m_initDone(false)
{
    init();
    setReading<QLidReading>(&m_reading);
    sensor->setDataRate(10);//set a default rate
}

void SensorfwLidSensor::slotDataAvailable(const LidData& data)
{
    switch (data.type_) {
    case data.BackLid:
        m_reading.setBackLidClosed(data.value_);
        break;
    case data.FrontLid:
        m_reading.setFrontLidClosed(data.value_);
        break;
    };

    m_reading.setTimestamp(data.timestamp_);
    newReadingAvailable();
}

bool SensorfwLidSensor::doConnect()
{
    Q_ASSERT(m_sensorInterface);
    return QObject::connect(m_sensorInterface, SIGNAL(lidChanged(LidData)),
                            this, SLOT(slotDataAvailable(LidData)));
}

QString SensorfwLidSensor::sensorName() const
{
    return "lidsensor";
}

void SensorfwLidSensor::init()
{
    m_initDone = false;
    initSensor<LidSensorChannelInterface>(m_initDone);
}

void SensorfwLidSensor::start()
{
    if (reinitIsNeeded)
        init();
    SensorfwSensorBase::start();
}
