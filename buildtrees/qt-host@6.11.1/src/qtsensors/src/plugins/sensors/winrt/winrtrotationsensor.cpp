// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "winrtrotationsensor.h"
#include "winrtcommon.h"

#include <QtSensors/QRotationSensor>

#include <functional>
#include <wrl.h>
#include <windows.devices.sensors.h>
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Devices::Sensors;

typedef ITypedEventHandler<Inclinometer *, InclinometerReadingChangedEventArgs *> InclinometerReadingHandler;

QT_BEGIN_NAMESPACE

class WinRtRotationSensorPrivate
{
public:
    WinRtRotationSensorPrivate(WinRtRotationSensor *p)
        : minimumReportInterval(0), q(p)
    {
        token.value = 0;
    }

    QRotationReading reading;

    ComPtr<IInclinometer> sensor;
    EventRegistrationToken token;
    quint32 minimumReportInterval;

    HRESULT readingChanged(IInclinometer *, IInclinometerReadingChangedEventArgs *args)
    {
        ComPtr<IInclinometerReading> value;
        HRESULT hr = args->get_Reading(&value);
        if (FAILED(hr)) {
            qCWarning(lcWinRtSensors) << "Failed to get rotation reading" << qt_error_string(hr);
            return hr;
        }

        FLOAT x;
        hr = value->get_PitchDegrees(&x);
        if (FAILED(hr))
            qCWarning(lcWinRtSensors) << "Failed to get rotation X" << qt_error_string(hr);

        FLOAT y;
        hr = value->get_RollDegrees(&y);
        if (FAILED(hr))
            qCWarning(lcWinRtSensors) << "Failed to get rotation Y" << qt_error_string(hr);

        FLOAT z;
        hr = value->get_YawDegrees(&z);
        if (FAILED(hr))
            qCWarning(lcWinRtSensors) << "Failed to get rotation Z" << qt_error_string(hr);

        DateTime dateTime;
        hr = value->get_Timestamp(&dateTime);
        if (FAILED(hr))
            qCWarning(lcWinRtSensors) << "Failed to get rotation timestamp" << qt_error_string(hr);

        reading.setFromEuler(x, y, z);
        reading.setTimestamp(dateTimeToMsSinceEpoch(dateTime));
        q->newReadingAvailable();

        return S_OK;
    }

private:
    WinRtRotationSensor *q;
};

WinRtRotationSensor::WinRtRotationSensor(QSensor *sensor)
    : QSensorBackend(sensor), d_ptr(new WinRtRotationSensorPrivate(this))
{
    Q_D(WinRtRotationSensor);

    HStringReference classId(RuntimeClass_Windows_Devices_Sensors_Inclinometer);
    ComPtr<IInclinometerStatics> factory;
    HRESULT hr = RoGetActivationFactory(classId.Get(), IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to initialize rotation sensor factory."
                                      << qt_error_string(hr);
        sensorError(hr);
        return;
    }
    hr = factory->GetDefault(&d->sensor);
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to get default rotation sensor."
                                      << qt_error_string(hr);
        sensorError(hr);
        return;
    }

    if (!d->sensor) {
        qCWarning(lcWinRtSensors) << "Default rotationsensor was not found on the system.";
        return;
    }

    hr = d->sensor->get_MinimumReportInterval(&d->minimumReportInterval);
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to get the minimum report interval."
                                  << qt_error_string(hr);
        sensorError(hr);
        return;
    }

    addDataRate(1, 1000 / d->minimumReportInterval); // dataRate in Hz
    sensor->setDataRate(1);

    setReading<QRotationReading>(&d->reading);
}

WinRtRotationSensor::~WinRtRotationSensor()
{
}

void WinRtRotationSensor::start()
{
    Q_D(WinRtRotationSensor);
    if (!d->sensor)
        return;
    if (d->token.value)
        return;

    ComPtr<InclinometerReadingHandler> callback =
            Callback<InclinometerReadingHandler>(d, &WinRtRotationSensorPrivate::readingChanged);
    HRESULT hr = d->sensor->add_ReadingChanged(callback.Get(), &d->token);

    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to attach to reading changed event."
                                  << qt_error_string(hr);
        sensorError(hr);
        return;
    }

    int dataRate = sensor()->dataRate();
    if (!dataRate)
        return;

    quint32 reportInterval = qMax(d->minimumReportInterval, quint32(1000/dataRate));
    hr = d->sensor->put_ReportInterval(reportInterval);
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to attach to set report interval."
                                  << qt_error_string(hr);
        sensorError(hr);
    }
}

void WinRtRotationSensor::stop()
{
    Q_D(WinRtRotationSensor);
    if (!d->sensor)
        return;
    if (!d->token.value)
        return;

    HRESULT hr = d->sensor->remove_ReadingChanged(d->token);
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to detach from reading changed event."
                                  << qt_error_string(hr);
        sensorError(hr);
        return;
    }
    hr = d->sensor->put_ReportInterval(0);
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to reset report interval."
                                  << qt_error_string(hr);
        sensorError(hr);
        return;
    }
    d->token.value = 0;
}

QT_END_NAMESPACE
