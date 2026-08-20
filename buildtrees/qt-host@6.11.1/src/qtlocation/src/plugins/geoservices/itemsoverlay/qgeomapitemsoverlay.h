// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPITEMSOVERLAY_H
#define QGEOMAPITEMSOVERLAY_H

#include <QtLocation/private/qgeomap_p.h>
#include <QtLocation/private/qgeoprojection_p.h>

QT_BEGIN_NAMESPACE

class QGeoMappingManagerEngineItemsOverlay;
class QGeoMapItemsOverlayPrivate;
class QGeoMapItemsOverlay: public QGeoMap
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoMapItemsOverlay)
public:
    QGeoMapItemsOverlay(QGeoMappingManagerEngineItemsOverlay *engine, QObject *parent);
    virtual ~QGeoMapItemsOverlay();

    QGeoMap::Capabilities capabilities() const override;

protected:
    QSGNode *updateSceneGraph(QSGNode *node, QQuickWindow *window) override;

private:
    Q_DISABLE_COPY(QGeoMapItemsOverlay)
};

QT_END_NAMESPACE

#endif
