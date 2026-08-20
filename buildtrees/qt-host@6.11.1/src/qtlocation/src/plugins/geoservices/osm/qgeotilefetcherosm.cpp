// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeotilefetcherosm.h"
#include "qgeomapreplyosm.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QtLocation/private/qgeotilefetcher_p_p.h>


QT_BEGIN_NAMESPACE

static bool providersResolved(const QList<QGeoTileProviderOsm *> &providers)
{
    for (const QGeoTileProviderOsm *provider : providers)
        if (!provider->isResolved())
            return false;
    return true;
}

class QGeoTileFetcherOsmPrivate : public QGeoTileFetcherPrivate
{
    Q_DECLARE_PUBLIC(QGeoTileFetcherOsm)
public:
    QGeoTileFetcherOsmPrivate();
    virtual ~QGeoTileFetcherOsmPrivate();

private:
    Q_DISABLE_COPY(QGeoTileFetcherOsmPrivate)
};

QGeoTileFetcherOsmPrivate::QGeoTileFetcherOsmPrivate() : QGeoTileFetcherPrivate()
{
}

QGeoTileFetcherOsmPrivate::~QGeoTileFetcherOsmPrivate()
{
}

QGeoTileFetcherOsm::QGeoTileFetcherOsm(const QList<QGeoTileProviderOsm *> &providers,
                                       QNetworkAccessManager *nm, QGeoMappingManagerEngine *parent)
    : QGeoTileFetcher(*new QGeoTileFetcherOsmPrivate(), parent),
      m_userAgent("Qt Location based application"),
      m_providers(providers),
      m_nm(nm),
      m_ready(true)
{
    m_nm->setParent(this);
    for (QGeoTileProviderOsm *provider : m_providers) {
        if (!provider->isResolved()) {
            m_ready = false;
            connect(provider, &QGeoTileProviderOsm::resolutionFinished,
                    this, &QGeoTileFetcherOsm::onProviderResolutionFinished);
            connect(provider, &QGeoTileProviderOsm::resolutionError,
                    this, &QGeoTileFetcherOsm::onProviderResolutionError);
            connect(provider, &QGeoTileProviderOsm::resolutionRequired,
                    this, &QGeoTileFetcherOsm::restartTimer, Qt::QueuedConnection);
            provider->resolveProvider();
        }
    }
    if (m_ready)
        readyUpdated();
}

void QGeoTileFetcherOsm::setUserAgent(const QByteArray &userAgent)
{
    m_userAgent = userAgent;
}

bool QGeoTileFetcherOsm::initialized() const
{
    if (!m_ready) {
        for (QGeoTileProviderOsm *provider : m_providers)
            if (!provider->isResolved())
                provider->resolveProvider();
    }
    return m_ready;
}

void QGeoTileFetcherOsm::onProviderResolutionFinished(const QGeoTileProviderOsm *provider)
{
    if ((m_ready = providersResolved(m_providers))) {
        qWarning("QGeoTileFetcherOsm: all providers resolved");
        readyUpdated();
    }
    emit providerDataUpdated(provider);
}

void QGeoTileFetcherOsm::onProviderResolutionError(const QGeoTileProviderOsm *provider)
{
    if ((m_ready = providersResolved(m_providers))) {
        qWarning("QGeoTileFetcherOsm: all providers resolved");
        readyUpdated();
    }
    emit providerDataUpdated(provider);
}

void QGeoTileFetcherOsm::restartTimer()
{
    Q_D(QGeoTileFetcherOsm);

    if (!d->queue_.isEmpty())
        d->timer_.start(0, this);
}

QGeoTiledMapReply *QGeoTileFetcherOsm::getTileImage(const QGeoTileSpec &spec)
{
    int id = spec.mapId();
    if (id < 1 || id > m_providers.size()) {
        qWarning("Unknown map id %d\n", spec.mapId());
        if (m_providers.isEmpty())
            return nullptr;
        else
            id = 1;
    }
    id -= 1; // TODO: make OSM map ids start from 0.

    if (spec.zoom() > m_providers[id]->maximumZoomLevel() || spec.zoom() < m_providers[id]->minimumZoomLevel())
        return nullptr;

    const QUrl url = m_providers[id]->tileAddress(spec.x(), spec.y(), spec.zoom());
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    request.setUrl(url);
    QNetworkReply *reply = m_nm->get(request);
    return new QGeoMapReplyOsm(reply, spec, m_providers[id]->format());
}

void QGeoTileFetcherOsm::readyUpdated()
{
    updateTileRequests(QSet<QGeoTileSpec>(), QSet<QGeoTileSpec>());
}

QT_END_NAMESPACE
