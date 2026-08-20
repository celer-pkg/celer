// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOMAP_NOKIA_H
#define QGEOMAP_NOKIA_H

#include "qgeotiledmap_p.h"
#include <QtGui/QImage>
#include <QtCore/QPointer>
typedef QGeoTiledMap Map;

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngineNokia;

class QGeoTiledMapNokia: public Map
{
Q_OBJECT
public:
    QGeoTiledMapNokia(QGeoTiledMappingManagerEngineNokia *engine, QObject *parent = nullptr);
    ~QGeoTiledMapNokia();

    QString getViewCopyright() const;
    void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles) override;

private:
    QImage m_logo;
    QImage m_copyrightsSlab;
    QString m_lastCopyrightsString;
    QPointer<QGeoTiledMappingManagerEngineNokia> m_engine;

    Q_DISABLE_COPY(QGeoTiledMapNokia)
};

QT_END_NAMESPACE

#endif // QGEOMAP_NOKIA_H
