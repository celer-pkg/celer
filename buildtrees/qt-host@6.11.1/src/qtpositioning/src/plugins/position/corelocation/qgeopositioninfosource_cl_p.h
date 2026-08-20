// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONINFOSOURCECL_H
#define QGEOPOSITIONINFOSOURCECL_H

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

#import <CoreLocation/CoreLocation.h>

#include "qgeopositioninfosource.h"
#include "qgeopositioninfo.h"

QT_BEGIN_NAMESPACE

class QGeoPositionInfoSourceCL : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    QGeoPositionInfoSourceCL(QObject *parent = 0);
    ~QGeoPositionInfoSourceCL();

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;
    PositioningMethods supportedPositioningMethods() const override;

    void setUpdateInterval(int msec) override;
    int minimumUpdateInterval() const override;
    Error error() const override;

    void locationDataAvailable(QGeoPositionInfo location);
    void setError(QGeoPositionInfoSource::Error positionError);
    void changeAuthorizationStatus(CLAuthorizationStatus status);

private:
    bool enableLocationManager();
    void setTimeoutInterval(int msec);

public Q_SLOTS:
    void startUpdates() override;
    void stopUpdates() override;

    void requestUpdate(int timeout = 0) override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    Q_DISABLE_COPY(QGeoPositionInfoSourceCL);
    CLLocationManager *m_locationManager;
    bool m_updatesWanted;

    QGeoPositionInfo m_lastUpdate;

    int m_updateTimer;
    int m_updateTimeout;

    QGeoPositionInfoSource::Error m_positionError;
};

QT_END_NAMESPACE

#endif // QGEOPOSITIONINFOSOURCECL_H
