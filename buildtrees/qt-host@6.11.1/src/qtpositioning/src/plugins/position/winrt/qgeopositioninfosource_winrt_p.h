// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONINFOSOURCEWINRT_H
#define QGEOPOSITIONINFOSOURCEWINRT_H

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

#include "qgeopositioninfosource.h"
#include "qgeopositioninfo.h"

#include <QtCore/qtimer.h>

#include <EventToken.h>
#include <wrl.h>

#include <memory>

namespace ABI {
    namespace Windows {
        namespace Devices {
            namespace Geolocation{
                struct IGeolocator;
                struct IPositionChangedEventArgs;
                struct IStatusChangedEventArgs;
            }
        }
    }
}

QT_BEGIN_NAMESPACE

class QGeoPositionInfoSourceWinRTPrivate;

class QGeoPositionInfoSourceWinRT : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    QGeoPositionInfoSourceWinRT(QObject *parent = nullptr);
    ~QGeoPositionInfoSourceWinRT() override;
    int init();

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;
    PositioningMethods supportedPositioningMethods() const override;

    void setPreferredPositioningMethods(PositioningMethods methods) override;

    void setUpdateInterval(int msec) override;
    int minimumUpdateInterval() const override;
    Error error() const override;

    HRESULT onPositionChanged(ABI::Windows::Devices::Geolocation::IGeolocator *locator,
                              ABI::Windows::Devices::Geolocation::IPositionChangedEventArgs *args);
    HRESULT onStatusChanged(ABI::Windows::Devices::Geolocation::IGeolocator *locator,
                              ABI::Windows::Devices::Geolocation::IStatusChangedEventArgs *args);

    bool requestAccess() const;
Q_SIGNALS:
    void nativePositionUpdate(const QGeoPositionInfo);
public slots:
    void startUpdates() override;
    void stopUpdates() override;

    void requestUpdate(int timeout = 0) override;

private slots:
    void stopHandler();
    void virtualPositionUpdate();
    void singleUpdateTimeOut();
    void updateSynchronized(const QGeoPositionInfo info);
    void reactOnError(QGeoPositionInfoSource::Error positionError);
private:
    bool startHandler();

    Q_DISABLE_COPY(QGeoPositionInfoSourceWinRT)
    void setError(QGeoPositionInfoSource::Error positionError);

    std::unique_ptr<QGeoPositionInfoSourceWinRTPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QGeoPositionInfoSourceWinRT)
};

QT_END_NAMESPACE

#endif // QGEOPOSITIONINFOSOURCEWINRT_H
