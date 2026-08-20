// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEFETCHEROSM_H
#define QGEOTILEFETCHEROSM_H

#include "qgeotileproviderosm.h"
#include <QtLocation/private/qgeotilefetcher_p.h>
#include <QList>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;
class QGeoMappingManagerEngine;
class QGeoTileFetcherOsmPrivate;

class QGeoTileFetcherOsm : public QGeoTileFetcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoTileFetcherOsm)

    friend class QGeoMapReplyOsm;
    friend class QGeoTiledMappingManagerEngineOsm;
public:
    QGeoTileFetcherOsm(const QList<QGeoTileProviderOsm *> &providers, QNetworkAccessManager *nm,
                       QGeoMappingManagerEngine *parent);

    void setUserAgent(const QByteArray &userAgent);

Q_SIGNALS:
    void providerDataUpdated(const QGeoTileProviderOsm *provider);

protected:
    bool initialized() const override;

protected Q_SLOTS:
    void onProviderResolutionFinished(const QGeoTileProviderOsm *provider);
    void onProviderResolutionError(const QGeoTileProviderOsm *provider);
    void restartTimer();

private:
    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec) override;
    void readyUpdated();

    QByteArray m_userAgent;
    QList<QGeoTileProviderOsm *> m_providers;
    QNetworkAccessManager *m_nm;
    bool m_ready;
};

QT_END_NAMESPACE

#endif // QGEOTILEFETCHEROSM_H

