// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "winrtcompass.h"
#include "winrtcommon.h"

#include <QtSensors/QCompass>

#include <functional>
#include <wrl.h>
#include <windows.devices.sensors.h>
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Devices::Sensors;

typedef ITypedEventHandler<Compass *, CompassReadingChangedEventArgs *> CompassReadingHandler;

QT_BEGIN_NAMESPACE

class WinRtCompassPrivate
{
public:
    WinRtCompassPrivate(WinRtCompass *p)
        : minimumReportInterval(0), q(p)
    {
        token.value = 0;
    }

    QCompassReading reading;

    ComPtr<ICompass> sensor;
    EventRegistrationToken token;
    quint32 minimumReportInterval;

    HRESULT readingChanged(ICompass *, ICompassReadingChangedEventArgs *args)
    {
        ComPtr<ICompassReading> value;
        HRESULT hr = args->get_Reading(&value);
        if (FAILED(hr)) {
            qCWarning(lcWinRtSensors) << "Failed to get light sensor reading." << qt_error_string(hr);
            return hr;
        }

        DOUBLE heading;
        hr = value->get_HeadingMagneticNorth(&heading);
        if (FAILED(hr)) {
            qCWarning(lcWinRtSensors) << "Failed to get compass heading." << qt_error_string(hr);
            return hr;
        }

        DateTime dateTime;
        hr = value->get_Timestamp(&dateTime);
        if (FAILED(hr)) {
            qCWarning(lcWinRtSensors) << "Failed to get compass reading timestamp." << qt_error_string(hr);
            return hr;
        }
        ComPtr<ICompassReadingHeadingAccuracy> accuracyReading;
        hr = value.As(&accuracyReading);
        if (FAILED(hr)) {
            qCWarning(lcWinRtSensors) << "Failed to cast compass reading to obtain accuracy." << qt_error_string(hr);
            return hr;
        }

        MagnetometerAccuracy accuracy;
        hr = accuracyReading->get_HeadingAccuracy(&accuracy);
        if (FAILED(hr)) {
            qCWarning(lcWinRtSensors) << "Failed to get compass reading accuracy." << qt_error_string(hr);
            return hr;
        }

        switch (accuracy) {
        default:
        case MagnetometerAccuracy_Unknown:
            reading.setCalibrationLevel(0.00);
            break;
        case MagnetometerAccuracy_Unreliable:
            reading.setCalibrationLevel(0.33);
            break;
        case MagnetometerAccuracy_Approximate:
            reading.setCalibrationLevel(0.67);
            break;
        case MagnetometerAccuracy_High:
            reading.setCalibrationLevel(1.00);
            break;
        }

        reading.setAzimuth(heading);
        reading.setTimestamp(dateTimeToMsSinceEpoch(dateTime));
        q->newReadingAvailable();
        return S_OK;
    }

private:
    WinRtCompass *q;
};

WinRtCompass::WinRtCompass(QSensor *sensor)
    : QSensorBackend(sensor), d_ptr(new WinRtCompassPrivate(this))
{
    Q_D(WinRtCompass);

    HStringReference classId(RuntimeClass_Windows_Devices_Sensors_Compass);
    ComPtr<ICompassStatics> factory;
    HRESULT hr = RoGetActivationFactory(classId.Get(), IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to initialize light sensor factory."
                                      << qt_error_string(hr);
        sensorError(hr);
        return;
    }

    hr = factory->GetDefault(&d->sensor);
    if (FAILED(hr)) {
        qCWarning(lcWinRtSensors) << "Unable to get default compass."
                                      << qt_error_string(hr);
        sensorError(hr);
        return;
    }

    if (!d->sensor) {
        qCWarning(lcWinRtSensors) << "Default compass was not found on the system.";
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

    setReading<QCompassReading>(&d->reading);
}

WinRtCompass::~WinRtCompass()
{
}

void WinRtCompass::start()
{
    Q_D(WinRtCompass);
    if (!d->sensor)
        return;
    if (d->token.value)
        return;
    ComPtr<CompassReadingHandler> callback =
            Callback<CompassReadingHandler>(d, &WinRtCompassPrivate::readingChanged);
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

void WinRtCompass::stop()
{
    Q_D(WinRtCompass);
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
