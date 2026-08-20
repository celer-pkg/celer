// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLSENSOR_P_H
#define QMLSENSOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qsensorsquickglobal_p.h"

#include <QObject>
#include <QProperty>
#include <QQmlParserStatus>
#include <QtQml/qqml.h>
#include <QQmlListProperty>
#include <QtSensors/QSensor>

#include "qmlsensorrange_p.h"

QT_BEGIN_NAMESPACE

class QSensor;
class QSensorReading;

class QmlSensorReading;

class QmlSensorPrivate;
class Q_SENSORSQUICK_EXPORT QmlSensor : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QmlSensor)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QByteArray identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QByteArray type READ type CONSTANT)
    Q_PROPERTY(bool connectedToBackend READ isConnectedToBackend NOTIFY connectedToBackendChanged)
    Q_PROPERTY(QQmlListProperty<QmlSensorRange> availableDataRates READ availableDataRates NOTIFY availableDataRatesChanged)
    Q_PROPERTY(int dataRate READ dataRate WRITE setDataRate NOTIFY dataRateChanged)
    Q_PROPERTY(QmlSensorReading* reading READ reading NOTIFY readingChanged BINDABLE bindableReading)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QQmlListProperty<QmlSensorOutputRange> outputRanges READ outputRanges NOTIFY outputRangesChanged)
    Q_PROPERTY(int outputRange READ outputRange WRITE setOutputRange NOTIFY outputRangeChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(int error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool alwaysOn READ isAlwaysOn WRITE setAlwaysOn NOTIFY alwaysOnChanged)
    Q_PROPERTY(bool skipDuplicates READ skipDuplicates WRITE setSkipDuplicates NOTIFY skipDuplicatesChanged REVISION 1)
    Q_PROPERTY(AxesOrientationMode axesOrientationMode READ axesOrientationMode WRITE setAxesOrientationMode NOTIFY axesOrientationModeChanged REVISION 1)
    Q_PROPERTY(int currentOrientation READ currentOrientation NOTIFY currentOrientationChanged REVISION 1)
    Q_PROPERTY(int userOrientation READ userOrientation WRITE setUserOrientation NOTIFY userOrientationChanged REVISION 1)
    Q_PROPERTY(int maxBufferSize READ maxBufferSize NOTIFY maxBufferSizeChanged REVISION 1)
    Q_PROPERTY(int efficientBufferSize READ efficientBufferSize NOTIFY efficientBufferSizeChanged REVISION 1)
    Q_PROPERTY(int bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged REVISION 1)

    QML_NAMED_ELEMENT(Sensor)
    QML_UNCREATABLE("Cannot create Sensor")
    QML_ADDED_IN_VERSION(5,0)
public:
    // Keep in sync with QSensor::Feature
    enum Feature : int {
        Buffering = QSensor::Buffering,
        AlwaysOn = QSensor::AlwaysOn,
        GeoValues = QSensor::GeoValues,
        FieldOfView = QSensor::FieldOfView,
        AccelerationMode = QSensor::AccelerationMode,
        SkipDuplicates = QSensor::SkipDuplicates,
        AxesOrientation = QSensor::AxesOrientation,
        PressureSensorTemperature = QSensor::PressureSensorTemperature
    };
    Q_ENUM(Feature)

    // Keep in sync with QSensor::AxesOrientationMode
    enum AxesOrientationMode {
        FixedOrientation,
        AutomaticOrientation,
        UserOrientation
    };
    Q_ENUM(AxesOrientationMode)

    explicit QmlSensor(QObject *parent = 0);
    ~QmlSensor();

    QByteArray identifier() const;
    void setIdentifier(const QByteArray &identifier);

    QByteArray type() const;

    bool isConnectedToBackend() const;

    bool isBusy() const;

    void setActive(bool active);
    bool isActive() const;

    bool isAlwaysOn() const;
    void setAlwaysOn(bool alwaysOn);

    bool skipDuplicates() const;
    void setSkipDuplicates(bool skipDuplicates);

    QQmlListProperty<QmlSensorRange> availableDataRates() const;
    int dataRate() const;
    void setDataRate(int rate);

    QQmlListProperty<QmlSensorOutputRange> outputRanges() const;
    int outputRange() const;
    void setOutputRange(int index);

    QString description() const;
    int error() const;

    QmlSensorReading *reading() const;
    QBindable<QmlSensorReading*> bindableReading() const;

    Q_INVOKABLE Q_REVISION(6, 7) bool isFeatureSupported(Feature feature) const;

    AxesOrientationMode axesOrientationMode() const;
    void setAxesOrientationMode(AxesOrientationMode axesOrientationMode);

    int currentOrientation() const;

    int userOrientation() const;
    void setUserOrientation(int userOrientation);

    int maxBufferSize() const;

    int efficientBufferSize() const;

    int bufferSize() const;
    void setBufferSize(int bufferSize);

    virtual QSensor *sensor() const = 0;

    void componentComplete() override;

public Q_SLOTS:
    bool start();
    void stop();

Q_SIGNALS:
    void identifierChanged();
    void connectedToBackendChanged();
    void availableDataRatesChanged();
    void dataRateChanged();
    void readingChanged();
    void activeChanged();
    void outputRangesChanged();
    void outputRangeChanged();
    void descriptionChanged();
    void errorChanged();
    void alwaysOnChanged();
    void busyChanged();
    Q_REVISION(1) void skipDuplicatesChanged(bool skipDuplicates);
    Q_REVISION(1) void axesOrientationModeChanged(AxesOrientationMode axesOrientationMode);
    Q_REVISION(1) void currentOrientationChanged(int currentOrientation);
    Q_REVISION(1) void userOrientationChanged(int userOrientation);
    Q_REVISION(1) void maxBufferSizeChanged(int maxBufferSize);
    Q_REVISION(1) void efficientBufferSizeChanged(int efficientBufferSize);
    Q_REVISION(1) void bufferSizeChanged(int bufferSize);

protected:
    virtual QmlSensorReading *createReading() const = 0;

private Q_SLOTS:
    void updateReading();

private:
    void classBegin() override;
    bool m_componentComplete = false;
    bool m_activateOnComplete = false;
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QmlSensor, QmlSensorReading*,
                                         m_reading, nullptr)
};

class Q_SENSORSQUICK_EXPORT QmlSensorReading : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint64 timestamp READ timestamp NOTIFY timestampChanged BINDABLE bindableTimestamp)
    QML_NAMED_ELEMENT(SensorReading)
    QML_UNCREATABLE("Cannot create SensorReading")
    QML_ADDED_IN_VERSION(5,0)
public:
    explicit QmlSensorReading() = default;
    ~QmlSensorReading() = default;

    quint64 timestamp() const;
    QBindable<quint64> bindableTimestamp() const;

    void update();

Q_SIGNALS:
    void timestampChanged();

private:
    virtual QSensorReading *reading() const = 0;
    virtual void readingUpdate() = 0;
    Q_OBJECT_BINDABLE_PROPERTY(QmlSensorReading, quint64,
                               m_timestamp, &QmlSensorReading::timestampChanged)
};

QT_END_NAMESPACE

#endif
