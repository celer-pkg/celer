// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOMAPMAPBOXGL_H
#define QGEOMAPMAPBOXGL_H

#include "qgeomappingmanagerenginemapboxgl.h"
#include <QtLocation/private/qgeomap_p.h>

class QGeoMapMapboxGLPrivate;

class QGeoMapMapboxGL : public QGeoMap
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoMapMapboxGL)

public:
    QGeoMapMapboxGL(QGeoMappingManagerEngineMapboxGL *engine, QObject *parent);
    virtual ~QGeoMapMapboxGL();

    QString copyrightsStyleSheet() const override;
    void setMapboxGLSettings(const QMapboxGLSettings &, bool useChinaEndpoint);
    void setUseFBO(bool);
    void setMapItemsBefore(const QString &);
    Capabilities capabilities() const override;

private Q_SLOTS:
    // QMapboxGL
    void onMapChanged(QMapboxGL::MapChange);

    // QDeclarativeGeoMapItemBase
    void onMapItemPropertyChanged();
    void onMapItemSubPropertyChanged();
    void onMapItemUnsupportedPropertyChanged();
    void onMapItemGeometryChanged();

public Q_SLOTS:
    void copyrightsChanged(const QString &copyrightsHtml);

private:
    QSGNode *updateSceneGraph(QSGNode *oldNode, QQuickWindow *window) override;

    QGeoMappingManagerEngineMapboxGL *m_engine;
};

#endif // QGEOMAPMAPBOXGL_H
