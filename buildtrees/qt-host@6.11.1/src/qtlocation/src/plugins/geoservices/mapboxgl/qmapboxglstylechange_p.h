// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQMAPBOXGLSTYLECHANGE_P_H
#define QQMAPBOXGLSTYLECHANGE_P_H

#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>
#include <QtGui/QImage>
#include <QtLocation/private/qdeclarativecirclemapitem_p.h>
#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtLocation/private/qdeclarativepolygonmapitem_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qdeclarativerectanglemapitem_p.h>

#include <QMapboxGL>

class QMapboxGLStyleChange
{
public:
    virtual ~QMapboxGLStyleChange() = default;

    static QList<QSharedPointer<QMapboxGLStyleChange>> addMapItem(QDeclarativeGeoMapItemBase *, const QString &before);
    static QList<QSharedPointer<QMapboxGLStyleChange>> removeMapItem(QDeclarativeGeoMapItemBase *);

    virtual void apply(QMapboxGL *map) = 0;
};

class QMapboxGLStyleSetLayoutProperty : public QMapboxGLStyleChange
{
public:
    static QList<QSharedPointer<QMapboxGLStyleChange>> fromMapItem(QDeclarativeGeoMapItemBase *);

    void apply(QMapboxGL *map) override;

private:
    static QList<QSharedPointer<QMapboxGLStyleChange>> fromMapItem(QDeclarativePolylineMapItem *);

    QMapboxGLStyleSetLayoutProperty() = default;
    QMapboxGLStyleSetLayoutProperty(const QString &layer, const QString &property, const QVariant &value);

    QString m_layer;
    QString m_property;
    QVariant m_value;
};

class QMapboxGLStyleSetPaintProperty : public QMapboxGLStyleChange
{
public:
    static QList<QSharedPointer<QMapboxGLStyleChange>> fromMapItem(QDeclarativeGeoMapItemBase *);

    void apply(QMapboxGL *map) override;

private:
    static QList<QSharedPointer<QMapboxGLStyleChange>> fromMapItem(QDeclarativeRectangleMapItem *);
    static QList<QSharedPointer<QMapboxGLStyleChange>> fromMapItem(QDeclarativeCircleMapItem *);
    static QList<QSharedPointer<QMapboxGLStyleChange>> fromMapItem(QDeclarativePolygonMapItem *);
    static QList<QSharedPointer<QMapboxGLStyleChange>> fromMapItem(QDeclarativePolylineMapItem *);

    QMapboxGLStyleSetPaintProperty() = default;
    QMapboxGLStyleSetPaintProperty(const QString &layer, const QString &property, const QVariant &value);

    QString m_layer;
    QString m_property;
    QVariant m_value;
};

class QMapboxGLStyleAddLayer : public QMapboxGLStyleChange
{
public:
    static QSharedPointer<QMapboxGLStyleChange> fromFeature(const QMapbox::Feature &feature, const QString &before);

    void apply(QMapboxGL *map) override;

private:
    QMapboxGLStyleAddLayer() = default;

    QVariantMap m_params;
    QString m_before;
};

class QMapboxGLStyleRemoveLayer : public QMapboxGLStyleChange
{
public:
    explicit QMapboxGLStyleRemoveLayer(const QString &id);

    void apply(QMapboxGL *map) override;

private:
    QMapboxGLStyleRemoveLayer() = default;

    QString m_id;
};

class QMapboxGLStyleAddSource : public QMapboxGLStyleChange
{
public:
    static QSharedPointer<QMapboxGLStyleChange> fromFeature(const QMapbox::Feature &feature);
    static QSharedPointer<QMapboxGLStyleChange> fromMapItem(QDeclarativeGeoMapItemBase *);

    void apply(QMapboxGL *map) override;

private:
    QMapboxGLStyleAddSource() = default;

    QString m_id;
    QVariantMap m_params;
};

class QMapboxGLStyleRemoveSource : public QMapboxGLStyleChange
{
public:
    explicit QMapboxGLStyleRemoveSource(const QString &id);

    void apply(QMapboxGL *map) override;

private:
    QMapboxGLStyleRemoveSource() = default;

    QString m_id;
};

class QMapboxGLStyleSetFilter : public QMapboxGLStyleChange
{
public:
    void apply(QMapboxGL *map) override;

private:
    QMapboxGLStyleSetFilter() = default;

    QString m_layer;
    QVariant m_filter;
};

class QMapboxGLStyleAddImage : public QMapboxGLStyleChange
{
public:
    void apply(QMapboxGL *map) override;

private:
    QMapboxGLStyleAddImage() = default;

    QString m_name;
    QImage m_sprite;
};

#endif // QQMAPBOXGLSTYLECHANGE_P_H
