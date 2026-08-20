// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef SENSORFWSENSORBASE_H
#define SENSORFWSENSORBASE_H

#include <QtSensors/qsensorbackend.h>
#include <sensormanagerinterface.h>
#include <abstractsensor_i.h>

#include <QtSensors/QAmbientLightSensor>
#include <QtSensors/QIRProximitySensor>
#include <QtSensors/QTapSensor>
#include <QtSensors/QProximitySensor>

class SensorfwSensorBase : public QSensorBackend
{
    Q_OBJECT
public:
    SensorfwSensorBase(QSensor *sensor);
    virtual ~SensorfwSensorBase();


protected:
    virtual bool doConnect()=0;
    void start() override;
    void stop() override;

    static const float GRAVITY_EARTH;
    static const float GRAVITY_EARTH_THOUSANDTH;    //for speed
    static const int KErrNotFound;
    static const int KErrInUse;
    static QStringList m_bufferingSensors;

    void setRanges(qreal correctionFactor=1);
    virtual QString sensorName() const=0;

    template<typename T>
    void initSensor(bool &initDone)
    {
        const QString name = sensorName();

        if (!initDone) {
            if (!m_remoteSensorManager) {
                qDebug() << "There is no sensor manager yet, do not initialize" << name;
                return;
            }
            if (!m_remoteSensorManager->loadPlugin(name)) {
                sensorError(KErrNotFound);
                return;
            }
            m_remoteSensorManager->registerSensorInterface<T>(name);
        }
        m_sensorInterface = T::controlInterface(name);
        if (!m_sensorInterface) {
            m_sensorInterface = const_cast<T*>(T::listenInterface(name));
        }
        initDone = initSensorInterface(name);
    };


    AbstractSensorChannelInterface* m_sensorInterface;
    int m_bufferSize;
    int bufferSize() const;
    virtual qreal correctionFactor() const;
    bool reinitIsNeeded;
    bool isFeatureSupported(QSensor::Feature feature) const override;

private:
    bool initSensorInterface(QString const &);
    static SensorManagerInterface* m_remoteSensorManager;
    int m_prevOutputRange;
    bool doConnectAfterCheck();
    int m_efficientBufferSize, m_maxBufferSize;

    QDBusServiceWatcher *watcher;
    bool m_available;
    bool running;
    bool m_attemptRestart;
private slots:
    void connectToSensord();
    void sensordUnregistered();
    void standyOverrideChanged();
};

#endif
