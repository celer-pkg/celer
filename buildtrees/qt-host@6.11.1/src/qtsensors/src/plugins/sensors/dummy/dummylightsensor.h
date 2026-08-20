// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DUMMYLIGHTSENSOR_H
#define DUMMYLIGHTSENSOR_H

#include "dummycommon.h"
#include <qambientlightsensor.h>

class dummylightsensor : public dummycommon
{
public:
    static char const * const id;

    dummylightsensor(QSensor *sensor);

    void poll() override;
private:
    QAmbientLightReading m_reading;
};

#endif

