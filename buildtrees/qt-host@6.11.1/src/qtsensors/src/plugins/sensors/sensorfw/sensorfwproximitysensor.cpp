// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "sensorfwproximitysensor.h"

char const * const SensorfwProximitySensor::id("sensorfw.proximitysensor");

SensorfwProximitySensor::SensorfwProximitySensor(QSensor *sensor)
    : SensorfwSensorBase(sensor),
      m_initDone(false),
      m_exClose(false),
      firstRun(true)
{
    init();
    setReading<QProximityReading>(&m_reading);
    addDataRate(10,10); //TODO: fix this when we know better
    sensor->setDataRate(10);//set a default rate
}

void SensorfwProximitySensor::start()
{
    if (reinitIsNeeded)
        init();
    SensorfwSensorBase::start();
    if (m_sensorInterface) {
        Unsigned data(((ProximitySensorChannelInterface*)m_sensorInterface)->proximity());
        // Note: Unlike reflectanceDataAvailable() signal, the query
        //       above returns only integer reflectance without the
        //       boolean withinProximity value.
        bool close = (data.x() == 0);
        m_exClose = close;
        m_reading.setClose(close);
        m_reading.setTimestamp(data.UnsignedData().timestamp_);
        m_exClose = (int)m_reading.close();
        newReadingAvailable();
    }
}

void SensorfwProximitySensor::slotReflectanceDataAvailable(const Proximity& data)
{
    bool close = data.x() ? true : false;
    if (!firstRun && close == m_exClose)
        return;
    m_reading.setClose(close);
    m_reading.setTimestamp(data.UnsignedData().timestamp_);
    newReadingAvailable();
    m_exClose = close;
    if (firstRun)
        firstRun = false;
}

bool SensorfwProximitySensor::doConnect()
{
    Q_ASSERT(qobject_cast<ProximitySensorChannelInterface*>(m_sensorInterface));
    return QObject::connect(qobject_cast<ProximitySensorChannelInterface*>(m_sensorInterface),
                            &ProximitySensorChannelInterface::reflectanceDataAvailable,
                            this, &SensorfwProximitySensor::slotReflectanceDataAvailable);
}


QString SensorfwProximitySensor::sensorName() const
{
    return "proximitysensor";
}

void SensorfwProximitySensor::init()
{
    m_initDone = false;
    initSensor<ProximitySensorChannelInterface>(m_initDone);
}
