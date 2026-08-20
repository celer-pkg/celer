// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OSMREQUEST_H
#define OSMREQUEST_H

#include <QNetworkAccessManager>
#include <QQueue>
#include <QTimer>
#include <qcomparehelpers.h>

QT_FORWARD_DECLARE_CLASS(QPoint)

struct OSMTileData
{
    int TileX;
    int TileY;
    int ZoomLevel;

    qreal distanceTo(QPoint p) const;

    friend bool comparesEqual(OSMTileData lhs,
                              OSMTileData rhs) noexcept
    {
        return lhs.TileX == rhs.TileX && lhs.TileY == rhs.TileY
                && lhs.ZoomLevel == rhs.ZoomLevel;
    }

    Q_DECLARE_EQUALITY_COMPARABLE(OSMTileData)
};

size_t qHash(OSMTileData data, size_t seed = 0) noexcept;

class OSMRequest : public QObject
{
    Q_OBJECT

public:
    explicit OSMRequest(QObject *parent = nullptr);
    bool isDemoToken() const;

    QString token() const;
    void setToken(const QString &token);

public slots:
    void getBuildingsData(const QQueue<OSMTileData> &buildingsQueue);
    void getMapsData(const QQueue<OSMTileData> &mapsQueue);

signals:
    void buildingsDataReady( const QList<QVariant> &geoVariantsList, int tileX, int tileY, int zoomLevel );
    void mapsDataReady(const QByteArray &mapData, int tileX, int tileY, int zoomLevel );

private slots:
    void getBuildingsDataRequest(const OSMTileData &tile);
    void getMapsDataRequest(const OSMTileData &tile);

private:

    int m_buildingsNumberOfRequestsInFlight = 0;
    int m_mapsNumberOfRequestsInFlight = 0;
    QTimer m_queuesTimer;
    QQueue<OSMTileData> m_buildingsQueue;
    QQueue<OSMTileData> m_mapsQueue;
    QNetworkAccessManager m_networkAccessManager;
    QString m_token;
};


#endif // REQUEST_H
