// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "dummylightsensor.h"
#include <QDebug>
#include <QRandomGenerator>

char const * const dummylightsensor::id("dummy.lightsensor");

dummylightsensor::dummylightsensor(QSensor *sensor)
    : dummycommon(sensor)
{
    setReading<QAmbientLightReading>(&m_reading);
    addDataRate(100,100);
}

void dummylightsensor::poll()
{
    m_reading.setTimestamp(getTimestamp());
    if (QRandomGenerator::global()->bounded(100) == 0)
        m_reading.setLightLevel(QAmbientLightReading::Dark);
    else
        m_reading.setLightLevel(QAmbientLightReading::Light);

    newReadingAvailable();
}

