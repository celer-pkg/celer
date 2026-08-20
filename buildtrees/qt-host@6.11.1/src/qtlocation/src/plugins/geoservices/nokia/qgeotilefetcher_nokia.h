// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOTILEFETCHER_NOKIA_H
#define QGEOTILEFETCHER_NOKIA_H

#include "qgeoserviceproviderplugin_nokia.h"

#include <QtLocation/private/qgeotilefetcher_p.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMapReply;
class QGeoTileSpec;
class QGeoTiledMappingManagerEngine;
class QGeoTiledMappingManagerEngineNokia;
class QNetworkReply;
class QGeoNetworkAccessManager;
class QGeoUriProvider;

class QGeoTileFetcherNokia : public QGeoTileFetcher
{
    Q_OBJECT

public:
    QGeoTileFetcherNokia(const QVariantMap &parameters, QGeoNetworkAccessManager *networkManager,
                         QGeoTiledMappingManagerEngineNokia *engine, const QSize &tileSize, int ppi);
    ~QGeoTileFetcherNokia();

    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec) override;

    QString apiKey() const;

public Q_SLOTS:
    void copyrightsFetched();
    void fetchCopyrightsData();
    void versionFetched();
    void fetchVersionData();

private:
    Q_DISABLE_COPY(QGeoTileFetcherNokia)

    QString getRequestString(const QGeoTileSpec &spec, int ppi=72) const;

    QString getLanguageString() const;

    QPointer<QGeoTiledMappingManagerEngineNokia> m_engineNokia;
    QGeoNetworkAccessManager *m_networkManager;
    int m_tileSize;
    int m_ppi;
    QNetworkReply *m_copyrightsReply;
    QNetworkReply *m_versionReply;

    QString m_apiKey;
    QGeoUriProvider *m_baseUriProvider;
    QGeoUriProvider *m_aerialUriProvider;
};

QT_END_NAMESPACE

#endif
