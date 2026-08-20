// Copyright (C) 2016 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qplugin.h>
#include <qsensorplugin.h>
#include <qsensorbackend.h>
#include <qsensormanager.h>
#include <qaccelerometer.h>
#include <qcompass.h>
#include "androidaccelerometer.h"
#include "androidcompass.h"
#include "androidgyroscope.h"
#include "androidlight.h"
#include "androidmagnetometer.h"
#include "androidpressure.h"
#include "androidproximity.h"
#include "androidrotation.h"
#include "androidtemperature.h"

#include "sensormanager.h"
#include <android/sensor.h>

namespace {
    const char AndroidCompassId[] = "android.synthetic.compass";
}

class AndroidSensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)
public:
    void registerSensors() override
    {
        bool accelerometer = false;
        bool magnetometer = false;
        ASensorList availableSensors;
        int count = ASensorManager_getSensorList(SensorManager::instance()->manager(), &availableSensors);
        for (int i = 0; i < count; i++) {
            int sensor = ASensor_getType(availableSensors[i]);
            switch (sensor) {
            case ASENSOR_TYPE_ACCELEROMETER:
                m_accelerationModes |= AndroidAccelerometer::Accelerometer;
                QSensorManager::registerBackend(QAccelerometer::sensorType, QByteArray::number(sensor), this);
                accelerometer = true;
                break;
            case ASENSOR_TYPE_GRAVITY:
                m_accelerationModes |= AndroidAccelerometer::Gravity;
                break;
            case ASENSOR_TYPE_LINEAR_ACCELERATION:
                m_accelerationModes |= AndroidAccelerometer::LinearAcceleration;
                break;
            case ASENSOR_TYPE_AMBIENT_TEMPERATURE:
                QSensorManager::registerBackend(QAmbientTemperatureSensor::sensorType, QByteArray::number(sensor), this);
                break;
            case ASENSOR_TYPE_GYROSCOPE:
                QSensorManager::registerBackend(QGyroscope::sensorType, QByteArray::number(sensor), this);
                break;
            case ASENSOR_TYPE_LIGHT:
                QSensorManager::registerBackend(QLightSensor::sensorType, QByteArray::number(sensor), this);
                break;
            case ASENSOR_TYPE_MAGNETIC_FIELD:
                QSensorManager::registerBackend(QMagnetometer::sensorType, QByteArray::number(sensor), this);
                magnetometer = true;
                break;
            case ASENSOR_TYPE_PRESSURE:
                QSensorManager::registerBackend(QPressureSensor::sensorType, QByteArray::number(sensor), this);
                break;
            case ASENSOR_TYPE_PROXIMITY:
                QSensorManager::registerBackend(QProximitySensor::sensorType, QByteArray::number(sensor), this);
                break;
            case ASENSOR_TYPE_ROTATION_VECTOR:
                QSensorManager::registerBackend(QRotationSensor::sensorType, QByteArray::number(sensor), this);
                break;

            case ASENSOR_TYPE_RELATIVE_HUMIDITY:
            case ASENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED:
            case ASENSOR_TYPE_GAME_ROTATION_VECTOR:
            case ASENSOR_TYPE_GYROSCOPE_UNCALIBRATED:
            case ASENSOR_TYPE_SIGNIFICANT_MOTION:
            case ASENSOR_TYPE_STEP_DETECTOR:
            case ASENSOR_TYPE_STEP_COUNTER:
            case ASENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
            case ASENSOR_TYPE_HEART_RATE:
            case ASENSOR_TYPE_POSE_6DOF:
            case ASENSOR_TYPE_STATIONARY_DETECT:
            case ASENSOR_TYPE_MOTION_DETECT:
            case ASENSOR_TYPE_HEART_BEAT:
            case ASENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT:
            case ASENSOR_TYPE_ACCELEROMETER_UNCALIBRATED:
                break; // ### TODO add backends for missing Android sensors
            }
        }
        if (accelerometer && magnetometer)
            QSensorManager::registerBackend(QCompass::sensorType, AndroidCompassId, this);
    }

    QSensorBackend *createBackend(QSensor *sensor) override
    {
        if (sensor->identifier() == AndroidCompassId)
            return new AndroidCompass(sensor);

        int type = sensor->identifier().toInt();
        switch (type) {
        case ASENSOR_TYPE_ACCELEROMETER:
            return new AndroidAccelerometer(m_accelerationModes, sensor);
        case ASENSOR_TYPE_AMBIENT_TEMPERATURE:
            return new AndroidTemperature(type, sensor);
        case ASENSOR_TYPE_GYROSCOPE:
            return new AndroidGyroscope(type, sensor);
        case ASENSOR_TYPE_LIGHT:
            return new AndroidLight(type, sensor);
        case ASENSOR_TYPE_MAGNETIC_FIELD:
            return new AndroidMagnetometer(type, sensor);
        case ASENSOR_TYPE_PRESSURE:
            return new AndroidPressure(type, sensor);
        case ASENSOR_TYPE_PROXIMITY:
            return new AndroidProximity(type, sensor);
        case ASENSOR_TYPE_ROTATION_VECTOR:
            return new AndroidRotation(type, sensor);
        }
        return nullptr;
    }
private:
    int m_accelerationModes = 0;
};

Q_IMPORT_PLUGIN (AndroidSensorPlugin) // automatically register the plugin

#include "main.moc"

