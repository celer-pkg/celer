// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef QGEOSATELLITEINFOSOURCEANDROID_H
#define QGEOSATELLITEINFOSOURCEANDROID_H

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

#include <QGeoSatelliteInfoSource>
#include <QTimer>

class QGeoSatelliteInfoSourceAndroid : public QGeoSatelliteInfoSource
{
    Q_OBJECT
public:
    explicit QGeoSatelliteInfoSourceAndroid(QObject *parent = 0);
    ~QGeoSatelliteInfoSourceAndroid();

    //From QGeoSatelliteInfoSource
    void setUpdateInterval(int msec) override;
    int minimumUpdateInterval() const override;

    Error error() const override;

public Q_SLOTS:
    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 0) override;

    void processSatelliteUpdate(const QList<QGeoSatelliteInfo> &satsInView,
                                const QList<QGeoSatelliteInfo> &satsInUse,
                                bool isSingleUpdate);

    void locationProviderDisabled();
private Q_SLOTS:
    void requestTimeout();

private:
    void reconfigureRunningSystem();
    void setError(QGeoSatelliteInfoSource::Error error);

    Error m_error;
    int androidClassKeyForUpdate;
    int androidClassKeyForSingleRequest;
    bool updatesRunning;

    QTimer requestTimer;
    QList<QGeoSatelliteInfo> m_satsInUse;
    QList<QGeoSatelliteInfo> m_satsInView;

};

#endif // QGEOSATELLITEINFOSOURCEANDROID_H
