// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONINFOSOURCE_ANDROID_P_H
#define QGEOPOSITIONINFOSOURCE_ANDROID_P_H

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

#include <QGeoPositionInfoSource>
#include <QTimer>

class QGeoPositionInfoSourceAndroid : public QGeoPositionInfoSource
{
    Q_OBJECT
public:
    QGeoPositionInfoSourceAndroid(const QVariantMap &parameters, QObject *parent = 0);
    ~QGeoPositionInfoSourceAndroid();

    // From QGeoPositionInfoSource
    void setUpdateInterval(int msec) override;
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;
    PositioningMethods supportedPositioningMethods() const override;
    void setPreferredPositioningMethods(PositioningMethods methods) override;
    int minimumUpdateInterval() const override;
    Error error() const override;

    bool useAltitudeConverter() const;

public Q_SLOTS:
    virtual void startUpdates() override;
    virtual void stopUpdates() override;

    virtual void requestUpdate(int timeout = 0) override;

    void processPositionUpdate(const QGeoPositionInfo& pInfo);
    void processSinglePositionUpdate(const QGeoPositionInfo& pInfo);

    void locationProviderDisabled();
    void locationProvidersChanged();
private Q_SLOTS:
    void requestTimeout();
    void regularUpdatesTimeout();

private:
    void reconfigureRunningSystem();
    void setError(Error error);
    void parseParameters(const QVariantMap &parameters);

    bool updatesRunning = false;
    int androidClassKeyForUpdate;
    int androidClassKeyForSingleRequest;
    QList<QGeoPositionInfo> queuedSingleUpdates;
    Error m_error = NoError;
    QTimer m_requestTimer;
    QTimer m_regularUpdatesTimer;
    qint64 m_lastUpdateTime = 0;
    bool m_regularUpdatesErrorRaised = false;
    bool m_useAltitudeConverter = false;
};

#endif // QGEOPOSITIONINFOSOURCE_ANDROID_P_H
