// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mybackend.h"
#include <qsensorplugin.h>
#include <qsensormanager.h>

const char *MyBackend::id = "mybackend";

//! [Plugin]
class MyPluginClass : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    //Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)
public:
    void registerSensors() override
    {
        QSensorManager::registerBackend(QAccelerometer::sensorType, MyBackend::id, this);
    }

    QSensorBackend *createBackend(QSensor *sensor) override
    {
        if (sensor->identifier() == MyBackend::id)
            return new MyBackend(sensor);
        return 0;
    }
};
//! [Plugin]

#include "plugin.moc"
