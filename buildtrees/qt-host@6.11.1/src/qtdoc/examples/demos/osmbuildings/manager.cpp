// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "manager.h"

#include <QImage>
#include <QRgb>

#include <algorithm>

OSMManager::OSMManager(QObject *parent)
    : QObject{parent},
      m_request(new OSMRequest(this))
{
    connect(m_request, &OSMRequest::buildingsDataReady, this, [this](const QList<QVariant> &geoVariantsList
                                                                     , int tileX, int tileY, int zoomLevel){
        m_buildingsHash.insert(OSMTileData{tileX, tileY, zoomLevel}, true);
        emit buildingsDataReady(geoVariantsList, tileX - m_startBuildingTileX,
                                tileY - m_startBuildingTileY,
                                zoomLevel);
    });

    connect(m_request, &OSMRequest::mapsDataReady, this, [this](const QByteArray &mapData, int tileX, int tileY, int zoomLevel){
        emit mapsDataReady(mapData, tileX - m_startBuildingTileX,
                           tileY - m_startBuildingTileY,
                           zoomLevel);
    });

}

void OSMManager::setCameraProperties(const QVector3D &position, const QVector3D &right,
                                    float cameraZoom, float minimumZoom, float maximumZoom,
                                    float cameraTilt, float minimumTilt, float maximumTilt)
{

    float tiltFactor = (cameraTilt - minimumTilt) / qMax(maximumTilt - minimumTilt, 1.0);
    float zoomFactor = (cameraZoom - minimumZoom) / qMax(maximumZoom - minimumZoom, 1.0);

    // Forward vector align to the XY plane
    QVector3D forwardVector = QVector3D::crossProduct(right,
                                                      QVector3D(0.0, 0.0, -1.0)).normalized();
    QVector3D projectionOfForwardOnXY = position
            + forwardVector * tiltFactor * zoomFactor * 50.0;

    QQueue<OSMTileData> queue;
    for ( int forwardIndex = -20; forwardIndex <= 20; ++forwardIndex ){
        for ( int sidewardIndex = -20; sidewardIndex <= 20; ++sidewardIndex ){
            QVector3D transferredPosition = projectionOfForwardOnXY
                      + QVector3D(float(m_tileSizeX * sidewardIndex), float(m_tileSizeY * forwardIndex), 0.0);
            addBuildingRequestToQueue(queue, m_startBuildingTileX + int(transferredPosition.x() / m_tileSizeX),
                                m_startBuildingTileY - int(transferredPosition.y() / m_tileSizeY));
        }
    }

    const QPoint projectedTile{m_startBuildingTileX + int(projectionOfForwardOnXY.x() / m_tileSizeX),
                               m_startBuildingTileY - int(projectionOfForwardOnXY.y() / m_tileSizeY)};

    auto closer = [projectedTile](const OSMTileData &v1, const OSMTileData &v2) -> bool {
        return v1.distanceTo(projectedTile) < v2.distanceTo(projectedTile);
    };
    std::sort(queue.begin(), queue.end(), closer);

    m_request->getBuildingsData( queue );
    m_request->getMapsData( queue );

}

void OSMManager::addBuildingRequestToQueue(QQueue<OSMTileData> &queue, int tileX, int tileY, int zoomLevel)
{
    OSMTileData data{tileX, tileY, zoomLevel};
    if ( !m_buildingsHash.contains(data) )
        queue.append(data);
}

int OSMManager::tileSizeX() const
{
    return m_tileSizeX;
}

int OSMManager::tileSizeY() const
{
    return m_tileSizeY;
}

bool OSMManager::isDemoToken() const
{
    return m_request->isDemoToken();
}

void OSMManager::setToken(const QString &token)
{
    m_request->setToken(token);
}

QString OSMManager::token() const
{
    return m_request->token();
}

void CustomTextureData::setImageData(const QByteArray &data)
{
    QImage image = QImage::fromData(data).convertToFormat(QImage::Format_RGBA8888);
    setTextureData( QByteArray(reinterpret_cast<const char*>(image.constBits()), image.sizeInBytes()) );
    setSize( image.size() );
    setHasTransparency(false);
    setFormat(Format::RGBA8);
}
