// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "sensorfwirproximitysensor.h"
#define RM680_PS "/dev/bh1770glc_ps"

char const * const SensorfwIrProximitySensor::id("sensorfw.irproximitysensor");

SensorfwIrProximitySensor::SensorfwIrProximitySensor(QSensor *sensor)
    : SensorfwSensorBase(sensor)
    , m_initDone(false)
{
    init();
    setReading<QIRProximityReading>(&m_reading);
    setDescription(QLatin1String("reflectance as percentage (%) of maximum"));
    addOutputRange(0, 100, 1);
    addDataRate(10,10);
    rangeMax = QFile::exists(RM680_PS)?255:1023;
    sensor->setDataRate(10);//set a default rate
}

void SensorfwIrProximitySensor::slotDataAvailable(const Proximity& proximity)
{
    m_reading.setReflectance((float)proximity.reflectance()*100 / rangeMax);
    m_reading.setTimestamp(proximity.UnsignedData().timestamp_);
    newReadingAvailable();
}


bool SensorfwIrProximitySensor::doConnect()
{
    Q_ASSERT(m_sensorInterface);
    return QObject::connect(m_sensorInterface, SIGNAL(reflectanceDataAvailable(Proximity)),
                            this, SLOT(slotDataAvailable(Proximity)));
}


QString SensorfwIrProximitySensor::sensorName() const
{
    return "proximitysensor";
}


void SensorfwIrProximitySensor::init()
{
    m_initDone = false;
    initSensor<ProximitySensorChannelInterface>(m_initDone);
}

void SensorfwIrProximitySensor::start()
{
    if (reinitIsNeeded)
        init();
    SensorfwSensorBase::start();
}
