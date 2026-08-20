// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONINFOSOURCEWASM_H
#define QGEOPOSITIONINFOSOURCEWASM_H

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
#include "qgeopositioninfosourcedispatcher_wasm_p.h"


#include <emscripten/val.h>

QT_BEGIN_NAMESPACE

using emscripten::val;
typedef emscripten::val GeolocationPosition;
typedef emscripten::val Geolocation;
typedef emscripten::val GeolocationPositionError;
typedef emscripten::val GeolocationPositionHandlerId;

class QGeoPositionInfoDispatcherWasm;

class QGeoPositionInfoSourceWasm : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    static Geolocation geolocation();
    QGeoPositionInfoSourceWasm(QObject *parent = 0);
    ~QGeoPositionInfoSourceWasm();

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;
    PositioningMethods supportedPositioningMethods() const override;

    void setPreferredPositioningMethods(PositioningMethods methods) override;

    void setUpdateInterval(int msec) override;
    int minimumUpdateInterval() const override;
    Error error() const override;

    void geolocationSuccess(GeolocationPosition position);
    void geolocationError(GeolocationPositionError error);

public Q_SLOTS:
    void startUpdates() override;
    void stopUpdates() override;

    void requestUpdate(int timeout = 0) override;

private:
    void reconfigureRunningSystem();

    GeolocationPositionHandlerId m_handlerId;
    QGeoPositionInfo m_lastKnownPosition;
    QGeoPositionInfoDispatcherWasm *m_dispatcher;
    quint32 m_instanceId;
    QGeoPositionInfoSource::Error m_lastError = NoError;
    bool m_updates_running = false;
    bool m_singleUpdatePending = false;
    val successCallback;
    val errorCallback;
    Q_DISABLE_COPY(QGeoPositionInfoSourceWasm)
};

QT_END_NAMESPACE

#endif // QGEOPOSITIONINFOSOURCEWASM_H
