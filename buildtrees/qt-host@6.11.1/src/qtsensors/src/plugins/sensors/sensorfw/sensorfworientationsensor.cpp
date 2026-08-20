// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "sensorfworientationsensor.h"

#include <datatypes/posedata.h>

char const * const SensorfwOrientationSensor::id("sensorfw.orientationsensor");

SensorfwOrientationSensor::SensorfwOrientationSensor(QSensor *sensor)
    : SensorfwSensorBase(sensor)
    , m_initDone(false)
{
    init();
    setReading<QOrientationReading>(&m_reading);
    sensor->setDataRate(10);//set a default rate
}


void SensorfwOrientationSensor::start()
{
    if (reinitIsNeeded)
        init();
    if (m_sensorInterface) {
        Unsigned data(((OrientationSensorChannelInterface*)m_sensorInterface)->orientation());
        m_reading.setOrientation(SensorfwOrientationSensor::getOrientation(data.x()));
        m_reading.setTimestamp(data.UnsignedData().timestamp_);
        newReadingAvailable();
    }
    SensorfwSensorBase::start();
}


void SensorfwOrientationSensor::slotDataAvailable(const Unsigned& data)
{
    m_reading.setOrientation(SensorfwOrientationSensor::getOrientation(data.x()));
    m_reading.setTimestamp(data.UnsignedData().timestamp_);
    newReadingAvailable();
}

bool SensorfwOrientationSensor::doConnect()
{
    Q_ASSERT(m_sensorInterface);
    return QObject::connect(m_sensorInterface, SIGNAL(orientationChanged(Unsigned)),
                            this, SLOT(slotDataAvailable(Unsigned)));
}

QString SensorfwOrientationSensor::sensorName() const
{
    return "orientationsensor";
}

QOrientationReading::Orientation SensorfwOrientationSensor::getOrientation(int orientation)
{
    switch (orientation) {
    case PoseData::BottomDown: return QOrientationReading::TopUp;
    case PoseData::BottomUp:   return QOrientationReading::TopDown;
    case PoseData::LeftUp:     return QOrientationReading::LeftUp;
    case PoseData::RightUp:    return QOrientationReading::RightUp;
    case PoseData::FaceUp:     return QOrientationReading::FaceUp;
    case PoseData::FaceDown:   return QOrientationReading::FaceDown;
    }
    return QOrientationReading::Undefined;
}

void SensorfwOrientationSensor::init()
{
    m_initDone = false;
    initSensor<OrientationSensorChannelInterface>(m_initDone);
}
