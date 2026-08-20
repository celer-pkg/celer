// Copyright (C) 2018 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONINFOSOURCE_GEOCLUE2_P_H
#define QGEOPOSITIONINFOSOURCE_GEOCLUE2_P_H

#include <QtPositioning/QGeoPositionInfoSource>
#include <QtCore/QPointer>
#include <manager_interface.h>

namespace QtPositioningPrivate {

class OrgFreedesktopGeoClue2ClientInterface;

} // namespace QtPositioningPrivate

QT_BEGIN_NAMESPACE
class QDBusObjectPath;
class QTimer;

class QGeoPositionInfoSourceGeoclue2 : public QGeoPositionInfoSource
{
    Q_OBJECT

public:
    explicit QGeoPositionInfoSourceGeoclue2(const QVariantMap &parameters,
                                            QObject *parent = nullptr);
    ~QGeoPositionInfoSourceGeoclue2();

    // From QGeoPositionInfoSource
    void setUpdateInterval(int msec) override;
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const override;
    PositioningMethods supportedPositioningMethods() const override;
    void setPreferredPositioningMethods(PositioningMethods methods) override;
    int minimumUpdateInterval() const override;

    Error error() const override;

    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 5000) override;

private:
    void setError(QGeoPositionInfoSource::Error error);
    void restoreLastPosition();
    void saveLastPosition();
    void createClient();
    bool configureClient();
    void startClient();
    void stopClient();
    void requestUpdateTimeout();
    void handleNewLocation(const QDBusObjectPath &oldLocation,
                           const QDBusObjectPath &newLocation);
    void parseParameters(const QVariantMap &parameters);

    QTimer *m_requestTimer = nullptr;
    QtPositioningPrivate::OrgFreedesktopGeoClue2ManagerInterface m_manager;
    QPointer<QtPositioningPrivate::OrgFreedesktopGeoClue2ClientInterface> m_client;
    bool m_running = false;
    bool m_lastPositionFromSatellite = false;
    QGeoPositionInfoSource::Error m_error = NoError;
    QGeoPositionInfo m_lastPosition;
    QString m_desktopId;
};

QT_END_NAMESPACE

#endif // QGEOPOSITIONINFOSOURCE_GEOCLUE2_P_H
