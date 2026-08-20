// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SENSORFWMAGNETOMETER_H
#define SENSORFWMAGNETOMETER_H

#include "sensorfwsensorbase.h"
#include <QtSensors/qmagnetometer.h>

#include <magnetometersensor_i.h>
#include <datatypes/magneticfield.h>



class SensorfwMagnetometer : public SensorfwSensorBase
{
    Q_OBJECT

public:
    static char const * const id;
    SensorfwMagnetometer(QSensor *sensor);
protected:
    bool doConnect() override;
    void start() override;
    QString sensorName() const override;
    qreal correctionFactor() const override;
    virtual void init();

private:
    static const float NANO;
    QMagnetometerReading m_reading;
    bool m_initDone;
    bool m_isGeoMagnetometer;

private slots:
    void slotDataAvailable(const MagneticField &data);
    void slotFrameAvailable(const QList<MagneticField> &);
};

#endif
