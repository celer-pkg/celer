// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OSMMANAGER_H
#define OSMMANAGER_H

#include <QQuick3DTextureData>
#include <QtQmlIntegration>
#include <QVector3D>
#include "request.h"

class OSMManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit OSMManager(QObject *parent = nullptr);
    int tileSizeX() const;
    int tileSizeY() const;
    Q_INVOKABLE bool isDemoToken() const;
    Q_INVOKABLE void setToken(const QString &token);
    Q_INVOKABLE QString token() const;

public slots:
    void setCameraProperties(const QVector3D &position , const QVector3D &right,
                            float cameraZoom, float minimunZoom, float maximumZoom,
                            float cameraTilt, float minimumTilt, float maxmumTilt);

signals:
    void buildingsDataReady(const QList<QVariant> &geoVariantsList, int tileX, int tileY, int zoomLevel);
    void mapsDataReady(class QByteArray mapData, int tileX, int tileY, int zoomLevel);

private:
    void addBuildingRequestToQueue(QQueue<OSMTileData> &queue, int tileX, int tileY, int zoomLevel = 15);
    class OSMRequest *m_request = nullptr;
    QHash<OSMTileData, bool> m_buildingsHash;
    const int m_startBuildingTileX = 17605;
    const int m_startBuildingTileY = 10746;
    const int m_tileSizeX = 37;
    const int m_tileSizeY = 37;

    Q_PROPERTY(int tileSizeX READ tileSizeX CONSTANT FINAL)
    Q_PROPERTY(int tileSizeY READ tileSizeY CONSTANT FINAL)
};

class CustomTextureData : public QQuick3DTextureData
{
    Q_OBJECT
    QML_ELEMENT
public slots:
    void setImageData(const QByteArray &data );
};


#endif // MANAGER_H
