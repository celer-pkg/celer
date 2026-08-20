// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OSMGEOMETRY_H
#define OSMGEOMETRY_H

#include <QQuick3DGeometry>
#include <QtQmlIntegration>
#include <QVector3D>

class OSMGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit OSMGeometry( QQuick3DGeometry *parent = nullptr );

signals:
    void geometryReady();

public slots:
    void updateData(const QList<QVariant> &geoVariantsList);

private slots:
    void loadGeometryFromData(const QList<QVariant> &geoVariantsList);
};


#endif // OSMGEOMETRY_H
