// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOAREAMONITORPOLLING_H
#define QGEOAREAMONITORPOLLING_H

#include <QtPositioning/qgeoareamonitorsource.h>
#include <QtPositioning/qgeopositioninfosource.h>
#include <QtCore/qmutex.h>

/**
 *  QGeoAreaMonitorPolling
 *
 */

QT_BEGIN_NAMESPACE

class QGeoAreaMonitorPollingPrivate;
class QGeoAreaMonitorPolling : public QGeoAreaMonitorSource
{
    Q_OBJECT
public :
    explicit QGeoAreaMonitorPolling(QObject *parent = 0);
    ~QGeoAreaMonitorPolling();

    void setPositionInfoSource(QGeoPositionInfoSource *source) override;
    QGeoPositionInfoSource* positionInfoSource() const override;

    Error error() const override;

    bool startMonitoring(const QGeoAreaMonitorInfo &monitor) override;
    bool requestUpdate(const QGeoAreaMonitorInfo &monitor,
                       const char *signal) override;
    bool stopMonitoring(const QGeoAreaMonitorInfo &monitor) override;

    QList<QGeoAreaMonitorInfo> activeMonitors() const override;
    QList<QGeoAreaMonitorInfo> activeMonitors(const QGeoShape &region) const override;

    QGeoAreaMonitorSource::AreaMonitorFeatures supportedAreaMonitorFeatures() const override;

    inline bool isValid() { return positionInfoSource(); }

private Q_SLOTS:
    void positionError(QGeoPositionInfoSource::Error error);
    void timeout(const QGeoAreaMonitorInfo &monitor);
    void processAreaEvent(const QGeoAreaMonitorInfo &minfo, const QGeoPositionInfo &pinfo, bool isEnteredEvent);

private:
    QGeoAreaMonitorPollingPrivate* d;
    QGeoAreaMonitorSource::Error lastError = QGeoAreaMonitorSource::NoError;
    friend class QGeoAreaMonitorPollingPrivate;

    int signalConnections = 0;
    // connectNotify() and disconnectNotify() can be called from a different
    // thread, so we need to synchronize the access to signalConnections
    QMutex connectionMutex;

    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;

    int idForSignal(const char *signal);

    bool hasConnections() const;
};

QT_END_NAMESPACE

#endif // QGEOAREAMONITORPOLLING_H
