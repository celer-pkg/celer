// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOCAMERATILES_P_H
#define QGEOCAMERATILES_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtCore/QScopedPointer>

#include <memory>

QT_BEGIN_NAMESPACE

class QGeoCameraData;
class QGeoTileSpec;
class QGeoMapType;
class QGeoCameraTilesPrivate;
class QSize;
class QRectF;

class Q_LOCATION_EXPORT QGeoCameraTiles {
public:
    QGeoCameraTiles();
    ~QGeoCameraTiles();

    void setCameraData(const QGeoCameraData &camera);
    QGeoCameraData cameraData() const;
    void setVisibleArea(const QRectF &visibleArea);
    void setScreenSize(const QSize &size);
    void setTileSize(int tileSize);
    int tileSize() const;
    void setViewExpansion(double viewExpansion);
    void setPluginString(const QString &pluginString);
    void setMapType(const QGeoMapType &mapType);
    QGeoMapType activeMapType() const;
    void setMapVersion(int mapVersion);
    const QSet<QGeoTileSpec>& createTiles();

protected:
    std::unique_ptr<QGeoCameraTilesPrivate> d_ptr;

    friend class QGeoCameraTilesPrivate;
    Q_DISABLE_COPY(QGeoCameraTiles)
};

QT_END_NAMESPACE

#endif // QGEOCAMERATILES_P_H
